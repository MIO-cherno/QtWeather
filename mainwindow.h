#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;
//SELF_START----------------------------------------------------------
protected:
    //右键菜单事件
    void contextMenuEvent(QContextMenuEvent * event);//重写父类的方法

private:
    QMenu * mExitMenu;//右键弹出菜单
    QAction * mExitAct;//菜单的一个选项为退出
//SELF_END------------------------------------------------------------
};
#endif // MAINWINDOW_H
