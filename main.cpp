#include "mainmenu.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <iostream>
#include "decoder.h"

int main(int argc, char *argv[]) {

#ifdef Q_OS_WIN32
    qputenv("MY_ADB_PATH", "adb\\win\\adb.exe");      // 设置环境变量, 跨平台时都需要改动
    qputenv("MY_SERVER_PATH", "server\\scrcpy_server.jar");
    qputenv("MY_IMG_PATH", ":/images/");
#else
    qputenv("MY_ADB_PATH", "adb/mac/adb");      // 设置环境变量, 跨平台时都需要改动
    qputenv("MY_SERVER_PATH", "server/scrcpy_server.jar");
    qputenv("MY_IMG_PATH", ":/images/");
#endif

    Decoder::init();

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QtScrcpy_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainMenu *w = new MainMenu();
    w->setAttribute(Qt::WA_QuitOnClose, true);
    w->setWindowTitle("JoyStick");
    w->show();

    int ret = a.exec();
    Decoder::destroy();
    return ret;
}
