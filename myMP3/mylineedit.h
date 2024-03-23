#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
#include <QPushButton>

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = nullptr);

public:
    MyLineEdit(QWidget *parent = nullptr) : QLineEdit(parent)
    {
        clearButton = new QPushButton(this);
        clearButton->setIcon(QIcon(":/images/clear.png")); // 设置清空按钮图标
        clearButton->setCursor(Qt::PointingHandCursor); // 设置鼠标样式
        clearButton->setFlat(true); // 设置按钮为扁平风格
        clearButton->setVisible(false); // 初始状态下隐藏按钮
        connect(clearButton, &QPushButton::clicked, this, &MyLineEdit::clear);
    }

signals:
protected:
    void resizeEvent(QResizeEvent *event) override
    {
        const int clearButtonSize = 16;
        clearButton->setGeometry(this->width() - clearButtonSize, (this->height() - clearButtonSize) / 2, clearButtonSize, clearButtonSize);
    }

private slots:
    void clear()
    {
        this->clear(); // 清空文本框内容
    }

private:
    QPushButton *clearButton;
};

#endif // MYLINEEDIT_H

