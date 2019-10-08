#include <QApplication>
#include <QQmlApplicationEngine>

#include "applicationcontroller.h"

//#include <QtPlugin>
//Q_IMPORT_PLUGIN(QtQuick2Plugin)


int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setOrganizationName(QStringLiteral("LNU"));
    QApplication::setApplicationName(QStringLiteral("RentgenReader"));

    QApplication app(argc, argv);

    int res;
    {
        qmlRegisterType<ApplicationController>("my_appController", 1, 0, "AppController");

        // ensure ApplicationController is destroyed when we return result
        ApplicationController c(&app);
        c.initQml();

        res = app.exec();
    }

    return res;
}
