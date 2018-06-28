#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QMenu>
#include <QSystemTrayIcon>
#include <QSignalMapper>

#include <QScreen>
#include <QDesktopWidget>

#include "screencalibration.h"
#include "calibrationserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName(QString(argv[0]));
    QApplication::setApplicationVersion("0.1");

    // parse command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Leap Motion-screen calibration");

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port", "Port on which the server will listen.", "port", "8889");
    parser.addOption(portOption);

    parser.process(a);

    bool ok;
    int portNum = parser.value(portOption).toInt(&ok);
    if(!ok || portNum < 0 || portNum > 65535){
        std::cerr << "ERROR: Port value have to be between 0 and 65535." << std::endl;
        return EXIT_FAILURE;
    }

    QString port = parser.value(portOption);

    // start server
    CalibrationServer s;
    if(s.listen(QHostAddress::LocalHost, portNum)){
        std::cout << "INFO: Server is listening on port " << s.serverPort() << std::endl;
    } else{
        std::cerr << "ERROR: Server could not start listening on port " << portNum << std::endl;
        return EXIT_FAILURE;
    }

    // start client
    ScreenCalibration c;
    if(!c.open(QUrl(QString("ws://localhost:") + port))){
        std::cerr << "ERROR: Client could not connect to server on port " << portNum << std::endl;
        return EXIT_FAILURE;
    }

    // create tray menu
    QMenu trayMenu;
    QSignalMapper signalMapper;

    int numScreens = QApplication::desktop()->numScreens();
    for(int i = 0; i < numScreens; i++){
        QAction * screenAction = trayMenu.addAction(QGuiApplication::screens()[i]->name());
        QObject::connect(screenAction, SIGNAL(triggered()), &signalMapper, SLOT(map()));
        signalMapper.setMapping(screenAction, i);
    }

    QObject::connect(&signalMapper, SIGNAL(mapped(int)), &c, SLOT(startCalibration(int)));

    QAction * quitAction = trayMenu.addAction("Quit");
    QObject::connect(quitAction, SIGNAL(triggered()), &a, SLOT(quit()));


    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(QIcon(":/icons/app.ico"));
    trayIcon.setContextMenu(&trayMenu);
    trayIcon.show();

    return a.exec();
}
