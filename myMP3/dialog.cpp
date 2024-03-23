#include "dialog.h"
#include "mythread.h"
#include "ui_dialog.h"
#include <QAction>
#include <QDebug>
#include <QUrlQuery>
#include <QRegion>
#include <QGraphicsDropShadowEffect>
#include <QBitmap>
#include <QPropertyAnimation>
#include <QWidgetAction>
#include <QThread>
#include <QtCore/qmath.h>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    //去掉标题框
    this->setWindowFlag(Qt::FramelessWindowHint);
    //设置窗口大小不变
    this->setFixedSize(this->width(),this->height());
    setStyleSheet("QWidget { border-radius: 5px;}");
    // 设置窗口为圆角
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(), 10, 10);
    setMask(bmp);


    auto clearAction = new QAction;
    clearAction->setIcon(QIcon(":/images/clean.png"));
    // QLineEdit::TrailingPosition表示将action放置在右边
    ui->lineEdit_InputSongs->addAction(clearAction, QLineEdit::TrailingPosition);
    connect(clearAction, &QAction::triggered, ui->lineEdit_InputSongs,[=]{ ui->lineEdit_InputSongs->setText(""); });


    // 播放进度条控件美化
    ui->horizontalSlider_PlayProgress->setStyleSheet(
                "QSlider::groove:horizontal {"
                "border:1px solid skyblue;"
                "background-color:skyblue;"
                "height:6px;"
                "border-radius:3px;"
                "}"

                "QSlider::handle:horizontal {"
                "background:qradialgradient(spread:pad,cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,stop:0.7 white,stop:0.8 rgb(140,212,255));"
                "width:10px;"
                "border-radius:5px;"
                "margin-top:-2px;"
                "margin-bottom:-2px;}"

                "horizontalSlider::sub-page:horizontal{"
                "margin:5px;"
                "border-radius:5px;}"
                );

    ui->horizontalSlider_Volume->setStyleSheet(
                "QSlider::groove:horizontal {"
                "border:1px solid skyblue;"
                "background-color:skyblue;"
                "height:6px;"
                "border-radius:3px;"
                "}"

                "QSlider::handle:horizontal {"
                "background:qradialgradient(spread:pad,cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,stop:0.7 white,stop:0.8 rgb(140,212,255));"
                "width:10px;"
                "border-radius:5px;"
                "margin-top:-2px;"
                "margin-bottom:-2px;}"

                "horizontalSlider::sub-page:horizontal{"
                "background:red;"
                "margin:5px;"
                "border-radius:5px;}"
                );
    //创建子线程对象
    QThread *t = new QThread;
    //创建任务对象
    myThread *download = new myThread;
    //将任务对象移动到线程对象
    download->moveToThread(t);
    //启动子线程
    connect(this,&Dialog::starting,download,&myThread::working);

    //下载按钮
    connect(ui->pushButton_download,&QPushButton::clicked,this,[=]()
    {
        if(p_PlayerList->isEmpty())
        {
               QMessageBox::warning(this, "警告", "没有选择歌曲！");
               return;
        }
        QString saveFilePath = QFileDialog::getSaveFileName(nullptr, "Save File As", QDir::homePath()+ "/" + musicName, "MP3 files (*.mp3)");
        connect(this,&Dialog::sendFilePath,download,&myThread::m_getFilePath);
        if(saveFilePath.isEmpty())
        {
            return;
        }
        emit sendFilePath(saveFilePath);
        emit starting(musicId);
        t->start();
    });

    connect(download,&myThread::downloadOK,this,[=](){
        QMessageBox::information(nullptr,"提示","下载完毕！");
    });

    connect(download,&myThread::downloadFalse,this,[=](){
        QMessageBox::critical(nullptr,"错误","下载失败！");
    });

    //将鼠标聚焦到音乐搜索栏
    ui->lineEdit_InputSongs->setFocus();

    //初始化网络请求
    networkAccessManagers = new QNetworkAccessManager(this);
    lyrics_networkAccessManagers = new QNetworkAccessManager(this);
    //处理富文本的对象
    docTextObject = ui->plainTextEdit_SongList->document();
    ui->plainTextEdit_SongList->setReadOnly(true);
    ui->plainTextEdit_lyrics->setReadOnly(true);

    //初始化多媒体实例
    p_PlayerObject = new QMediaPlayer(this);//音乐播放器对象
    p_PlayerList = new QMediaPlaylist(this);//音乐播放列表


    //设置播放列表
    p_PlayerObject->setMedia(p_PlayerList);
    //设置播放模式
    songState = 0;
    p_PlayerList->setPlaybackMode(QMediaPlaylist::Loop);


    //处理get请求后返回的数据
    connect(networkAccessManagers,SIGNAL(finished(QNetworkReply*)),this,SLOT(handleDataBackFunc(QNetworkReply*)));//歌曲信息
    connect(lyrics_networkAccessManagers,SIGNAL(finished(QNetworkReply*)),this,SLOT(handleDataBackFunc_lyrics(QNetworkReply*)));//歌词

    //处理标题显示歌曲信息动画字幕
    ui->label_DisplaySongTitle->setFont(QFont("宋体",18,QFont::Normal));
    titleMSG = ui->label_DisplaySongTitle->text();


    //处理进度条和LCD时间
    //positionChanged 发送当前进度条读取的总长度
    //durationChanged 当前进度条总长度改变时发送改变后的进度条总长度
    //处理LCD时间随播放位置改变
    connect(p_PlayerObject,&QMediaPlayer::positionChanged,this,[=](qint64 duration)
    {
        int int_second = duration/1000;//将毫秒转换为秒
        int int_minute = int_second/60;//将秒转换为分
        int_second = int_second % 60;//分钟分离后剩余的秒数
        //将格式改为mm:ss
        QString songTime = QString::asprintf("%02d:%02d",int_minute,int_second);
        //将格式化的字符串显示在LCD屏幕上
        ui->lcdNumber_PlayTime->display(songTime);
    });

    //处理进度条随播放位置改变
    connect(p_PlayerObject,&QMediaPlayer::positionChanged,this,[=](qint64 position)
    {
        //用户在拖动进度条的话不做处理
        if(ui->horizontalSlider_PlayProgress->isSliderDown())
            return;

        //音乐播放时，设置滑块位置随音乐移动
        ui->horizontalSlider_PlayProgress->setSliderPosition(position);
    });

    //歌曲切换时请求该歌曲的歌词并显示在歌词栏中
    connect(p_PlayerObject, &QMediaPlayer::currentMediaChanged, [=](const QMediaContent &media)
    {
        QString urlStr;
        urlStr = QString(media.canonicalUrl().toString());

        QUrl url(urlStr);
        QUrlQuery query(url);
        musicId = query.queryItemValue("id").toInt();



        //请求歌词信息
        QString lyrics_requestURL;
        lyrics_requestURL = QString("http://music.163.com/api/song/lyric?os=pc&id=%0&lv=-1&kv=-1&tv=-1").arg(musicId);

        // 定义一个QNetworkRequest请求对象
        QNetworkRequest networkRequest_lyrics;

        // 将请求格式设置给对应请求对象
        networkRequest_lyrics.setUrl(lyrics_requestURL);

        // 使用QNetworkAccessManager类对应的API发送GET请求并获取响应数据
        lyrics_networkAccessManagers->get(networkRequest_lyrics); // 请求处理


        //qDebug() << "Song ID: " << songId;
        // 在这里处理歌曲改变时的逻辑
        //qDebug() << "Media changed to: " << urlStr;
    });

    //切换歌曲时改变进度条总长度，并更新滚动歌曲信息
    connect(p_PlayerObject,&QMediaPlayer::durationChanged,this,[=](qint64 duration)
    {
        //设置进度调最大值，确保进度条的长度可以完整地表示整首歌曲的时间范围
        ui->horizontalSlider_PlayProgress->setMaximum(duration);
        //获取当前歌曲在播放列表中的索引值
        int int_value = p_PlayerList->currentIndex();
        //根据索引值获取文档中对应的文本块
        QTextBlock qtextBlocks = docTextObject->findBlockByNumber(int_value);
        //从文本块中获取歌曲标题信息
        QString title = qtextBlocks.text();
        musicName = title;
        //设置滚动歌曲标题信息
        ui->label_DisplaySongTitle->setText(title);


    });



    // TODO:实现用户点击进度条时改变进度条位置

    //双击搜索列表，将搜索列表中选中的歌曲添加到音乐播放列表
    connect(ui->listWidget_searchList,&QListWidget::itemDoubleClicked,this,[=](QListWidgetItem* item)
    {

        // 将提取的字符串转换为整数
        musicId = item->data(Qt::UserRole).toInt();

        //用于获取音乐文件的直接播放链接
        QString requestURL;
        requestURL = QString("https://music.163.com/song/media/outer/url?id=%0").arg(musicId);

        //将获取的歌曲添加到播放列表
        // 将歌曲倒序插入到音乐播放列表
        p_PlayerList->insertMedia(0, QMediaContent(QUrl(requestURL)));
        //p_PlayerList->addMedia(QUrl(requestURL));


        //立即播放刚添加的歌曲
        int m_index = p_PlayerList->currentIndex();
        //m_index += 1;
        QPixmap on(":/images/pause.png");
        ui->pushButton_PlaySong_4->setIcon(on);
        p_PlayerList->setCurrentIndex(m_index - 1);
        p_PlayerObject->play();
        isPlay = true;


        QString currentText = ui->plainTextEdit_SongList->toPlainText();
        ui->plainTextEdit_SongList->setText(item->text() + "\n" + currentText);

    });



    //使用定时器移动标题
    QTimer *p_moveTitle = new QTimer(this);
    titleChar = 0;
    connect(p_moveTitle,&QTimer::timeout,this,[=]()
    {
        titleMSG = ui->label_DisplaySongTitle->text();
        if(titleChar < titleMSG.length())
        {
            QString temp = titleMSG.mid(titleChar) + titleMSG.mid(0,titleChar);
            ui->label_DisplaySongTitle->setText(temp);
            titleChar++;
        }
        else
            titleChar = 0;
    });
    p_moveTitle->start(800);//800ms滚动一次

    QPixmap image(":/images/1.png");
    image = PixmapToRound(image,40);//将图片变为圆形

    // 在 QLabel 中显示圆形图片
     ui->record->setPixmap(image);

     ui->record->setAlignment(Qt::AlignCenter);


    QTimer *timer = new QTimer(this);
    // 创建一个定时器
    connect(timer, &QTimer::timeout, this, [=]()
    {
        static qreal rotationAngle = 0.0;
        QPixmap rotatedImage(image.size());
        rotatedImage.fill(Qt::transparent);

        QPainter painter(&rotatedImage);
        painter.setRenderHint(QPainter::Antialiasing);

        QPointF center = QPointF(image.width() / 2, image.height() / 2); // 设置旋转中心为图片中心点
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(rotationAngle);
        transform.translate(-center.x(), -center.y());

        painter.setTransform(transform);
        painter.drawPixmap(0, 0, image);

        ui->record->setPixmap(rotatedImage);
        if(isPlay)
        {
            rotationAngle += 0.2; // 每次旋转增加的角度
        }
        if (rotationAngle >= 360.0)
        {
            rotationAngle = 0.0; // 重置角度
        }


    });

    // 启动定时器，每隔一段时间进行图片旋转
    timer->start(20); // 500毫秒间隔




}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        moveflag = true;
        win_start_point = this->frameGeometry().topLeft();//获取窗口左上角的坐标
        mouse_start_point = event->globalPos();//获取鼠标当前坐标
