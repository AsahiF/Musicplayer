#include "mythread.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkReply>
#include <QThread>
myThread::myThread(QObject *parent) : QObject(parent)
{
    //初始化网络请求对象
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager,&QNetworkAccessManager::finished,this,&myThread::handleDataBackFunc);
}

void myThread::working(int musicId)
{
    qDebug()<<"musicId:"<<musicId;
    //用于获取音乐文件的直接播放链接
    QString requestURL;
    //requestURL = QString("http://m801.music.126.net/20240319205020/ca0179b83e63747ef80b9502b182a38e/jdymusic/obj/wo3DlMOGwrbDjj7DisKw/31721716170/9f28/3c36/df02/ae1dcd7d2e321996128ff6288f5dd6e5.mp3");
    requestURL = QString("https://music.163.com/song/media/outer/url?id=%0").arg(musicId);

    // 定义一个QNetworkRequest请求对象
    QNetworkRequest networkRequest;

    // 将请求格式设置给对应请求对象
    networkRequest.setUrl(QUrl(requestURL));
    // 设置请求头信息，模拟浏览器发送请求
    networkRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36 Edg/122.0.0.0");
    networkRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");

    qDebug()<<"url:"<<requestURL;
    //发送get请求
    networkManager->get(networkRequest);

}

void myThread::handleDataBackFunc(QNetworkReply *pReply)
{

    if(pReply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid())
    {

        //重定向地址
        QUrl redirectUrl = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "重定向地址：" << redirectUrl.toString();
        //向重定向地址发送get请求
        QNetworkRequest downloadRequest(redirectUrl);
        QNetworkReply *downloadReply = networkManager->get(downloadRequest);

        QObject::connect(downloadReply, &QNetworkReply::finished, [=]()
        {

            QFile file(saveFilePath);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(downloadReply->readAll());
                file.close();
                //QMessageBox::information(nullptr,"提示","歌曲下载完毕！");
                emit downloadOK();
                qDebug() << "下载完毕！";
            }
            else
            {
                emit downloadFalse();
                qDebug() << "无法打开文件进行写入";
            }
        });
    }


}
