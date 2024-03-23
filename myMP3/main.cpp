#include "dialog.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/icon.png"));
    Dialog w;
    w.show();
    return a.exec();
}
