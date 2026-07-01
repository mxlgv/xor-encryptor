#ifndef WORKER_H
#define WORKER_H

#include <QDir>
#include <QMutexLocker>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QWaitCondition>

struct WorkerSettings
{
    const QString &inDir;
    const QString &mask;
    const QString &outDir;
    bool renameOnConflict;
    bool delInFiles;
    quint64 key;
    QTime interval;
    bool useInterval;
};

enum class WorkerResult {
    OK,
    Warn,
    Error,
};

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker(const WorkerSettings &setting);

    // Delete copy сtor
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    //  Delete move сtor
    Worker(Worker &&) = delete;
    Worker &operator=(Worker &&) = delete;

    void stop();
    void setPaused(bool pause);
    bool getPaused();

public slots:
    void doWork();

signals:
    void progress(int percentValue);
    void statusMsg(const QString &msg);
    void finished(WorkerResult status, const QString &msg);

private:
    quint64 m_key;

    QDir m_inDir;
    QDir m_outDir;
    bool m_renameOnConflict;
    bool m_delInputFiles;

    int m_interval{0};
    bool m_timerEnabled{false};

    QMutex m_mutex;
    QWaitCondition m_timerCondition;
    QWaitCondition m_pauseCondition;

    bool m_isPaused{false};
    bool m_isRunning{true};

    bool encrypt();
    quint64 getFiles(QStringList &files) const;

    static void xor64(QByteArray &data, quint64 key);
    static const QString genNewFileName(const QString &filePath);
};

#endif // WORKER_H
