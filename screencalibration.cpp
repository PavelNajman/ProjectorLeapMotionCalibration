#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

#include <QJsonDocument>
#include <QJsonObject>

#include "screencalibration.h"

ScreenCalibration::ScreenCalibration(QWidget *parent) :
    QWidget(parent), state(IDLE), pattern(NULL), collector(NULL), timer(NULL),
    markerRadius(25), patternSize(2)
{
    setWindowIcon(QIcon(":icons/app.ico"));
}

bool ScreenCalibration::open(const QUrl &url)
{
    connect(&serverSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&serverSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(&serverSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));
    connect(&serverSocket, SIGNAL(disconnected()), this, SLOT(onConnectionClose()));

    serverSocket.open(QUrl(url));

    while(serverSocket.state() == QAbstractSocket::ConnectingState)
        QCoreApplication::processEvents(QEventLoop::AllEvents);

    if(serverSocket.state() != QAbstractSocket::ConnectedState){
        return false;
    }
    return true;
}

ScreenCalibration::~ScreenCalibration()
{
    delete timer;
    delete pattern;
    delete collector;
}

void ScreenCalibration::calibrate()
{
    state = CALIBRATION2D;

    // create calibration pattern
    if(pattern)
        delete pattern;
    pattern = new CalibrationPattern;
    pattern->distributeMarkers(this->size(), patternSize * patternSize, markerRadius);
    pattern->activateFirst();

    // create collector
    if(collector)
        delete collector;
    collector = new PointCollector;
    collector->setGoal(patternSize * patternSize);

    // start timer (disconnect all previously connected signals)
    if(timer)
        delete timer;
    timer = new QTimer;
    timer->setInterval(1000.0f / 60.0f);

    // connect signals to slots
    connect(timer, SIGNAL(timeout()), collector, SLOT(processFrame()));
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    connect(collector, SIGNAL(collected()), pattern, SLOT(activateNext()));
    connect(collector, SIGNAL(finished()), this, SLOT(finishCalibration()));

    // start calibration
    timer->start();
}

void ScreenCalibration::calibrate3D()
{
    state = CALIBRATION3D;

    // create calibration pattern
    if(pattern)
        delete pattern;
    pattern = new Pattern3D(3);
    pattern->distributeMarkers(this->size(), patternSize * patternSize, markerRadius);
    pattern->activateFirst();

    // create collector
    if(collector)
        delete collector;
    collector = new PointCollector;
    collector->setGoal(3 * patternSize * patternSize);

    // start timer (disconnect all previously connected signals)
    if(timer)
        delete timer;
    timer = new QTimer;
    timer->setInterval(1000.0f / 60.0f);

    // connect signals to slots
    connect(timer, SIGNAL(timeout()), collector, SLOT(processFrame()));
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    connect(collector, SIGNAL(collected()), pattern, SLOT(activateNext()));
    connect(collector, SIGNAL(finished()), this, SLOT(finishCalibration()));

    // start calibration
    timer->start();
}

void ScreenCalibration::test()
{
    state = TESTING;

    // start timer (disconnect all previously connected signals)
    if(timer)
        delete timer;
    timer = new QTimer;
    timer->setInterval(1000.0f / 60.0f);

    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    timer->start();
}

void ScreenCalibration::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    if(state == CALIBRATION2D)
        drawPattern(&painter);
    if(state == CALIBRATION3D)
        drawPattern3D(&painter);
    if(state == TESTING)
        drawCursor(&painter);
}

void ScreenCalibration::drawPattern(QPainter * painter)
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    QVector<Marker> markers = pattern->getMarkers();
    foreach(Marker marker, markers){
        QVector4D mPos = marker.getPosition();

        if(marker.isActive()){
            painter->setPen(Qt::blue);
            brush.setColor(Qt::blue);
        }else{
            painter->setPen(Qt::gray);
            brush.setColor(Qt::gray);
        }
        painter->setBrush(brush);
        painter->drawEllipse(QPoint(mPos.x(), mPos.y()), marker.getRadius(), marker.getRadius());

        painter->setPen(Qt::black);
        if(marker.isActive()){
            painter->drawLine(mPos.x(), mPos.y() + marker.getRadius(), mPos.x(), mPos.y() - marker.getRadius());
            painter->drawLine(mPos.x() + marker.getRadius(), mPos.y(), mPos.x() - marker.getRadius(), mPos.y());
            painter->drawEllipse(QPoint(mPos.x(), mPos.y()), 5, 5);
        }

        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPoint(mPos.x(), mPos.y()), marker.getRadius(), marker.getRadius());
    }
}

void ScreenCalibration::drawPattern3D(QPainter *painter)
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    QVector<Marker> markers = pattern->getMarkers();
    foreach(Marker marker, markers){
        if(marker.isActive()){
            if(collector->getPoints().size() % 3 == 1){
                painter->setPen(Qt::green);
                brush.setColor(Qt::green);
            }else if(collector->getPoints().size() % 3 == 2){
                painter->setPen(Qt::red);
                brush.setColor(Qt::red);
            }else{
                painter->setPen(Qt::blue);
                brush.setColor(Qt::blue);
            }
        }else{
            painter->setPen(Qt::gray);
            brush.setColor(Qt::gray);
        }
        painter->setBrush(brush);
        QVector4D mPos = marker.getPosition();
        painter->drawEllipse(QPoint(mPos.x(), mPos.y()), marker.getRadius(), marker.getRadius());

        painter->setPen(Qt::black);
        if(marker.isActive()){
            painter->drawLine(mPos.x(), mPos.y() + marker.getRadius(), mPos.x(), mPos.y() - marker.getRadius());
            painter->drawLine(mPos.x() + marker.getRadius(), mPos.y(), mPos.x() - marker.getRadius(), mPos.y());
            painter->drawEllipse(QPoint(mPos.x(), mPos.y()), 5, 5);
        }

        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPoint(mPos.x(), mPos.y()), marker.getRadius(), marker.getRadius());
    }
}

