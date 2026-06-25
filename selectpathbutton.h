#ifndef SELECTPATHBUTTON_H
#define SELECTPATHBUTTON_H

#include <QToolButton>

class SelectPathButton : public QToolButton
{
    Q_OBJECT
signals:
    void pathSelected(const QString &path);

public:
    explicit SelectPathButton(QWidget *parent = nullptr);
    void setDialogText(const QString &dialogText);

private:
    QString dialogText_m;

private slots:
    void selectPath();
};

#endif // SELECTPATHBUTTON_H
