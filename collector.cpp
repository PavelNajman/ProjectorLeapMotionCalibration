#include "collector.h"

#define FAST_MOVEMENT_SPEED 35.0f
#define SLOW_MOVEMENT_SPEED 1.5f
#define MAX_POSITIONS 15

PointCollector::PointCollector(QObject *parent)
    :QObject(parent), goal(0), state(FAST_MOVEMENT_EXPECTED)
{
}

void PointCollector::setGoal(int goal)
{
    this->goal = goal;
}

QVector<QVector4D> PointCollector::getPoints()
{
    return points;
}

void PointCollector::restart()
{
    points.clear();
    state = FAST_MOVEMENT_EXPECTED;
}

void PointCollector::processFrame()
{    
    if(points.size() == goal){
        emit finished();
        return;
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
        Leap::Vector tipPosition = middleFinger.tipPosition();
        switch(state){
        case IDLE:
            break;
        case FAST_MOVEMENT_EXPECTED:
            if(middleFinger.tipVelocity().magnitude() > FAST_MOVEMENT_SPEED)
                state = SLOW_MOVEMENT_EXPECTED;
            break;
        case SLOW_MOVEMENT_EXPECTED:
            if(middleFinger.tipVelocity().magnitude() < SLOW_MOVEMENT_SPEED){
                state = POINT_COLLECTING;
            }
            break;
        case POINT_COLLECTING:
            if(fingerPositions.size() < MAX_POSITIONS){
                fingerPositions.append(QVector3D(tipPosition.x, tipPosition.y, tipPosition.z));
            }else{
                // get median position
                QVector<float> xs, ys, zs;
                foreach(QVector3D position, fingerPositions){
                    xs << position.x(); ys << position.y(); zs << position.z();
                }

                qSort(xs);qSort(ys);qSort(zs);
                QVector4D fingerPosition(xs[xs.size()/2], ys[ys.size()/2], zs[zs.size()/2], 1.0f);

                state = FAST_MOVEMENT_EXPECTED;
                points.append(fingerPosition);
                emit collected();

                fingerPositions.clear();
            }
            break;
        default:
            break;
        }
    }
}
