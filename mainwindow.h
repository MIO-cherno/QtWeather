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
    void UpdataUI();

private slots:
    void on_SearchButton_clicked();

private:
    void GetReply(QNetworkReply * Reply);

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
