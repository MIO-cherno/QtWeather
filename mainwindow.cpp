#include "mainwindow.h"
#include "ui_mainwindow.h"

#define INCREMENT 2     //温度每升高或降低1度，y轴坐标的增量
#define POINT_RADIUS 3  //曲线秒点的大小
#define TEXT_OFFSET_X 9    //温度文本相对于x点的偏移
#define TEXT_OFFSET_Y 7    //温度文本相对于y点的偏移


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//------------------------------mainwindow构造函数↓↓↓↓↓↓↓---------------------------------------------
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

    weaType();

    /* 大致流程
     * 先在mainwindow类中创建 mNetAccessManger 网络请求对象，
     * 然后GetWeatherInfor方法通过mNetAccessManger向api接口发送get请求
     * mNetAccessManger请求完成（QNetworkAccessManager::finished）返回QNetworkReply
     * GetReply接收QNetworkReply并对接收的json数据进行分析（AnalysisJson方法）
     * AnalysisJson方法分析完成后又调用Update方法更新ui
     * 最后是绘制高低温曲线
    */

    //网络请求
    mNetAccessManger = new QNetworkAccessManager(this);
    GetWeatherInfor("西安");//默认西安市
    connect(mNetAccessManger,&QNetworkAccessManager::finished,this,&MainWindow::GetReply);
    //mNetAccessManger()是通过GetWeatherInfor()来获取信息的
    //第2参数转换其实是一个函数指针，类似下面的代码
    //void (QPushButton::*ff)(bool)=&QPushButton::clicked;
    //connect(ui->pushButton_2, ff, this, SLOT(pushButon2_clicked()));



    //给标签添加事件过滤器
    ui->labelHighPoint->installEventFilter(this);
    ui->labelLowPoint->installEventFilter(this);
    /* 事件过滤器（Event Filter）是一种强大的机制，允许一个对象（称为事件过滤器）接收并处理另一个对象的事件。
     * 这通常用于在事件到达目标对象之前拦截并修改它们。
     * 确保你的类继承了QObject或它的子类,   * eventFilter()是QObject类的一个虚方法。
     *
     * targetWidget->installEventFilter(filter);
     * QWidget 指针 targetWidget 和一个 MyEventFilter 类的实例 filter
    */


    /* 绘制温度曲线大致流程：
     * 1.调用标签的update方法
     * 2.框架发送QEvent：：Paint事件给标签
     * 3.事件被maiwindow拦截，进而调用其eventFilter方法
     * 4.在eventFilter中，调用paintHighCurve和paintLowCurve来绘制曲线
    */

//------------------------------mainwindow构造函数↑↑↑↑↑-----------------------------------------------
}

MainWindow::~MainWindow()
{
    delete ui;

}
//down from here is mycode

//WeatherTool的静态成员mCityMaps实现
//建议在cpp文件中定义，而在.h文件中声明，否则如果有多个cpp都包含哪个头文件，即使有包含守卫（Include Guards）也会报错
//如果在头文件中声明一个静态变量，并将该头文件包含在两个不同的c++文件中，就是在两个翻译单元中都声明了相同的静态变量
//因为mainwindow.cpp 和 main.pp都包含mainwindow.h,而mainwindow.h是包含weathertool.h的，所以会报错
QMap<QString,QString> WeatherTool::mCityMaps = {};//静态成员要在类外初始化


/*===================================================================================================*/
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
/*===================================================================================================*/
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-mOffset);
    //减去窗口与鼠标的偏移量，否则窗口的左上角会移动到鼠标当前位置，与预期不符
}

/*===================================================================================================*/
/*GetWeatherInfor 使用提供的城市代码来构建URL并发起GET请求
  QNetworkReply 类封装了使用 QNetworkAccessManager 发布的请求相关的回复信息。
*/
void MainWindow::GetWeatherInfor(QString CityName)
{
    QString CityCode = WeatherTool::getCityCode(CityName);

    if(CityCode.isEmpty()){
        QMessageBox::warning(this,"天气搜索","请检查输入是否正确！",QMessageBox::Ok);
        return;
    }

    QUrl url("http://t.weather.itboy.net/api/weather/city/"+CityCode);

    /*将这个 QNetworkRequest 对象传递给 QNetworkAccessManager 的相关函数（如 get(), post(), put(), deleteResource() 等）
     * 来发送网络请求。这些函数将返回一个 QNetworkReply 对象，用于处理请求的响应。*/
    mNetAccessManger->get(QNetworkRequest(url));
}
/*===================================================================================================*/
void MainWindow::GetReply(QNetworkReply *reply)
{
    //qDebug()<<"success get";


    int StatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError || StatusCode != 200){
        qDebug() << reply->errorString().toUtf8().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }else{
        QByteArray  byteArray = reply->readAll();
        /*byteArray 是在GetReply的栈上创建的，json的数据存储在此处，出了函数byteArray会销毁
         * 所以要将UpdateUI（）放在AnalysisJson（）中，否则信息丢失，起不到更新的作用
        */
        qDebug() << "读所有：" << byteArray.data();
        AnalysisJson(byteArray);
    }
    reply->deleteLater();
}


