#include "about.h"
#include "ui_about.h"

#include <QPainter>

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

//    // 去掉窗口标题栏
//    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowTitle("关于");
    ui->textEdit->setReadOnly(true);
    // 获取当前窗口的窗口标志
    Qt::WindowFlags flags = this->windowFlags();

    // 去掉帮助按钮
    flags &= ~Qt::WindowContextHelpButtonHint;

    // 设置窗口标志
    this->setWindowFlags(flags);
    flags &= ~Qt::WindowContextHelpButtonHint;

    // 禁止窗口改变尺寸大小
    this->setFixedSize(this->width(),this->height());
}

About::~About()
{
    delete ui;
}

void About::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter qPainter(this);

    // 调用drawPixmap()函数进行绘图
    qPainter.drawPixmap(0,0,width(),height(),QPixmap(":/images/111.png"));

}
