#include "visualizationthread.h"

#include <QPoint>

visualizationThread::visualizationThread()
{
    rectPadding = 5;
}

visualizationThread::visualizationThread(generalSettings *settings,
                                         visualizationSettings_general *vSettings,
                                         visualizationSettings_RSESRules *vSettings_RSES)
{
    this->settings = settings;
    this->vSettings = vSettings;
    this->vSettings_RSES = vSettings_RSES;
}

visualizationThread::~visualizationThread()
{

}

void visualizationThread::run(ruleCluster *c)
{
    visualize(c);
}

void visualizationThread::visualize(ruleCluster *c)
{
    QString logText;

    switch(settings->dataTypeID)
    {
    case 0://settings->RSES_RULES_ID:

        logText = "Rozpoczynam wizualizację dla RSES Rules...";
        emit passLogMsg(logText);

        visualizeRSESRules(c);
        break;
    default:
        logText = "Nie rozpoznano typu obiektów. Grupowanie nie rozpocznie się.";
        emit passLogMsg(logText);

        ;
    }
}

void visualizationThread::visualizeRSESRules(ruleCluster *c)
{
    switch(vSettings->visualizationAlgorithmID)
    {
    case visualizationSettings_general::RT_SLICE_AND_DICE_ID:
        RSES_RTSAD(vSettings->sceneRect,c);
        break;
    case visualizationSettings_general::CIRCULAR_TREEMAP_ID:
        RSES_CircularTreemap(vSettings->sceneRect,c);
        break;
    default:
        ;
    }
}

void visualizationThread::RSES_RTSAD(QRect* rect, ruleCluster* c)
{

    QRect paintAreaRect = *rect;

    if(paintAreaRect.width() >= paintAreaRect.height())
        RSES_RTSAD_PaintVertical(paintAreaRect, c);
    else
        RSES_RTSAD_PaintHorizontal(paintAreaRect, c);
}

void visualizationThread::RSES_RTSAD_PaintVertical(QRect rect, ruleCluster* c)
{
    QRect paintAreaRect = rect;

    if(c == NULL)
    {
        for(int i = 0; i < settings->stopCondition; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            float width = widthScaled(vSettings_RSES->clusteredRules[i]->size(),
                                    rect, settings->objectsNumber);
            if(width<1)
                width = 1;
            int height = paintAreaRect.height();
            if(height<1)
                height =1;
            int sV = countShadeValue(vSettings_RSES->clusteredRules[i]->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            QColor shadeOfGray(sV,sV,sV);

            customGraphicsRectObject* vRect =
                    new customGraphicsRectObject(newRect,
                                                 shadeOfGray,
                                                 vSettings_RSES->clusteredRules[i]);
            emit passGraphicsRectObject(vRect);

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 0 &&
                    width > 0 &&
                    vSettings_RSES->clusteredRules[i]->size() > 1)
                RSES_RTSAD(&smallerRect, vSettings_RSES->clusteredRules[i]);

            paintAreaRect.setLeft(paintAreaRect.left() + width);
        }
    }
    else
    {
        ruleCluster** clusters;
        int clustersNumber;

        if(c->hasBothNodes())
        {
            clusters = new ruleCluster*[2];
            clusters[0] = (ruleCluster*)c->leftNode;
            clusters[1] = (ruleCluster*)c->rightNode;
            clustersNumber = 2;
        }
        else
        {
            clusters = new ruleCluster*[1];
            clusters[0] = c;
            clustersNumber = 1;
        }

        for(int i = 0; i < clustersNumber; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            float width = widthScaled(clusters[i]->size(), rect, c->size());
            int height = paintAreaRect.height();
            int sV = countShadeValue(clusters[i]->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            QColor shadeOfGray(sV,sV,sV);

            customGraphicsRectObject* vRect = new customGraphicsRectObject(newRect, shadeOfGray, clusters[i]);

            emit passGraphicsRectObject(vRect);;

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 0 &&
                    width > 0 &&
                    clusters[i]->size() > 1)
                RSES_RTSAD(&smallerRect, clusters[i]);

            paintAreaRect.setLeft(paintAreaRect.left() + width);
        }
    }
}