/*===================================================================================================*/
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
    //int index = mToday.city.indexOf("市");
    //QString result = mToday.city.left(index); // 取出 "市" 前面的子串
    QString result = mToday.city;
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
    mToday.wendu = (int) DataObj.value("wendu").toString().toDouble();//可能会出现小数情况，直接toInt会返回0
    mToday.shidu = DataObj.value("shidu").toString();
    mToday.pm25 = DataObj.value("pm25").toInt();
    mToday.quality = DataObj.value("quality").toString();
    //forecast 中的第一个数组元素，即今天的数据
    mToday.type = mDay[1].type;

    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;

    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;


    UpdateUI();
    //搜索时更新最高最低温度曲线
    /* 调用时会除法paint事件，然后被当前调用update（）的窗口（mainwindow）拦截，
     * 会通过过滤器调用eventFilter（），才会实际进行一次绘制曲线
     *
    */
    ui->labelHighPoint->update();
    ui->labelLowPoint->update();
}
/*===================================================================================================*/
void MainWindow::weaType()
{

    //将控件添加到控件数组
    //是在设计ui是给空间起的名字
    mWeekList << ui->labelWeek0 << ui->labelWeek1 << ui->labelWeek2 << ui->labelWeek3
              << ui->labelWeek4 << ui->labelWeek5;
    mDateList << ui->labelDate0 << ui->labelDate1 << ui->labelDate2 << ui->labelDate3
              << ui->labelDate4 << ui->labelDate5;

    mTypeList << ui->labelType0 << ui->labelType1 << ui->labelType2 << ui->labelType3
                  << ui->labelType4 << ui->labelType5;
    mTypeIconList << ui->labelTypeIcon0 << ui->labelTypeIcon1 << ui->labelTypeIcon2
                  << ui->labelTypeIcon3 << ui->labelTypeIcon4 << ui->labelTypeIcon5;

    mAqiList << ui->labelQ0 << ui->labelQ1 << ui->labelQ2 << ui->labelQ3
             << ui->labelQ4 << ui->labelQ5;

    mFxList << ui->label_fx0 << ui->label_fx1 << ui->label_fx2 << ui->label_fx3
            << ui->label_fx4 << ui->label_fx5;
    mFlList << ui->label_fl0 << ui->label_fl1 << ui->label_fl2 << ui->label_fl3
            << ui->label_fl4 << ui->label_fl5;


    //天气对应图标
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到暴雪",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到大暴雪",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
}
/*===================================================================================================*/
void MainWindow::UpdateUI()
{
    ui->NowDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")
                         +" "+mDay[1].week);
    ui->NowPlace->setText(mToday.city);

    //更新今天
    ui->TypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->NowTemp->setText(QString::number(mToday.wendu) + "°");
    ui->NowWeather->setText(mToday.type);
    ui->UpLowTemp->setText(QString::number(mToday.low) + "~"
                            + QString::number(mToday.high) + "°C");

    ui->labelTip->setText("感冒指数：" + mToday.ganmao);
    ui->nowfx->setText(mToday.fx);
    ui->nowfl->setText(mToday.fl);

    ui->pmlevel->setText(QString::number(mToday.pm25));

    ui->humidlevel->setText(mToday.shidu);
    ui->airqulitylevel->setText(mToday.quality);

    //更新六天的数据
    for(int i = 0;i < 6;i++){
        //更新日期和时间
        mWeekList[i]->setText("星期" + mDay[i].week.right(1));
        ui->labelWeek0->setText("昨天");
        ui->labelWeek1->setText("今天");
        ui->labelWeek2->setText("明天");
        QStringList ymdList = mDay[i].date.split("-");//split分割
        mDateList[i]->setText(ymdList[1] + "/" + ymdList[2]);

        //更新天气类型
        mTypeList[i]->setText(mDay[i].type);
        mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);

        //更新空气质量
        if(mDay[i].aqi >=0 && mDay[i].aqi <= 50){
            mAqiList[i]->setText("优");
            mAqiList[i]->setStyleSheet("background-color: rgb(92, 255, 138);");
        }else if(mDay[i].aqi > 50 && mDay[i].aqi <= 100){
            mAqiList[i]->setText("良");
            mAqiList[i]->setStyleSheet("background-color: rgb(228, 255, 90);");
        }else if(mDay[i].aqi > 100 && mDay[i].aqi <= 150){
            mAqiList[i]->setText("轻度");
            mAqiList[i]->setStyleSheet("background-color: rgb(255, 199, 67);");
        }else if(mDay[i].aqi > 150 && mDay[i].aqi <= 200){
            mAqiList[i]->setText("中度");
            mAqiList[i]->setStyleSheet("background-color: rgb(255,17,27);");
        }else if(mDay[i].aqi > 200 && mDay[i].aqi <= 300){
            mAqiList[i]->setText("重度");
            mAqiList[i]->setStyleSheet("background-color: rgb(210,0,0);");
        }else{
            mAqiList[i]->setText("严重");
            mAqiList[i]->setStyleSheet("background-color: rgb(110,0,0);");
        }

        //更新风力、风向

        mFxList[i]->setText(mDay[i].fx);
        mFlList[i]->setText(mDay[i].fl);
    }

}
/*===================================================================================================*/
//点击搜索按钮
void MainWindow::on_SearchButton_clicked()
{
    QString CityName = ui->SearchGet->text();
    if(CityName == ""){
        return;
    }
    GetWeatherInfor(CityName);
}
/*===================================================================================================*/
/* watched：指向你正在监视的对象的指针。
 * event：指向要过滤的事件的指针。
 * 如果函数返回 true，则事件已被处理，并且不会传递给 watched 对象。
 * 如果返回 false，则事件将被正常地传递给 watched 对象。*/
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //绘制最高温度曲线
    if(watched == ui->labelHighPoint && event->type() == QEvent::Paint){
        paintHighCurve();
    }
    //绘制最低温度曲线
    if(watched == ui->labelLowPoint && event->type() == QEvent::Paint){
        paintLowCurve();
    }
    //其他控件做默认处理，所以要调用父类的方法
    return QWidget::eventFilter(watched,event);

}
/*===================================================================================================*/
void MainWindow::paintHighCurve()
{
    QPainter painter(ui->labelHighPoint);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    //获取x坐标
    //在显示日期的控件的对称轴位置
    int pointX[6] = {0};
    for(int i = 0;i < 6;i++){
        pointX[i] = mWeekList[i]->pos().x() + mWeekList[i]->width() / 2;
    }

    //获取y坐标
    //平均值为中间位置，>avg
    //当天温度与6天温度平均值差值，差值>0，在控件的上下对称轴上方，反之在下方
    int tmpSum = 0;
    int tmpAvg = 0;//6天气温平均值
    for(int i = 0;i < 6;i++){
        tmpSum += mDay[i].high;
    }
    tmpAvg = tmpSum / 6;
    int pointY[6] = {0};
    int yCenter = ui->labelHighPoint->height() / 2;//控件的上下对称轴
    for(int i = 0;i < 6;i++){
        pointY[i] = yCenter - ((mDay[i].high - tmpAvg) * INCREMENT);
    }

    //绘制
    //初始化画笔相关
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255,170,80));
    painter.setPen(pen);
    //上面只画了轮廓
    painter.setBrush(QColor(255 ,170,80));    //设置画刷内部填充的颜色

    //画点、写文本
    for(int i = 0;i < 6;i++){
        //画点，参数为：x坐标、y坐标，x半轴长度，y半轴长度
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        //显示温度文本
        painter.drawText(pointX[i] - TEXT_OFFSET_X,pointY[i] - TEXT_OFFSET_Y,
                         QString::number(mDay[i].high) + "°");
    }
    //连线
    for(int i = 0;i < 5;i++){
        //昨天到今天为虚线
        if(i == 0){
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
        }else{
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        //画线
        painter.drawLine(pointX[i],pointY[i],pointX[i + 1],pointY[i + 1]);

    }
}
/*===================================================================================================*/
void MainWindow::paintLowCurve()
{
    QPainter painter(ui->labelLowPoint);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    //获取x坐标
    //在显示日期的控件的对称轴位置
    int pointX[6] = {0};
    for(int i = 0;i < 6;i++){
        pointX[i] = mWeekList[i]->pos().x() + mWeekList[i]->width() / 2;
    }

    //获取y坐标
    //平均值为中间位置，>avg
    //当天温度与6天温度平均值差值，差值>0，在控件的上下对称轴上方，反之在下方
    int tmpSum = 0;
    int tmpAvg = 0;//6天气温平均值
    for(int i = 0;i < 6;i++){
        tmpSum += mDay[i].low;
    }
    tmpAvg = tmpSum / 6;
    int pointY[6] = {0};
    int yCenter = ui->labelLowPoint->height() / 2;//控件的上下对称轴
    for(int i = 0;i < 6;i++){
        pointY[i] = yCenter - ((mDay[i].low - tmpAvg) * INCREMENT);
    }

    //绘制
    //初始化画笔相关
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(100,255,255));
    painter.setPen(pen);
    //上面只画了轮廓
    painter.setBrush(QColor(100 ,255,255));    //设置画刷内部填充的颜色

    //画点、写文本
    for(int i = 0;i < 6;i++){
        //画点，参数为：x坐标、y坐标，x半轴长度，y半轴长度
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        //显示温度文本
        painter.drawText(pointX[i] - TEXT_OFFSET_X,pointY[i] - TEXT_OFFSET_Y,
                         QString::number(mDay[i].low) + "°");
    }
    //连线
    for(int i = 0;i < 5;i++){
        //昨天到今天为虚线
        if(i == 0){
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
        }else{
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
            //画线
        painter.drawLine(pointX[i],pointY[i],pointX[i + 1],pointY[i + 1]);

    }
}

/*===================================================================================================*/