//        qDebug()<<"mouse start: "<<mouse_start_point;
//        qDebug()<<"win start: "<<win_start_point;
    }
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(moveflag)
    {
        QPoint distance = mouse_start_point - event->globalPos();//鼠标移动距离
        this->move(win_start_point - distance);//移动窗口
    }
}

void Dialog::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(event->button() == Qt::LeftButton)
    {
        moveflag = false;
    }
}


void Dialog::on_pushButton_Exit_clicked()
{
    close();
}
//搜索歌曲
void Dialog::on_pushButton_SearchSong_clicked()
{
    ui->listWidget_searchList->clear();
    //获取需要搜索的歌曲名
    QString songName;

    songName = ui->lineEdit_InputSongs->text();

    //发送请求的地址
    QString cloudAddress = QString("http://music.163.com/api/search/get/web");

    // 定义一个QNetworkRequest请求对象
    QNetworkRequest networkRequest;

    //设置请求的 URL 地址
    networkRequest.setUrl(cloudAddress);
    // 循环搜索两页,每页8首歌
    for (int pageNum = 1; pageNum <= 5; ++pageNum)
    {
        // 设置偏移量
        int offset = (pageNum - 1) * 8;

        // 构建查询参数
        QUrlQuery query;
        query.addQueryItem("csrf_token", "");
        query.addQueryItem("hlpretag", "");
        query.addQueryItem("hlposttag", "");
        query.addQueryItem("s", songName);
        query.addQueryItem("type", "1");
        query.addQueryItem("offset", QString::number(offset));
        query.addQueryItem("total", "true");

        // 设置请求 URL
        QUrl url(cloudAddress);
        url.setQuery(query);

        // 发送 GET 请求
        networkRequest.setUrl(url);
        networkAccessManagers->get(networkRequest);

    }


}

