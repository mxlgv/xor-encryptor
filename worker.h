#ifndef WORKER_H
#define WORKER_H

#include <QDir>
#include <QMutexLocker>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QWaitCondition>

enum class WorkerResult { OK, Warn, Error };

class Worker : public QObject
{
    Q_OBJECT

public:
    // General ctor
    Worker(const QString &inDir, const QString &outDir, const QString &mask, quint64 key);

    // Delete copy сtor
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    //  Delete move сtor
    Worker(Worker &&) = delete;
    Worker &operator=(Worker &&) = delete;

    // Worker settings:
    void enableDeleteInputFiles(bool enable);
    void enableRenamingOnConflict(bool enable);
    void setTimerInterval(QTime interval);
    void enableTimer(bool enable);

    // Thread control:
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
    // General
    QDir m_inDir;
    QDir m_outDir;
    quint64 m_key;

    bool m_renamingOnConflict{false};
    bool m_deleteInputFiles{false};

    // Timer mode:
    int m_timerInterval{0};
    bool m_timerEnabled{false};
    QWaitCondition m_timerCondition;

    // Thread control
    QMutex m_mutex;
    QWaitCondition m_pauseCondition;
    bool m_isPaused{false};
    bool m_isRunning{true};

    bool encrypt();

    // Helpers:
    quint64 getFiles(QStringList &files) const;

    static void xor64(QByteArray &data, quint64 key);
    static const QString genNewFileName(const QString &filePath);
};

#endif // WORKER_H
