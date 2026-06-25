#include <QDir>
#include <QStringList>

#include "filescanner.h"

void FileScanner::setMask(const QString &mask)
{
    mask_m = mask;
}

void FileScanner::setPath(const QString &path)
{
    path_m = path;
}

void FileScanner::run()
{
    if (path_m.isEmpty())
    {
        emit error("The directory path is empty");
        return;
    }

    if (mask_m.isEmpty())
    {
        emit error("File mask is empty");
        return;
    }

    QDir dir(path_m);

    if (!dir.exists())
    {
        emit error("Directory does not exist: " + path_m);
        return;
    }

    QStringList filters;
    filters << mask_m;

    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    emit success(dir.entryList());
}