//处理服务器返回的数据
void Dialog::handleDataBackFunc(QNetworkReply *pReply)
{
    //读取网络回馈数据
    m_QByteArySearchInfo  = pReply->readAll();

    QJsonParseError jsonPError; // 定义错误信息对象

    //qDebug()<<"song json:"<<m_QByteArySearchInfo;

    // 将json文本转换为json文件对象
    QJsonDocument jsonDoc_recvData = QJsonDocument::fromJson(m_QByteArySearchInfo,&jsonPError);

    if(jsonPError.error != QJsonParseError::NoError)
    {
        QMessageBox::critical(this,"Prompt","提示：搜索歌曲获得json格式错误，请重新检查？",QMessageBox::Yes);
        return ;
    }

    //qDebug()<<"song json after:"<<jsonDoc_recvData;

    // QJsonObject使用函数object()
    QJsonObject totalJsonObject = jsonDoc_recvData.object();

    //qDebug()<<"object:"<<totalJsonObject;

    // 列出json里面所有的key
    QStringList jsonKeys = totalJsonObject.keys();
    //qDebug()<<"keys:"<<jsonKeys;

    //如果返回的json里有result标签，说明搜索到了歌曲
    if(jsonKeys.contains("result"))
    {
        //将带有result的数据内容提取后转换为对象
        QJsonObject resultObject = totalJsonObject["result"].toObject();
        //qDebug()<<"resultObject:"<<resultObject;

        // 存储所有keys
        QStringList resultKeys = resultObject.keys();
        //qDebug()<<"resultKeys:"<<resultKeys;

        //含有songs标签说明能搜索到对应的歌曲
        if(resultKeys.contains("songs"))
        {
            QJsonArray songsArray = resultObject["songs"].toArray();//获取songs对应的列表
            //qDebug()<<"songs array:"<<resultObject["songs"];
            //qDebug()<<"songs jsonArray:"<<array;

            //循环查找歌曲中的字段信息
            for(auto isong:songsArray)
            {
                //qDebug()<<"isong:"<<isong;
                QJsonObject songsObject = isong.toObject();
                //qDebug()<<"songsObject:"<<songsObject;
                //获取歌曲ID 歌名 歌手
                musicId = songsObject["id"].toInt();
                musicName = songsObject["name"].toString();

                fee = songsObject["fee"].toInt();

                // 存储所有keys
                QStringList songsKeys = songsObject.keys();
                if(songsKeys.contains("artists"))
                {
                    QJsonArray artistsArrays = songsObject["artists"].toArray();
                    for (auto iartists : artistsArrays)
                    {
                        QJsonObject iartistsObject = iartists.toObject();
                        singerName = iartistsObject["name"].toString();
                    }
                }
                qDebug()<<"musicId:"<<musicId;
                qDebug()<<"musicName:"<<musicName;
                qDebug()<<"singerName:"<<singerName;


                //用于获取音乐文件的直接播放链接
                //QString requestURL;
                //requestURL = QString("https://music.163.com/song/media/outer/url?id=1331357732").arg(musicId);2854052

                //将获取的歌曲添加到播放列表
                //p_PlayerList->addMedia(QUrl(requestURL));
                //ui->plainTextEdit_SongList->append(musicName + "-" + singerName);
                // 在 listWidget_searchList 中添加一个项


                QListWidgetItem *item = new QListWidgetItem(musicName + "-" + singerName);
                item->setData(Qt::UserRole, musicId); // 将 musicId 存储为项的用户数据

                item->setSizeHint(QSize(50, 30));

                // 将VIP歌曲设置为不可点击
                if (fee == 1)
                {
                    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                }

                ui->listWidget_searchList->addItem(item);

            }

        }
    }


}


