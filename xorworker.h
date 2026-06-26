#ifndef XORWORKER_H
#define XORWORKER_H

#include <QString>
#include <QStringList>
#include <QThread>
#include <QDir>

class XorWorker : public QObject
{
    Q_OBJECT

public:
    XorWorker(const QString &inDir, const QString &mask, const QString &outDir);

    // Delete copy сtor
    XorWorker(const XorWorker&) = delete;
    XorWorker& operator=(const XorWorker&) = delete;

    //  Delet move сtor
    XorWorker(XorWorker&&) = delete;
    XorWorker& operator=(XorWorker&&) = delete;

    void doWork();

signals:
    void progress(int value);
    void setProgressMax(int max);
    void finished(const QString &errorText = QString());

private:
    QDir m_inDir;
    QDir m_outDir;
};

#endif // XORWORKER_H
