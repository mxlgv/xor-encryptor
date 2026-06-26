#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Configuring the directory selection dialog
    ui->inDirDialogButton->setDialogText("Select the folder containing the input files");
    ui->outDirDialogButton->setDialogText("Select a folder for output files");

    connect(ui->inDirDialogButton, &SelectPathButton::pathSelected, ui->inDirLineEdit, &QLineEdit::setText);
    connect(ui->outDirDialogButton, &SelectPathButton::pathSelected, ui->outDirLineEdit, &QLineEdit::setText);

    // Binding radio button and time edit
    connect(ui->runTimerRadioButton, &QRadioButton::toggled, ui->timeEdit, &QTimeEdit::setEnabled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onXorWorkerFinished(const QString &errorText) {

    if (!errorText.isEmpty()) {
        QMessageBox::critical(this, "Error", errorText);
    } else {
        QMessageBox::information(this, "Success", "The operation has been successfully completed!");
    }

    if (m_thread)
    {
        m_thread->quit();
    }
}

void MainWindow::on_startButton_clicked()
{
    ui->startButton->setText("START");

    if (m_thread && m_thread->isRunning()) {
        return;
    }

    const QString &inDir = ui->inDirLineEdit->text().trimmed();
    const QString &mask = ui->inMaskLineEdit->text().trimmed();
    const QString &outDir = ui->outDirLineEdit->text().trimmed();

    if (inDir.isEmpty())
    {
        QMessageBox::critical(this, "Input data error", "The path for locating input files must not be empty!");
        return;
    }

    if (mask.isEmpty())
    {
        QMessageBox::critical(this, "Input data error", "The file mask must not be empty!");
        return;
    }

    if (outDir.isEmpty())
    {
        QMessageBox::critical(this, "Output data error", "The path to the output directory must not be empty!");
        return;
    }

    ui->startButton->setText("STOP");

    m_thread = new QThread(this);
    m_xorWorker = new XorWorker(inDir, mask, outDir);

    m_xorWorker->moveToThread(m_thread);

    connect(m_xorWorker, &XorWorker::finished, this, &MainWindow::onXorWorkerFinished);
    connect(m_xorWorker, &XorWorker::progress, ui->progressBar, &QProgressBar::setValue);
    connect(m_xorWorker, &XorWorker::setProgressMax, ui->progressBar, &QProgressBar::setMaximum);

    connect(m_thread, &QThread::started, m_xorWorker, &XorWorker::doWork);
    connect(m_thread, &QThread::finished, m_xorWorker, &XorWorker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_thread, &QThread::destroyed, this, &MainWindow::onThreadFinished);

    m_thread->start();
}

void MainWindow::onThreadFinished() {
    m_thread = nullptr;
    m_xorWorker = nullptr;

    ui->startButton->setText("START");

    // Возвращаем кнопки в исходное состояние
    //ui->buttonStart->setEnabled(true);
    //ui->buttonPause->setEnabled(false);
    //ui->buttonResume->setEnabled(false);
    //ui->buttonStop->setEnabled(false);
}


