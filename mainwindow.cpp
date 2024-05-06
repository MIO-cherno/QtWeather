#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QMenu>
#include<QContextMenuEvent>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//---------------------------------

    setWindowFlag(Qt::FramelessWindowHint);  // 设置无边框
    setFixedSize(width(), height());         // 设置固定窗口大小

    mExitMenu = new QMenu(this);
    mExitAct = new QAction();

    //mExitAct->setText(tr("退出"));
    mExitAct->setText(tr("退出"));
    mExitAct->setIcon(QIcon(":/res/close.png"));

    mExitMenu->addAction(mExitAct);

    connect(mExitAct,&QAction::triggered,this,[=](){qApp->exit(0);});
//--------------------------------

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    mExitMenu->exec(QCursor::pos());//当前鼠标位置"QCursor::pos()"传递，在此位置弹出菜单项
    event->accept();
}
