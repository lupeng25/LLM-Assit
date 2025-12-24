#include <QApplication>
#include "Frm_AIAssit.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/QtWidgetsApp/ICONs/Chat.png"));
    Frm_AIAssit w;
    w.show();
    return app.exec();
}
