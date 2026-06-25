#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->inputDirDialogButton->setDialogText("Select the folder containing the input files");
    ui->outputFilesPathDialogButton->setDialogText("Select a folder for output files");

    connect(ui->inputDirDialogButton, &SelectPathButton::pathSelected, ui->inputDirLineEdit, &QLineEdit::setText);
    connect(ui->outputFilesPathDialogButton, &SelectPathButton::pathSelected, ui->outputFilesPathLineEdit, &QLineEdit::setText);

    connect(ui->runTimerRadioButton, &QRadioButton::toggled, ui->timeEdit, &QTimeEdit::setEnabled);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_runButton_clicked()
{

}

