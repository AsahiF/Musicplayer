#ifndef DIALOG_H
#define DIALOG_H

#include "about.h"

#include <QDialog>

#include <QPainter>
#include <QRect>
#include <QMessageBox>
// 使用该类可以进行网络请求、获取响应、处理错误等操作
#include <QNetworkAccessManager>
// QNetworkReply是QNetworkAccessManager发送请求之后返回的响应类
#include <QNetworkReply>

// 添加多媒体播放器类
#include <QMediaPlayer>
// 添加多媒体播放列表类
#include <QMediaPlaylist>

#include <QTextBlock>
#include <QTimer>
#include <QFileDialog>

// 用于解析JSON数据时错误处理类
#include <QJsonParseError>
// 用于操作JSON数组的类
#include <QJsonObject>

#include <QLCDNumber>
#include <QJsonArray>


QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    QPixmap PixmapToRound(const QPixmap &src, int radius)
    {
        if (src.isNull()) {
            return QPixmap();
        }

        QPixmap pixmapa(src);
        QPixmap pixmap(radius,radius);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addEllipse(0, 0, radius, radius);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, radius, radius, pixmapa);

        return pixmap;
    }

    int get_musicId()
    {
        return musicId;
    }

protected:
    qreal m_rotationAngle;
    void paintEvent(QPaintEvent *event);
    //鼠标左键拖动窗口
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);


private slots:

    void on_pushButton_Exit_clicked();

    void on_pushButton_SearchSong_clicked();

    void on_pushButton_PlaySong_4_clicked();

    void on_pushButton_Min_clicked();

    void on_pushButton_PreviousSong_4_clicked();

    void on_pushButton_PlayNextSong_4_clicked();

    void on_horizontalSlider_PlayProgress_valueChanged(int value);

    void on_horizontalSlider_Volume_valueChanged(int value);

    void on_pushButton_SoundYesNo_clicked();



    void on_pushButton_state_change_clicked();

    void on_pushButton_playlist_clicked();

    void on_pushButton_lyrics_clicked();

    void on_pushButton_About_clicked();

public slots:
    //处理API接口返回的数据
    void handleDataBackFunc(QNetworkReply* pReply);
    //处理歌词数据
    void handleDataBackFunc_lyrics(QNetworkReply* pReply);
signals:
    void starting(int musicId);//开启歌曲下载信号
    void sendFilePath(QString filepath);//发送文件路径

private:
    Ui::Dialog *ui;
    QPainter p;

    //鼠标左键拖动窗口
    bool moveflag = false;
    QPoint mouse_start_point;
    QPoint win_start_point;

    //网络请求
    QNetworkAccessManager *networkAccessManagers;
    QNetworkAccessManager *lyrics_networkAccessManagers;

    //多媒体
    QMediaPlayer *p_PlayerObject; // 定义多媒体播放器
    QMediaPlaylist *p_PlayerList; // 定义多媒体播放列表

    QTextDocument *docTextObject; // 处理富文本内容

    QByteArray m_QByteArySearchInfo; // 存储搜索返回数据信息
    QByteArray lyrics_QByteArySearchInfo; // 存储搜索返回数据信息

    //歌曲相关信息
    int musicId;
    QString musicName;
    QString singerName;
    int fee;//vip相关歌曲
    QString lyrics;

    //歌曲播放情况
    bool isPlay = false;

    //动态歌曲信息
    QString titleMSG;
    int titleChar;//表示当前标题移动到的字符位置，初始值为0

    //黑胶唱片旋转角度
    int angle;
    //歌曲播放模式
    int songState;

    About *about;

enum
{
    LOOP,
    ONELOOP,
    SEQUENCE,
    RANDOM
};

};
#endif // DIALOG_H
