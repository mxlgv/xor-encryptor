#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QThread>

#include "worker.h"

constexpr qint64 k_blockSize = 50 * 1024 * 1024; // 50 Mb

Worker::Worker(const WorkerSettings &settings)
    : m_inDir(settings.inDir)
    , m_outDir(settings.outDir)
    , m_delInputFiles(settings.delInFiles)
    , m_renameOnConflict(settings.renameOnConflict)
    , m_timerEnabled(settings.useInterval)
{
    QStringList filters;
    filters << settings.mask;

    m_inDir.setNameFilters (filters);
    m_inDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QTime midnight(0, 0, 0);
    m_interval = midnight.msecsTo(settings.interval);
}

quint64 Worker::getFiles(QStringList &files) const
{
    quint64 totalSize = 0;
    QDirIterator dirIter(m_inDir);

    while (dirIter.hasNext()) {
        dirIter.next();

        totalSize += dirIter.fileInfo().size();
        files.append(dirIter.fileName());

        qDebug() << "Found file: " << dirIter.filePath();
    }

    return totalSize;
}

const QString Worker::genNewFileName(const QString &filePath)
{
    QString probeFile = filePath;

    QFileInfo fileInfo(probeFile);
    const QString &suffix = fileInfo.completeSuffix();
    const QString &basename = fileInfo.completeBaseName();
    const QDir &dir = fileInfo.dir();

    int postfixCount = 0;
    while (fileInfo.exists()) {
        if (suffix.isEmpty()) {
            probeFile = dir.absoluteFilePath(basename) + "_" + QString::number(postfixCount);
        } else {
            probeFile = dir.absoluteFilePath(basename + "_" + QString::number(postfixCount) + "."
                                             + suffix);
        }

        fileInfo.setFile(probeFile);
        postfixCount++;
    }

    return probeFile;
}

void Worker::doWork()
{
    while (true) {
        if (!encrypt()) {
            emit statusMsg("Ready");
            return;
        }

        if (m_timerEnabled) {
            emit statusMsg("Waiting for the timer...");

            QMutexLocker locker(&m_mutex);
            m_timerCondition.wait(&m_mutex, m_interval);

            if (m_isRunning) {
                while (m_isPaused) {
                    emit statusMsg("Pause");
                    m_pauseCondition.wait(&m_mutex);
                    emit statusMsg("Waiting for the timer...");
                }

                continue;
            }
        }

        break;
    }

    emit statusMsg("Ready");
    emit finished(WorkerResult::OK, "Successfully completed");
}

bool Worker::encrypt()
{
    // Stage 1. Scan input directory:
    emit progress (0);

    if (!m_inDir.exists())
    {
        emit finished(WorkerResult::Error,
                      "Directory does not exist: '" + m_inDir.absolutePath() + "'");
        return false;
    }

    if (!m_outDir.exists())
    {
        emit finished(WorkerResult::Error,
                      "Directory does not exist: '" + m_outDir.absolutePath() + "'");
        return false;
    }

    emit statusMsg("Scanning the input directory...");

    QStringList inFileNames;
    const quint64 totalBytes = getFiles(inFileNames);
    quint64 processedBytes = 0;

    qDebug() << "Total bytes: " << totalBytes;

    // Stage 2. XOR encrypt:
    foreach (const QString &fileName, inFileNames) {
        const QString &inFilePath = m_inDir.absoluteFilePath(fileName);
        const QString &outFilePath = m_outDir.absoluteFilePath(fileName);

        QFile inFile(m_inDir.absoluteFilePath(fileName));
        QFile outFile(m_outDir.absoluteFilePath(fileName));

        if (outFile.exists() && m_renameOnConflict) {
            const QString &newFilePath = genNewFileName(outFile.fileName());
            outFile.setFileName(newFilePath);
            qDebug() << "Renaming output file:" << outFile.fileName() << "->" << newFilePath;
        }

        const bool encryptSelf = outFile.fileName() == inFile.fileName();
        const auto inMode = encryptSelf ? QIODevice::ReadWrite : QIODevice::ReadOnly;

        if (!inFile.open(inMode)) {
            emit finished(WorkerResult::Error,
                          "Failed to open input file '" + inFile.fileName()
                              + "': " + inFile.errorString());
            return false;
        }

        if (!encryptSelf) {
            if (!outFile.open(QIODevice::WriteOnly)) {
                emit finished(WorkerResult::Error,
                              "Failed to open output file '" + outFile.fileName()
                                  + "': " + outFile.errorString());
                return false;
            }
        }

        emit statusMsg("Encrypting file: " + inFile.fileName());

        while (!inFile.atEnd()) {
            {
                QMutexLocker locker(&m_mutex);
                if (!m_isRunning) {
                    emit finished(WorkerResult::Warn, "Interrupted by the user");
                    return false;
                }

                while (m_isPaused) {
                    emit statusMsg("Paused");
                    m_pauseCondition.wait(&m_mutex);
                    emit statusMsg("Encrypting file: " + inFile.fileName());
                }
            }

            auto filePos = inFile.pos();

            QByteArray buffer = inFile.read(k_blockSize);

            const quint64 bufferSize = buffer.size();
            xor64(buffer, m_key);

            quint64 writtenBytes = 0;
            if (encryptSelf) {
                inFile.seek(filePos);
                writtenBytes = inFile.write(buffer);
            } else {
                writtenBytes = outFile.write(buffer);
            }

            if (writtenBytes != bufferSize) {
                emit finished(WorkerResult::Error,
                              "Not all data was written to the file '" + outFile.fileName()
                                  + "': " + outFile.errorString());
                return false;
            }

            processedBytes += writtenBytes;
            emit progress(100 * processedBytes / totalBytes);
        }

        if (m_delInputFiles && !encryptSelf) {
            inFile.close();
            inFile.remove();
        }
    }

    emit progress(100);
    return true;
}

void Worker::xor64(QByteArray &data, quint64 key)
{
    char *const ptr = data.data();

    const qsizetype size = data.size();

    // XORing in 8-byte blocks:
    qsizetype fastSize = size & ~7; // size alignment to 8 bytes

    qsizetype idx = 0;
    for (; idx < fastSize; idx += 8) {
        *(reinterpret_cast<quint64 *>(ptr + idx)) ^= key;
    }

    // XOR of unaligned remainder:
    const char *const keyBytes = reinterpret_cast<const char *>(&key);
    int keyIdx = 0;
    for (; idx < size; ++idx) {
        ptr[idx] ^= keyBytes[keyIdx++];
    }
}

void Worker::setPaused(bool pause)
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = pause;

    if (!m_isPaused) {
        m_pauseCondition.wakeAll();
    }
}

bool Worker::getPaused()
{
    QMutexLocker locker(&m_mutex);
    return m_isPaused;
}

void Worker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_isRunning = false;
    m_isPaused = false;
    m_pauseCondition.wakeAll();
    m_timerCondition.wakeAll();
}