//播放/暂停音乐
void Dialog::on_pushButton_PlaySong_4_clicked()
{
    //播放列表为空，不做动作
    if(p_PlayerList->isEmpty())
    {
        return;
    }
    //如果是停止状态或暂停状态，则播放
    if(isPlay == false)
    {
        p_PlayerObject->play();
        QPixmap on(":/images/pause.png");
        ui->pushButton_PlaySong_4->setIcon(on);
        isPlay = !isPlay;
    }

    //如果是停止状态或暂停状态，则播放
    else
    {
        p_PlayerObject->pause();
        QIcon off(":/images/play.png");
        ui->pushButton_PlaySong_4->setIcon(off);
        isPlay = !isPlay;
    }

}


//窗口最小化
void Dialog::on_pushButton_Min_clicked()
{
    this->showMinimized(); // 调用 showMinimized 函数将窗口最小化
}

//上一曲
void Dialog::on_pushButton_PreviousSong_4_clicked()
{
    int m_index = p_PlayerList->currentIndex();
    int m_count = p_PlayerList->mediaCount();

    if(m_index > 0)
    {
        m_index -= 1;
        p_PlayerList->setCurrentIndex(m_index);
        p_PlayerObject->play();
    }
    else//第一曲点击时跳转到最后一曲
    {
        m_index = m_count - 1;
        p_PlayerList->setCurrentIndex(m_index);
        p_PlayerObject->play();
    }
}

