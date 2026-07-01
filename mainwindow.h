#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <worker.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startStopButton_clicked();
    void on_pauseButton_clicked();

private:
    void onThreadFinished();
    void onXorWorkerFinished(WorkerResult result, const QString &msg);
    void setGuiInputEnabled(bool enabledl);

    Ui::MainWindow *ui;
    Worker *m_xorWorker{nullptr};
    QThread *m_thread {nullptr};

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
