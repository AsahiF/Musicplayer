#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QNetworkAccessManager>
#include <QObject>

class myThread : public QObject
{
    Q_OBJECT
public:
    explicit myThread(QObject *parent = nullptr);

    void working(int musicId);
    void handleDataBackFunc(QNetworkReply *pReply);
signals:
    void finished();
    void downloadOK();
    void downloadFalse();


private:
    QNetworkAccessManager *networkManager;
    QString saveFilePath;//文件储存路径
public slots:
    void m_getFilePath(QString filepath)
    {
       saveFilePath = filepath;
    }


};

#endif // MYTHREAD_H
