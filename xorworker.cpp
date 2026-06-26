#include <QDir>
#include <QDebug>
#include <QStringList>

#include "xorworker.h"

XorWorker::XorWorker(const QString &inDir, const QString &mask, const QString &outDir)
    : m_inDir(inDir), m_outDir(outDir)
{
    QStringList filters;
    filters << mask;

    m_inDir.setNameFilters (filters);
    m_inDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
}

void XorWorker::doWork()
{
    emit progress (0);

    if (!m_inDir.exists())
    {
        emit finished("Directory does not exist: " + m_inDir.absolutePath());
        return;
    }

    if (!m_outDir.exists())
    {
        emit finished("Directory does not exist: " + m_outDir.absolutePath ());
        return;
    }

    // Enable infinite progress bar
    emit setProgressMax(0);

    QThread::sleep(10);

    foreach (const QString &file, m_inDir.entryList()) {
        //QDir dir(m_InputDir);
        //dir.filePath (file);
        qDebug() << "Found file: " << file;
    }

    // Disable infinite progress bar
    emit setProgressMax(100);

    // TODO: Stage 2. XOR encrypt:

    emit progress (100);
    emit finished ();
}
