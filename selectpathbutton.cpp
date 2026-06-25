#include <QFileDialog>

#include "selectpathbutton.h"

SelectPathButton::SelectPathButton(QWidget *parent)
    : QToolButton(parent)
{
    connect(this, &QToolButton::clicked, this, &SelectPathButton::selectPath);
}

void SelectPathButton::selectPath() {
    QString dirPath = QFileDialog::getExistingDirectory(this, dialogText_m, "");
    if (!dirPath.isEmpty()) {
        emit pathSelected(dirPath);
    }
}

void SelectPathButton::setDialogText(const QString &dialogText)
{
    dialogText_m = dialogText;
}
