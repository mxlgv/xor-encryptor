#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <QString>
#include <QStringList>
#include <QThread>
#include <QDir>

class FileScanner : public QThread
{
    Q_OBJECT

public:
    FileScanner(const FileScanner&) = delete;
    FileScanner& operator=(const FileScanner&) = delete;

    static FileScanner& getGlobal() {
        static FileScanner global;
        return global;
    }

    void setPath(const QString &path);
    void setMask(const QString &mask);

signals:
    void success(const QStringList &files);
    void error(const QString &errorText);

protected:
    void run() override;

private:
    FileScanner() {};
    QString path_m;
    QString mask_m;
};

#endif // FILESCANNER_H