void ScreenCalibration::drawCursor(QPainter *painter)
{
    if(calibrationData.T == NONE)
        return;

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    brush.setColor(Qt::green);
    painter->setBrush(brush);
    painter->setPen(Qt::green);

    painter->drawEllipse(QPoint(touchCursor.x(), touchCursor.y()), 25, 25);

    brush.setColor(Qt::red);
    painter->setBrush(brush);
    painter->setPen(Qt::red);

    painter->drawEllipse(QPoint(pointCursor.x(), pointCursor.y()), 25, 25);

    if(calibrationData.T == C3D){
        brush.setColor(Qt::blue);
        painter->setBrush(brush);
        painter->setPen(Qt::blue);

        painter->drawEllipse(QPoint(paintCursor.x(), paintCursor.y()), 25, 25);
    }

    Leap::Frame frame = controller.frame();
    if(!frame.isValid())
        return;

    Leap::HandList hands = frame.hands();
    if(hands.isEmpty())
        return;

    Leap::Hand hand = hands[0];
    if(!hand.isValid())
        return;

    Leap::Finger middleFinger = hand.fingers().fingerType(Leap::Finger::TYPE_MIDDLE)[0];
    if(middleFinger.isExtended()){
        Leap::Vector tipPosition = middleFinger.stabilizedTipPosition();
        Leap::Vector direction = middleFinger.direction();

        QVector4D o = QVector4D(tipPosition.x, tipPosition.y, tipPosition.z, 1.0);
        QVector4D d = QVector4D(direction.x, direction.y, direction.z, 0.0);

        serverSocket.sendTextMessage(createTouchRequest(o));
        serverSocket.sendTextMessage(createPointRequest(o, d));
        serverSocket.sendTextMessage(createPaintRequest(o));
    }
}

void ScreenCalibration::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()){
    case Qt::Key_Escape:
        this->hide();
        break;
    case Qt::Key_Delete:
        if(state == CALIBRATION3D)
            calibrate3D();
        else
            calibrate();
        break;
    case Qt::Key_1:
        calibrate();
        break;
    case Qt::Key_2:
        calibrate3D();
        break;
    case Qt::Key_3:
        test();
        break;
    case Qt::Key_Plus:
        if(state != CALIBRATION2D && state != CALIBRATION3D)
            break;
        if(event->modifiers() & Qt::ShiftModifier){
            if(patternSize * (markerRadius + 5) * 2 < (this->width() < this->height() ? this->width() : this->height()))
                markerRadius += 5;
        }else{
            if(patternSize < 5){
                patternSize++;
                if(patternSize * markerRadius * 2 > (this->width() < this->height() ? this->width() : this->height()))
                    markerRadius = ((this->width() < this->height() ? this->width() : this->height()) / (patternSize * 2) / 5) * 5;
            }
        }
        if(state == CALIBRATION2D)
            calibrate();
        if(state == CALIBRATION3D)
            calibrate3D();
        break;
    case Qt::Key_Minus:
        if(state != CALIBRATION2D && state != CALIBRATION3D)
           break;
        if(event->modifiers() & Qt::ShiftModifier)
            markerRadius = (markerRadius - 5) <= 0 ? markerRadius : markerRadius -5;
        else
            if(patternSize > 2)
                patternSize--;
        if(state == CALIBRATION2D)
           calibrate();
        if(state == CALIBRATION3D)
           calibrate3D();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void ScreenCalibration::startCalibration(int screenNum)
{
    QScreen * screen = QGuiApplication::screens()[screenNum];

    // move the calibration window to specified screen
    move(screen->geometry().topLeft());
    resize(screen->geometry().width(),screen->geometry().height());

    // show in fullscreen
    showFullScreen();

    markerRadius = this->width() > this->height() ? this->width() / 32 : this->height() / 32;

    // based on screen num and calibration state select if we start calibration or test calibration
    if(calibrationData.T == NONE)
        calibrate();
    else
        test();
}

void ScreenCalibration::finishCalibration()
{   
    timer->stop();

    serverSocket.sendTextMessage(createCalibRequest(state == CALIBRATION2D ? C2D: C3D, collector->getPoints(), pattern->getMarkerPositions()));

    delete pattern;
    pattern = NULL;
    delete collector;
    collector = NULL;

    test();
}

void ScreenCalibration::onConnected()
{
}

void ScreenCalibration::processTextMessage(const QString & message)
{
    QVector4D intersectionPoint;
    if(parseCalibResponse(message, calibrationData)){
    } else if(parseTouchResponse(message, intersectionPoint)){
        touchCursor = intersectionPoint.toVector2D();
    } else if(parsePointResponse(message, intersectionPoint)){
        pointCursor = intersectionPoint.toVector2D();
    } else if(parsePaintResponse(message, intersectionPoint)){
        paintCursor = intersectionPoint.toVector2D();
    }
}

void ScreenCalibration::processBinaryMessage(const QByteArray & /*msg*/)
{
}

void ScreenCalibration::onConnectionClose()
{
}
