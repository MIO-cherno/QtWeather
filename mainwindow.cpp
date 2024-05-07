#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QMenu>
#include<QContextMenuEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//---------------------------------------------------------------------------
    //设置窗口属性
    setWindowFlag(Qt::FramelessWindowHint);  // 设置无边框
    setFixedSize(width(), height());         // 设置固定窗口大小

    //构建右键菜单
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText(tr("退出"));
    mExitAct->setIcon(QIcon(":/res/close.png"));
    mExitMenu->addAction(mExitAct);
    //用lamba函数连接退出按钮，触发就退出该应用
    connect(mExitAct, &QAction::triggered, this, [=]() { qApp->exit(0); });
//-----------------------------------------------------------------------------
}

MainWindow::~MainWindow()
{
    delete ui;

}

//重写父类虚函数
//父类中的默认实现是忽略右键菜单时间，重写后就可以
void MainWindow::contextMenuEvent(QContextMenuEvent * event)
{
    //弹出右键菜单
    mExitMenu->exec(QCursor::pos());//当前鼠标位置"QCursor::pos()"传递，在此位置弹出菜单项
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffset = event->globalPos()-this->pos();
    //event->globalPos() 来获取鼠标指针在屏幕上的全局位置。
    /*this->pos() 返回的是这个窗口部件在其父窗口部件中的位置（或称为“坐标”）。
     * 这个位置也是一个 QPoint 对象，包含了窗口部件左上角的 x 和 y 坐标（相对于其父窗口部件）
        这里时计算鼠标相对于mainwindow的坐标*/

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-mOffset);
    //减去窗口与鼠标的偏移量，否则窗口的左上角会移动到鼠标当前位置，与预期不符
}
