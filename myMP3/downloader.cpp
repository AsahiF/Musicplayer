#include "downloader.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDebug>

Downloader::Downloader(QObject *parent) : QObject(parent) {}

void Downloader::downloadMusic(const QString& urlStr) {
    QUrl url(urlStr);
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            QFile file("downloaded_song.mp3");
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                qDebug() << "Download completed.";
            } else {
                qDebug() << "Error: Failed to open the file for writing.";
            }
        } else {
            qDebug() << "Error: " << reply->errorString();
        }

        reply->deleteLater();
        emit finished();
    });
}