float visualizationThread::widthScaled(int width, QRect rect, int rulesInCluster)
{
    float result;

    if(rulesInCluster != 0)
        result =  (width) * rect.width() / rulesInCluster;
    else
        result = 0;

    return result;
}

void visualizationThread::RSES_RTSAD_PaintHorizontal(QRect rect, ruleCluster* c)
{
    QRect paintAreaRect = rect;

    if(c == NULL)
    {
        for(int i = 0; i < settings->stopCondition; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            int width = paintAreaRect.width();
            float height = heightScaled(vSettings_RSES->clusteredRules[i]->size(),
                                      rect, settings->objectsNumber);
            int sV = countShadeValue(vSettings_RSES->clusteredRules[i]->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            QColor shadeOfGray(sV,sV,sV);

            customGraphicsRectObject* vRect =
                    new customGraphicsRectObject(newRect,
                                                 shadeOfGray,
                                                 vSettings_RSES->clusteredRules[i]);

            emit passGraphicsRectObject(vRect);

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 6 &&
                    width > 6 &&
                    vSettings_RSES->clusteredRules[i]->size() > 1)
                RSES_RTSAD(&smallerRect, vSettings_RSES->clusteredRules[i]);

            paintAreaRect.setLeft(paintAreaRect.top() + height);
        }
    }
    else
    {
        ruleCluster** clusters;
        int clustersNumber;

        if(c->hasBothNodes())
        {
            clusters = new ruleCluster*[2];
            clusters[0] = (ruleCluster*)c->leftNode;
            clusters[1] = (ruleCluster*)c->rightNode;
            clustersNumber = 2;
        }
        else
        {
            clusters = new ruleCluster*[1];
            clusters[0] = c;
            clustersNumber = 1;
        }

        for(int i = 0; i < clustersNumber; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            int width = paintAreaRect.width();
            float height = heightScaled(clusters[i]->size(), rect, c->size());
            int sV = countShadeValue(clusters[i]->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            QColor shadeOfGray(sV,sV,sV);

            customGraphicsRectObject* vRect =
                    new customGraphicsRectObject(newRect, shadeOfGray, clusters[i]);

            emit passGraphicsRectObject(vRect);

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 6 &&
                    width > 6 &&
                    clusters[i]->size() > 1)
                RSES_RTSAD(&smallerRect, clusters[i]);

            paintAreaRect.setTop(paintAreaRect.top() + height);
        }
    }
}

float visualizationThread::heightScaled(int height, QRect rect,int rulesInCluster)
{
    if(rulesInCluster != 0)
        return (height) * rect.height() / rulesInCluster;
    else
        return 0;
}

int visualizationThread::countShadeValue(int cSize)
{
    int sV = 255 * cSize / settings->objectsNumber;

    if(sV > 255)
        return 255;
    if(sV < 40)
        return 40;

    return sV;
}

