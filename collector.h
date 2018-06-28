#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <Leap.h>
#include <QObject>
#include <QVector>
#include <QVector4D>
#include <QVector3D>



class PointCollector : public QObject
{
    Q_OBJECT

public:
    explicit PointCollector(QObject *parent = 0);
    void setGoal(int goal);
    QVector<QVector4D> getPoints();

signals:
    void finished();
    void collected();

public slots:
    void restart();
    void processFrame();

private:
    enum CollectorState{
        IDLE,
        FAST_MOVEMENT_EXPECTED,
        SLOW_MOVEMENT_EXPECTED,
        POINT_COLLECTING
    };

    int goal;
    CollectorState state;
    Leap::Controller controller;
    QVector<QVector3D> fingerPositions;
    QVector<QVector4D> points;
};

#endif // COLLECTOR_H
