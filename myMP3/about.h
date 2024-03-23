#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

    // 处理窗口重绘，实现背景图片
    void paintEvent(QPaintEvent *event);


private:
    Ui::About *ui;
};

#endif // ABOUT_H
