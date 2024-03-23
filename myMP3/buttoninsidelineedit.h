#ifndef BUTTONINSIDELINEEDIT_H
#define BUTTONINSIDELINEEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class ButtonInsideLineEdit : public QWidget
{
    Q_OBJECT

public:
    ButtonInsideLineEdit(QWidget *parent = nullptr) : QWidget(parent)
    {
        lineEdit = new QLineEdit(this);
        button = new QPushButton("X", this); // 按钮文本为X，可以根据需要自定义

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->addWidget(button);
        layout->addWidget(lineEdit);

        setLayout(layout);

        connect(button, &QPushButton::clicked, this, &ButtonInsideLineEdit::clearText);
    }

    QString text() const
    {
        return lineEdit->text();
    }

    void setText(const QString &text)
    {
        lineEdit->setText(text);
    }

signals:
    void textChanged(const QString &text);

private slots:
    void clearText()
    {
        lineEdit->clear();
    }

private:
    QLineEdit *lineEdit;
    QPushButton *button;
};

#endif // BUTTONINSIDELINEEDIT_H
