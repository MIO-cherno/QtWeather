#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//------------------------------mainwindow构造函数---------------------------------------------
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


    mNetAccessManger = new QNetworkAccessManager(this);

    connect(mNetAccessManger,&QNetworkAccessManager::finished,this,&MainWindow::GetReply);
    //第2参数转换其实是一个函数指针，类似下面的代码
    //void (QPushButton::*ff)(bool)=&QPushButton::clicked;
    //connect(ui->pushButton_2, ff, this, SLOT(pushButon2_clicked()));

    GetWeatherInfor("101010100");

//------------------------------mainwindow构造函数-----------------------------------------------
}

MainWindow::~MainWindow()
{
    delete ui;

}


//down from here is mycode
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
/*GetWeatherInfor 使用提供的城市代码来构建URL并发起GET请求
  QNetworkReply 类封装了使用 QNetworkAccessManager 发布的请求相关的回复信息。
*/
void MainWindow::GetWeatherInfor(QString CityCode)
{
    QUrl url("http://t.weather.itboy.net/api/weather/city/"+CityCode);

    /*将这个 QNetworkRequest 对象传递给 QNetworkAccessManager 的相关函数（如 get(), post(), put(), deleteResource() 等）
     * 来发送网络请求。这些函数将返回一个 QNetworkReply 对象，用于处理请求的响应。*/
    mNetAccessManger->get(QNetworkRequest(url));
}

void MainWindow::GetReply(QNetworkReply *reply)
{
    //qDebug()<<"success get";


    int StatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError || StatusCode != 200){
        qDebug() << reply->errorString().toUtf8().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }else{
        QByteArray  byteArray = reply->readAll();
        qDebug() << "读所有：" << byteArray.data();
        //AnalysisJson(byteArray);
    }
    reply->deleteLater();
}

//分析获取的json
void MainWindow::AnalysisJson(QByteArray &byteArray)
{
    QJsonParseError mError;
    /*将 JSON 格式的字节数组（QByteArray）解析为 QJsonDocument 对象
     *如果出错将出错信息存到mError中
    */
    QJsonDocument doc = QJsonDocument::fromJson(byteArray,&mError);
    if(mError.error != QJsonParseError::NoError){    // Json格式错误
        return;
    }
    //返回的信息是一个json对象，故调用object()
    QJsonObject rootObj = doc.object();

    //解析日期和城市
    //value("xxx")提取key键为"xxx"所对应的值
    mToday.date = rootObj.value("date").toString();
    //value() in #include <QJsonObject>
    mToday.city = rootObj.value("cityInfo").toObject().value("city").toString();
    int index = mToday.city.indexOf("市");
    QString result = mToday.city.left(index); // 取出 "市" 前面的子串
    mToday.city = result;

    //解析昨天
    QJsonObject DataObj = rootObj.value("data").toObject();//把data整个对象提取
    //将data中的yesterday整个对象提取
    QJsonObject objYesterday = DataObj.value("yesterday").toObject();

    //将提取的数据载入到mDay[0]中
    mDay[0].date = objYesterday.value("ymd").toString();
    mDay[0].week = objYesterday.value("week").toString();
    mDay[0].type = objYesterday.value("type").toString();
    //风向风力
    mDay[0].fx = objYesterday.value("fx").toString();
    mDay[0].fl = objYesterday.value("fl").toString();
    //空气质量指数
    mDay[0].aqi = objYesterday.value("aqi").toInt();

    QString s;
    //使用空格分割，取后面那部分
    /*"high": "高温 29℃",
      "low": "低温 15℃",*/
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length() - 1);//左到右，取length-1个，即去掉°C
    mDay[0].high = s.toInt();

    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length() - 1);
    mDay[0].low = s.toInt();

    //解析预报中的后5天数据
    QJsonArray forecatArr = DataObj.value("forecast").toArray();
    for(int i = 0;i < 5;i++){
        QJsonObject objForecast = forecatArr[i].toObject();
        mDay[i + 1].week = objForecast.value("week").toString();
        mDay[i + 1].date = objForecast.value("ymd").toString();
        //天气类型
        mDay[i + 1].type = objForecast.value("type").toString();

        QString s;
        s = objForecast.value("high").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        mDay[i + 1].high = s.toInt();

        s = objForecast.value("low").toString().split(" ").at(1);
        s = s.left(s.length() - 1);
        mDay[i + 1].low = s.toInt();

        //风向风力
        mDay[i + 1].fx = objForecast.value("fx").toString();
        mDay[i + 1].fl = objForecast.value("fl").toString();
        //空气质量指数
        mDay[i + 1].aqi = objForecast.value("aqi").toInt();
    }

    //解析今天的数据 窗口左边特有的
    mToday.ganmao = DataObj.value("ganmao").toString();
    mToday.wendu = DataObj.value("wendu").toString().toInt();
    mToday.shidu = DataObj.value("shidu").toString();
    mToday.pm25 = DataObj.value("pm25").toInt();
    mToday.quality = DataObj.value("quality").toString();
    //forecast 中的第一个数组元素，即今天的数据
    mToday.type = mDay[1].type;

    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;

    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;
}


