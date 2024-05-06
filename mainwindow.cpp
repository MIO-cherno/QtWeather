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

    setWindowFlag(Qt::FramelessWindowHint);  // �����ޱ߿�
    setFixedSize(width(), height());         // ���ù̶����ڴ�С

    mExitMenu = new QMenu(this);
    mExitAct = new QAction();

    //mExitAct->setText(tr("�˳�"));
    mExitAct->setText(tr("�˳�"));
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
    mExitMenu->exec(QCursor::pos());//��ǰ���λ��"QCursor::pos()"���ݣ��ڴ�λ�õ����˵���
    event->accept();
}
