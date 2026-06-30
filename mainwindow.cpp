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

    ui->statusbar->showMessage("User action expected.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onXorWorkerFinished(WorkerResult result, const QString& msg) {

    switch (result) {
    case WorkerResult::OK:
        QMessageBox::information(this, "Success", msg);
        break;
    case WorkerResult::Warn:
        QMessageBox::warning(this, "Warning", msg);
        break;
    case WorkerResult::Error:
        QMessageBox::critical(this, "Error", msg);
        break;
    default:
        break;
    }

    if (m_thread)
    {
        m_thread->quit();
    }

}

void MainWindow::on_startStopButton_clicked()
{
    setGuiInputEnabled(false);
    ui->startStopButton->setText("START");

    // Stop
    if (m_thread && m_thread->isRunning()) {    
        ui->startStopButton->setDisabled(true);
        m_xorWorker->stop();
        m_thread->quit();
        return;
    }

    const QString &inDir = ui->inDirLineEdit->text().trimmed();
    const QString &mask = ui->inMaskLineEdit->text().trimmed();
    const QString &outDir = ui->outDirLineEdit->text().trimmed();

    if (inDir.isEmpty())
    {
        QMessageBox::critical(this, "Input data error", "The path to the input directory must not be empty!");
        setGuiInputEnabled(true);
        return;
    }

    if (mask.isEmpty())
    {
        QMessageBox::critical(this, "Input data error", "The file mask must not be empty!");
        setGuiInputEnabled(true);
        return;
    }

    if (outDir.isEmpty())
    {
        QMessageBox::critical(this, "Output data error", "The path to the output directory must not be empty!");
        setGuiInputEnabled(true);
        return;
    }

    qDebug() << "XOR Key-str: " << ui->xorKeyLineEdit->displayText();

    bool keyConverted = false;
    quint64 xorKey = ui->xorKeyLineEdit->displayText().replace(" ", "") .toULongLong(&keyConverted, 16);
    if (!keyConverted){
        xorKey = 0;
    }

    qDebug() << "XOR Key: " << QString("%1").arg(xorKey , 0, 16);

    const WorkerSettings workerSettings = {
        .inDir = inDir,
        .mask = mask,
        .outDir = outDir,
        .renameOnConflict = ui->conflictComboBox->currentIndex() == 1,
        .delInFiles = ui->inDeleteCheckBox->isChecked(),
        .key = xorKey
    };

    ui->pauseButton->setDisabled(false);
    ui->startStopButton->setText("STOP");
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(0); // Infinite

    m_thread = new QThread(this);
    m_xorWorker = new Worker(workerSettings);

    m_xorWorker->moveToThread(m_thread);

    connect(m_xorWorker, &Worker::finished, this, &MainWindow::onXorWorkerFinished);
    connect(m_xorWorker, &Worker::progress, ui->progressBar, [this](int value) {
        ui->progressBar->setValue(value);
        ui->progressBar->setMaximum(100);
    });

    connect(m_thread, &QThread::started, m_xorWorker, &Worker::doWork);
    connect(m_thread, &QThread::finished, m_xorWorker, &Worker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_thread, &QThread::destroyed, this, &MainWindow::onThreadFinished);

    m_thread->start();

    ui->statusbar->showMessage("Encrypting files...");
}

void MainWindow::onThreadFinished() {
    m_thread = nullptr;
    m_xorWorker = nullptr;

    ui->startStopButton->setText("START");
    ui->startStopButton->setDisabled(false);
    ui->pauseButton->setDisabled(true);
    ui->progressBar->setValue(0);

    setGuiInputEnabled(true);

    ui->statusbar->showMessage("Ready");
}

void MainWindow::on_pauseButton_clicked()
{
    if (!m_xorWorker)
    {
        return;
    }

    if (!m_xorWorker->getPaused())
    {
        m_xorWorker->setPaused(true);
        ui->pauseButton->setText("СONTINUE");
        ui->statusbar->showMessage("Encryption paused");
        return;
    }

    m_xorWorker->setPaused(false);
    ui->pauseButton->setText("PAUSE");
    ui->statusbar->showMessage("Continuing file encryption...");
}

void MainWindow::setGuiInputEnabled(bool enabled)
{
    ui->inputGroupBox->setEnabled(enabled);
    ui->outputGroupBox->setEnabled(enabled);
    ui->generalGroupBox->setEnabled(enabled);
}