//下一曲
void Dialog::on_pushButton_PlayNextSong_4_clicked()
{
    int m_index = p_PlayerList->currentIndex();
    int m_count = p_PlayerList->mediaCount();

    if(m_index < m_count)
    {
        m_index += 1;
        p_PlayerList->setCurrentIndex(m_index);
        p_PlayerObject->play();
    }
    else//第一曲点击时跳转到最后一曲
    {
        m_index = 0;
        p_PlayerList->setCurrentIndex(m_index);
        p_PlayerObject->play();
    }
}


//用户拖动进度条
void Dialog::on_horizontalSlider_PlayProgress_valueChanged(int value)
{
    //改变歌曲播放位置到用户拖动的位置
    if(ui->horizontalSlider_PlayProgress->isSliderDown())
    {
        p_PlayerObject->setPosition(value);
    }
}

void Dialog::on_horizontalSlider_Volume_valueChanged(int value)
{
    p_PlayerObject->setVolume(value);
}

void Dialog::on_pushButton_SoundYesNo_clicked()
{
    if (p_PlayerObject->isMuted())
    {
       // 如果当前处于静音状态，则恢复音量
       p_PlayerObject->setMuted(false);
       QPixmap on(":/images/sound-on.png");
       ui->pushButton_SoundYesNo->setIcon(on);
       //ui->pushButton_Mute->setText("Mute");
    }
    else
    {
       // 如果当前处于非静音状态，则设置静音
       p_PlayerObject->setMuted(true);
       QPixmap off(":/images/sound-off.png");
       ui->pushButton_SoundYesNo->setIcon(off);
       //ui->pushButton_Mute->setText("Unmute");
    }
}

