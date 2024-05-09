#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include <QMap>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>


class WeatherTool{

private:
    static QMap<QString,QString> mCityMaps;

    static void initCityMap(){
        //读取文件
        QString filePath = "D:\\Dev\\QtProject\\MyWeather\\citycode-2019-08-23.json";    //"\" or "/" ?
        QFile file(filePath);

        file.open(QIODevice::ReadOnly | QIODevice::Text);//?

        QByteArray json = file.readAll();
        file.close();


        //解析，写入到map中
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json,&err);
        //将 JSON 格式的字符串解析为 QJsonDocument 对象。这个对象可以进一步用于访问和修改 JSON 数据

        if(err.error != QJsonParseError::NoError){//获取出错直接return
            return;
        }
        if(!doc.isArray()){//内容出错直接return
            return;
        }
        QJsonArray cities = doc.array();
        for(int i = 0;i <cities.size();i++){
            QString city = cities[i].toObject().value("city_name").toString();
            QString code = cities[i].toObject().value("city_code").toString();
            if(code.size() > 0){//有些县城、省没有code
                mCityMaps.insert(city,code);
            }
        }
    }

public:
    static QString getCityCode(QString cityName){
        if(mCityMaps.isEmpty()){
            initCityMap();
        }
        QMap<QString,QString>::iterator it = mCityMaps.find(cityName);
        /*find() 函数接受一个键作为参数，并返回一个迭代器，该迭代器指向与给定键关联的元素（如果存在的话）。
         * 如果未找到该键，则返回 end() 迭代器。*/
        //
        if(it == mCityMaps.end()){
            it = mCityMaps.find(cityName + "市");
        }
        if(it != mCityMaps.end()){
            return it.value();
        }
        return "";
    }
};

//QMap<QString,QString> WeatherTool::mCityMaps={};//静态成员要在类外初始化

#endif // WEATHERTOOL_H