void visualizationThread::RSES_CircularTreemap(QRect *rect, ruleCluster *c)
{
    QRect mainBRect;
    ruleCluster* newC;

    if(rect->width() != rect->height())
        mainBRect = getCircleBoundingRect(vSettings->sceneRect);
    else
        mainBRect = *rect;


    if(c == NULL)
    {
        if(settings->stopCondition == 1)
        {
            newC = vSettings_RSES->clusteredRules[0];

            int sV = countShadeValue(newC->size()); // sV -> shadeValue
            QColor shadeOfGray(sV,sV,sV);

            customGraphicEllipseObject* cRect =
                    new customGraphicEllipseObject(mainBRect,shadeOfGray, newC);

            emit passGraphicsEllipseObject(cRect);

            if(newC->size() > 1 &&
               vSettings->visualizeAllHierarchyLevels)
            {
                qreal d1 = mainBRect.width() * newC->leftNode->size() / newC->size();
                qreal d2 = mainBRect.width() * newC->rightNode->size() / newC->size();

                qreal top = vSettings->sceneRect->height()/2 - d1/2;
                qreal left = mainBRect.left();

                QRect* c1BRect =
                    new QRect(left,top,d1,d1);

                top = vSettings->sceneRect->height()/2 - d2/2;
                left = mainBRect.left() + mainBRect.width() - d2;

                QRect* c2BRect =
                    new QRect(left,top,d2,d2);

                RSES_CircularTreemap(c1BRect,(ruleCluster*)newC->leftNode);
                RSES_CircularTreemap(c2BRect,(ruleCluster*)newC->rightNode);
            }
        }

        if(settings->stopCondition >= 2)
        {
            QList<ruleCluster*> sortedRuleClusters = getSortedRuleClusters(vSettings_RSES->clusteredRules);
            QList<qreal> diameters;
            QList<QRect*> circlesBRects;

            qreal top;
            qreal left;

            qreal circleICenterX;
            qreal circleJCenterX;
            qreal circleICenterY;
            qreal circleJCenterY;

            qreal directionCoefficient;

            for(int i = 0; i < sortedRuleClusters.size(); i++)
                diameters.append(mainBRect.width() * sortedRuleClusters[i]->size() / settings->objectsNumber);

            emit passMainEllipseRect(&mainBRect);

            top = (mainBRect.width() - diameters[0])/2;
            left = mainBRect.left() + (mainBRect.width() - diameters[0] - diameters[1]) / 2;

            circlesBRects.append(new QRect(left+1,top+1,diameters[0],diameters[0]));

            top = (mainBRect.width() - diameters[1])/2;
            left = circlesBRects[0]->left() + diameters[0];

            circlesBRects.append(new QRect(left+1,top+1,diameters[1],diameters[1]));


            for(int k = 2; k < settings->stopCondition; k++)
            {
                bool canBePlaced = true;

                //Add other circles
                QRect* potentialCircleKBRect;

                for(int i = 0; i < circlesBRects.size(); i++)
                {
                    for(int j = i+1; j < circlesBRects.size(); j++)
                    {
                        canBePlaced = true;

                        qreal distFromCIToh;
                        qreal h;
                        QPoint* hTouchPoint;
                        QPoint kCenter;

                        circleICenterX = circlesBRects[i]->center().x();
                        circleJCenterX = circlesBRects[j]->center().x();
                        circleICenterY = circlesBRects[i]->center().y();
                        circleJCenterY = circlesBRects[j]->center().y();

                        distFromCIToh = qPow((diameters[i]+diameters[j])/2,2);
                        distFromCIToh += qPow((diameters[i]+diameters[k])/2,2);
                        distFromCIToh -= qPow((diameters[j]+diameters[k])/2,2);
                        distFromCIToh /= 2/2*diameters[i]+diameters[j];

                        directionCoefficient = (circleICenterY - circleJCenterY);
                        directionCoefficient /= (circleICenterX - circleJCenterX);

                        qreal x1 = qCeil(qSqrt(qPow(directionCoefficient,2) + 1) * distFromCIToh);
                        qreal y1 = qCeil(directionCoefficient * x1);

                        hTouchPoint = new QPoint(circleICenterX,circleICenterY);

                        if(circleICenterX < circleJCenterX)
                            hTouchPoint->setX(hTouchPoint->x() + x1);
                        else
                            hTouchPoint->setX(hTouchPoint->x() - x1);

                        if(circleICenterY < circleJCenterY)
                            hTouchPoint->setY(hTouchPoint->y() + y1);
                        else
                            hTouchPoint->setY(hTouchPoint->y() - y1);

                        h = (double) qCeil(qSqrt(qPow((diameters[i]+diameters[k])/2,2) - qPow(distFromCIToh,2)));

                        qreal x2;
                        qreal y2;

                        if(circleICenterY - circleJCenterY <= 1)
                        {
                            x2 = 0;
                            y2 = h;
                        }
                        else
                        {
                            x2 = (double) qCeil(qSqrt(qPow(-1/directionCoefficient,2)+1) * h);
                            y2 = (double) qCeil(-1/directionCoefficient * x2);
                        }

                        kCenter = *hTouchPoint;

                        if(circleICenterX < circleJCenterX)
                        {
                            if(circleICenterY < circleJCenterY)
                            {
                                kCenter.setX(kCenter.x() + x2);
                                kCenter.setY(kCenter.y() + y2);
                            }
                            else
                            {
                                kCenter.setX(kCenter.x() - x2);
                                kCenter.setY(kCenter.y() - y2);
                            }
                        }
                        else
                        {
                            if(circleICenterY < circleJCenterY)
                            {
                                kCenter.setX(kCenter.x() - x2);
                                kCenter.setY(kCenter.y() + y2);
                            }
                            else
                            {
                                kCenter.setX(kCenter.x() + x2);
                                kCenter.setY(kCenter.y() - y2);
                            }

                        }


                        potentialCircleKBRect = new QRect(kCenter.x()-diameters[k]/2+1,
                                                    kCenter.y()-diameters[k]/2+1,
                                                    diameters[k],diameters[k]);

                        for(int l = 0; l < circlesBRects.size(); l++)
                        {
                            QPoint lCircleCenter = circlesBRects[l]->center();
                            QPoint kCircleCenter = potentialCircleKBRect->center();

                            qreal distanceFromCenter = (double) pointsEuklideanDistance(mainBRect.center(), kCircleCenter);
                            qreal distance = (double) pointsEuklideanDistance(lCircleCenter, kCircleCenter);
                            qreal radiusSum = (double) (diameters[k] + diameters[l])/2;

                            if(distanceFromCenter > (mainBRect.width()-diameters[k])/2)
                            {
                                canBePlaced = false;
                                break;
                            }

                            if(qCeil(distance) < radiusSum )
                            {
                                canBePlaced = false;
                                break;
                            }

                        }

                        if(canBePlaced)
                        {
                            circlesBRects.append(potentialCircleKBRect);
                            break;
                        }

                        canBePlaced = true;

                        kCenter = *hTouchPoint;

                        if(circleICenterX < circleJCenterX)
                        {
                            if(circleICenterY < circleJCenterY)
                            {
                                kCenter.setX(kCenter.x() - x2);
                                kCenter.setY(kCenter.y() - y2);
                            }
                            else
                            {
                                kCenter.setX(kCenter.x() + x2);
                                kCenter.setY(kCenter.y() + y2);
                            }
                        }
                        else
                        {
                            if(circleICenterY < circleJCenterY)
                            {
                                kCenter.setX(kCenter.x() + x2);
                                kCenter.setY(kCenter.y() - y2);
                            }
                            else
                            {
                                kCenter.setX(kCenter.x() - x2);
                                kCenter.setY(kCenter.y() + y2);
                            }

                        }

                        potentialCircleKBRect = new QRect(kCenter.x()-diameters[k]/2+1,
                                                    kCenter.y()-diameters[k]/2+1,
                                                    diameters[k],diameters[k]);

                        for(int l = 0; l < circlesBRects.size(); l++)
                        {
                            QPoint lCircleCenter = circlesBRects[l]->center();
                            QPoint kCircleCenter = potentialCircleKBRect->center();

                            qreal distanceFromCenter = (double) pointsEuklideanDistance(vSettings->sceneRect->center(), kCircleCenter);
                            qreal distance = (double) pointsEuklideanDistance(lCircleCenter, kCircleCenter);
                            qreal radiusSum = (double) (diameters[k] + diameters[l])/2;

                            if(distanceFromCenter > (mainBRect.width()-diameters[k])/2)
                            {
                                canBePlaced = false;
                                break;
                            }

                            if(qCeil(distance) < radiusSum )
                            {
                                canBePlaced = false;
                                break;
                            }

                        }


                        if(canBePlaced)
                        {
                            circlesBRects.append(potentialCircleKBRect);
                            break;
                        }

                    }
                    if(canBePlaced)
                        break;
                }
            }

            for(int i = 0; i < circlesBRects.size(); i++)
            {
                int sV = countShadeValue(sortedRuleClusters[i]->size()); // sV -> shadeValue
                QColor shadeOfGray(sV,sV,sV);

                customGraphicEllipseObject* cRect =
                    new customGraphicEllipseObject(*circlesBRects[i],shadeOfGray, sortedRuleClusters[i]);

                emit passGraphicsEllipseObject(cRect);

                if(sortedRuleClusters[i]->size() > 1 &&
                   vSettings->visualizeAllHierarchyLevels)
                {
                    qreal d1 = circlesBRects[i]->width() * sortedRuleClusters[i]->leftNode->size() / sortedRuleClusters[i]->size();
                    qreal d2 = circlesBRects[i]->width() * sortedRuleClusters[i]->rightNode->size() / sortedRuleClusters[i]->size();

                    qreal top = circlesBRects[i]->center().y() - d1/2;
                    qreal left = circlesBRects[i]->left();

                    QRect* c1BRect =
                        new QRect(left,top,d1,d1);

                    //top = vSettings->sceneRect->height()/2 - d2/2;
                    top = circlesBRects[i]->center().y() - d2/2;
                    left = circlesBRects[i]->left() + circlesBRects[i]->width() - d2;

                    QRect* c2BRect =
                        new QRect(left,top,d2,d2);

                    RSES_CircularTreemap(c1BRect,(ruleCluster*)sortedRuleClusters[i]->leftNode);
                    RSES_CircularTreemap(c2BRect,(ruleCluster*)sortedRuleClusters[i]->rightNode);
                }
            }

        }

    }
    else
    {
        newC = c;

        int sV = countShadeValue(newC->size()); // sV -> shadeValue
        QColor shadeOfGray(sV,sV,sV);

        customGraphicEllipseObject* cRect =
            new customGraphicEllipseObject(mainBRect,shadeOfGray, newC);

        emit passGraphicsEllipseObject(cRect);

        if(newC->size() > 1){

            qreal d1 = mainBRect.width() * newC->leftNode->size() / newC->size();
            qreal d2 = mainBRect.width() * newC->rightNode->size() / newC->size();

            qreal top = mainBRect.center().y() - d1/2;
            qreal left = mainBRect.left();

            QRect* c1BRect =
                new QRect(left,top+1,d1,d1);
            top = mainBRect.center().y() - d2/2;
            left = mainBRect.left() + mainBRect.width() - d2;

            QRect* c2BRect =
                new QRect(left,top+1,d2,d2);

            if(vSettings->visualizeAllHierarchyLevels){
                RSES_CircularTreemap(c1BRect,(ruleCluster*)newC->leftNode);
                RSES_CircularTreemap(c2BRect,(ruleCluster*)newC->rightNode);
            }
            else
            {
                sV = countShadeValue(newC->leftNode->size());
                shadeOfGray = QColor(sV,sV,sV);

                customGraphicEllipseObject* object =
                   new customGraphicEllipseObject(*c1BRect,shadeOfGray,(ruleCluster*)newC->leftNode);

                emit passGraphicsEllipseObject(object);

                sV = countShadeValue(newC->rightNode->size());
                shadeOfGray = QColor(sV,sV,sV);

                object = new customGraphicEllipseObject(*c2BRect,shadeOfGray,(ruleCluster*)newC->rightNode);

                emit passGraphicsEllipseObject(object);
            }
        }
    }
}

