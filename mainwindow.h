#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QLabel>
#include <QMouseEvent>
#include <QNetworkAccessManager>//网络请求
#include <QNetworkReply>//网络接受
#include <QString>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>//?
//Json分析
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonObject>
#include <QMap>

#include <QPainter>

#include"weatherdata.h"
#include "weathertool.h"

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

//---------------------------------------
protected:
    void contextMenuEvent(QContextMenuEvent* event);

    //按住鼠标移动窗口
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

    //获取某个天气信息
    void GetWeatherInfor(QString CityCode);
    void AnalysisJson(QByteArray & byteArray);

    void weaType();
    void UpdateUI();

private slots:
    void on_SearchButton_clicked();
    /*
     * warring: Slots named on_foo_bar are error prone
     * 这个警告的出现，是因为我们在处理信号–槽关系时，是通过 ui designer中的"Go to slot" ，让程序自动生成。
     * 而这种自动生成的弱点就是也许有一天，你在 ui designer中改了控件的名字，但此时编译也不会报错。
     * 程序还是正常跑，编译也不提示错误。
     * 这样，控件就相当于连不到槽函数上去了，就失效了
    */

private:
    void GetReply(QNetworkReply * Reply);

    //重写父类的eventFilter方法
    bool eventFilter(QObject* watched,QEvent* event);

    void paintHighCurve();
    void paintLowCurve();

private:
    QMenu* mExitMenu;   // 退出菜单
    QAction* mExitAct;  // 菜单项（退出）

    QPoint mOffset;//窗口移动时，按下的鼠标位置与窗口左上角偏移量

    QNetworkAccessManager *mNetAccessManger;//网络请求

    Today mToday;
    Day   mDay[6];

    //星期和日期
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;

    //天气和图标
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIconList;

    // 空前质量指数
    QList<QLabel*> mAqiList;

    //风力风向
    QList<QLabel*> mFxList;
    QList<QLabel*> mFlList;

    //图标名称及图标路径
    QMap<QString,QString>mTypeMap;


//----------------------------------------

};
#endif // MAINWINDOW_H
