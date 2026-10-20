#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("计算器-吴湘楠-2024414290333"));

    // 显示框只读，靠右对齐，初始显示 0
    ui->editDisplay->setReadOnly(true);
    ui->editDisplay->setText(QStringLiteral("0"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