QRect visualizationThread::getCircleBoundingRect(QRect *rect)
{
    QRect bRect;

    if(rect->height() > rect->width())
    {
        bRect.setTop(rect->height()/2 - rect->width()/2);
        bRect.setHeight(rect->width());
        bRect.setWidth(rect->width());
    }
    else
    {
        bRect.setLeft(rect->width()/2 - rect->height()/2);
        bRect.setHeight(rect->height());
        bRect.setWidth(rect->height());
    }

    return bRect;
}

QList<ruleCluster *> visualizationThread::getSortedRuleClusters(ruleCluster **c)
{
    QList<ruleCluster*> result;
    QList<ruleCluster*> clusters;

    int largestClusterIdx = 0;
    int largestClusterSize = 1;

    int i;

    for(i = 0; i < settings->stopCondition; i++)
    {
        clusters.append(c[i]);

        if(c[i]->size() > largestClusterSize)
        {
            largestClusterIdx = i;
            largestClusterSize = c[i]->size();
        }
    }

    result.append(clusters[largestClusterIdx]);
    clusters.removeAt(largestClusterIdx);

    while(clusters.size() > 0)
    {
        largestClusterIdx = 0;
        largestClusterSize = 1;

        for(i = 0; i < clusters.size(); i++)
        {
            if(clusters[i]->size() > largestClusterSize)
            {
                largestClusterIdx = i;
                largestClusterSize = c[i]->size();
            }
        }

        result.append(clusters[largestClusterIdx]);
        clusters.removeAt(largestClusterIdx);
    }

    return result;
}

qreal visualizationThread::pointsEuklideanDistance(QPoint p1, QPoint p2)
{
    qreal x = (double) p1.x() - p2.x();
    qreal y = (double) p1.y() - p2.y();

    return (double) qSqrt((qPow(x,2) + qPow(y,2)));
}