void Dialog::on_pushButton_state_change_clicked()
{
    songState = (songState + 1) % 4;
    QPixmap m_state;
    switch (songState)
    {
        case LOOP:
            m_state = QPixmap(":/images/loop.png");
            ui->pushButton_state_change->setIcon(m_state);
            p_PlayerList->setPlaybackMode(QMediaPlaylist::Loop);
            break;
        case ONELOOP:
            m_state = QPixmap(":/images/oneLoop.png");
            ui->pushButton_state_change->setIcon(m_state);
            p_PlayerList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
            break;
        case SEQUENCE:
            m_state = QPixmap(":/images/sequence.png");
            ui->pushButton_state_change->setIcon(m_state);
            p_PlayerList->setPlaybackMode(QMediaPlaylist::Sequential);
            break;
        case RANDOM:
            m_state = QPixmap(":/images/random.png");
            ui->pushButton_state_change->setIcon(m_state);
            p_PlayerList->setPlaybackMode(QMediaPlaylist::Random);
            break;
    }
    qDebug()<<"songState:"<<songState;
    qDebug()<<"paly mode:"<<p_PlayerList->playbackMode();

}

void Dialog::on_pushButton_playlist_clicked()
{
    ui->stackedWidget->setCurrentIndex(0); // 设置当前页面为播放列表页面，索引为0
}

void Dialog::on_pushButton_lyrics_clicked()
{
    ui->stackedWidget->setCurrentIndex(1); // 设置当前页面为歌词页面，索引为1
}
//获取歌词
void Dialog::handleDataBackFunc_lyrics(QNetworkReply *pReply)
{
    // 读取网络回馈数据
    lyrics_QByteArySearchInfo = pReply->readAll();

    QJsonParseError JSonPError; // 定义错误信息对象

    // 将json文本转换为json文件对象
    QJsonDocument JSonDoc_RecvData=QJsonDocument::fromJson(lyrics_QByteArySearchInfo,&JSonPError);
    if(JSonPError.error != QJsonParseError::NoError) // 判断是否符合规则
    {
        QMessageBox::critical(this,"Prompt","提示：搜索歌曲获取json格式错误，请重新检查？",QMessageBox::Yes);
        return;
    }

    // QJsonObject使用函数object()
    QJsonObject TotalJsonOjbect=JSonDoc_RecvData.object();
    // 列出json里面所有的key
    QStringList strKeys=TotalJsonOjbect.keys();
    //qDebug()<<"keys:"<<strKeys;

    if(strKeys.contains("lrc"))
    {
        // 将带有result的数据内容提取之后转换为对象
        QJsonObject resultobject = TotalJsonOjbect["lrc"].toObject();

        // 存储所有keys
        QStringList strResultKeys = resultobject.keys();

        //qDebug()<<"strResultKeys:"<<strResultKeys;

        // 如果keys是lrc，证明搜索到对应的歌词
        if(strResultKeys.contains("lyric"))
        {
            lyrics = resultobject["lyric"].toString();
            //qDebug()<<"lyrics:"<<lyrics;
        }
    }

    ui->plainTextEdit_lyrics->setText(lyrics);
}
//点击关于按钮
void Dialog::on_pushButton_About_clicked()
{
    about = new About(this);
    about->exec();
}
