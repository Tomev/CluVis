#include "visualizationthread.h"

#include <QPoint>

visualizationThread::visualizationThread(){}

visualizationThread::visualizationThread(generalSettings *settings,
                                         visualizationSettings_general *vSettings)
{
    this->settings = settings;
    this->vSettings = vSettings;
}

visualizationThread::~visualizationThread(){}

void visualizationThread::run(cluster *c)
{
    rectPadding = 10;
    visualize(c);
}

void visualizationThread::visualize(cluster *c)
{
    switch(settings->dataTypeID)
    {
        case RSESRulesId:

            emit passLogMsg(tr("log.rsesVisualizationStarted"));

            visualizeRSESRules(c);
            break;

        default:
            emit passLogMsg(tr("log.unknownObjectsType"));
            emit passLogMsg(tr("log.operationWontStart"));
    }
}

void visualizationThread::visualizeRSESRules(cluster *c)
{
    switch(vSettings->visualizationAlgorithmID)
    {
        case SliceAndDiceRTId:
            RSES_RTSAD(vSettings->sceneRect,c);
            break;
        case CircularTreemapId:
            RSES_CircularTreemap(vSettings->sceneRect,c);
            break;
        default:
            ;
    }
}

void visualizationThread::RSES_RTSAD(QRect* rect, cluster* c)
{
  QRect paintAreaRect = *rect;

  if(paintAreaRect.width() >= paintAreaRect.height())
      RSES_RTSAD_PaintVertical(paintAreaRect, c);
  else
      RSES_RTSAD_PaintHorizontal(paintAreaRect, c);
}

void visualizationThread::RSES_RTSAD_PaintVertical(QRect rect, cluster* c)
{
    QRect paintAreaRect = rect;

    if(c == NULL)
    {
        for(int i = 0; i < settings->stopCondition; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            float width = widthScaled(settings->clusters->size(), rect, settings->objectsNumber);

            if(width<1)
                width = 1;

            int height = paintAreaRect.height();

            if(height<1)
                height =1;
            QColor sV = getColorFromSize(settings->clusters->at(i)->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);

            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            customGraphicsRectObject* vRect =
                    new customGraphicsRectObject(newRect, sV, (ruleCluster*)(vSettings->clusters->at(i).get()));
            emit passGraphicsRectObject(vRect);

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 2*rectPadding &&
                    width > 2*rectPadding &&
                    vSettings->clusters->at(i)->size() > 1)
                RSES_RTSAD(&smallerRect, (ruleCluster*)(vSettings->clusters->at(i).get()));

            paintAreaRect.setLeft(paintAreaRect.left() + width);
        }
    }
    else
    {
        QVector<cluster*> clusters;

        int clustersNumber;

        if(c->hasBothNodes())
        {
            clusters.push_back((ruleCluster*)(c->leftNode.get()));
            clusters.push_back((ruleCluster*)(c->rightNode.get()));
            clustersNumber = 2;
        }
        else
        {
            clusters.push_back(c);
            clustersNumber = 1;
        }

        for(int i = 0; i < clustersNumber; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            float width = widthScaled(clusters[i]->size(), rect, c->size());
            int height = paintAreaRect.height();
            QColor sV = getColorFromSize(clusters[i]->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            customGraphicsRectObject* vRect = new customGraphicsRectObject(newRect, sV, (ruleCluster*)clusters[i]);

            emit passGraphicsRectObject(vRect);;

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 2*rectPadding &&
                    width > 2*rectPadding &&
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

void visualizationThread::RSES_RTSAD_PaintHorizontal(QRect rect, cluster *c)
{
    QRect paintAreaRect = rect;

    if(c == NULL)
    {
        for(int i = 0; i < settings->stopCondition; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            int width = paintAreaRect.width();
            float height = heightScaled(vSettings->clusters->at(i)->size(),
                                      rect, settings->objectsNumber);
            QColor sV = getColorFromSize(vSettings->clusters->at(i)->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            customGraphicsRectObject* vRect =
                    new customGraphicsRectObject(newRect,
                                                 sV,
                                                 (ruleCluster*)(vSettings->clusters->at(i).get()));

            emit passGraphicsRectObject(vRect);

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 2*rectPadding &&
                    width > 2*rectPadding &&
                    vSettings->clusters->at(i)->size() > 1)
                RSES_RTSAD(&smallerRect, (ruleCluster*)(vSettings->clusters->at(i).get()));

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
            clusters[0] = (ruleCluster*)c->leftNode.get();
            clusters[1] = (ruleCluster*)c->rightNode.get();
            clustersNumber = 2;
        }
        else
        {
          /*
            clusters = new ruleCluster*[1];
            clusters[0] = c;
            clustersNumber = 1;
          */
        }

        for(int i = 0; i < clustersNumber; i++)
        {
            int top = paintAreaRect.top();
            int left = paintAreaRect.left();
            int width = paintAreaRect.width();
            float height = heightScaled(clusters[i]->size(), rect, c->size());
            QColor sV = getColorFromSize(clusters[i]->size()); // sV -> shadeValue

            QRect newRect(left,top,width,height);
            QRect smallerRect(left+rectPadding, top+rectPadding,
                              width-2*rectPadding,height-2*rectPadding);

            customGraphicsRectObject* vRect =
                    new customGraphicsRectObject(newRect, sV, clusters[i]);

            emit passGraphicsRectObject(vRect);

            if(vSettings->visualizeAllHierarchyLevels &&
                    height > 2*rectPadding &&
                    width > 2*rectPadding &&
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

QColor visualizationThread::getColorFromSize(int cSize)
{
    if(cSize == 1) // Red
        return QColor(128,0,0);

    qreal sizePercent = 100 * cSize / settings->objectsNumber;
    int shade, minShadeValue = 120, maxShadeValue = 240;

    if(sizePercent < 10) // Orange
    {
        shade = maxShadeValue/2 * sizePercent/10;
        if(shade < minShadeValue) shade = minShadeValue;
        return QColor(2*shade,shade,0);
    }
    if(sizePercent >= 10 && sizePercent < 25) // Yellow
    {
        shade = maxShadeValue * (sizePercent - 10)/(25-10);
        if(shade < minShadeValue) shade = minShadeValue;
        return QColor(shade,shade,0);
    }
    if(sizePercent >= 25 && sizePercent < 50) // Green
    {
        shade = maxShadeValue * (sizePercent - 25)/(50-25);
        if(shade < minShadeValue) shade = minShadeValue;
        return QColor(0,shade,0);
    }
    if(sizePercent >= 50 && sizePercent < 75) // Blue
    {
        shade = maxShadeValue * (sizePercent - 50)/(75-50);
        if(shade < minShadeValue) shade = minShadeValue;
        return QColor(0,0,shade);
    }
    if(sizePercent >= 75) // Purple
    {
        shade = maxShadeValue * (sizePercent - 75)/(100 - 75);
        if(shade < minShadeValue) shade = minShadeValue;
        return QColor(shade,0,shade);
    }

    return QColor(0,0,0);
}

void visualizationThread::RSES_CircularTreemap(QRect *rect, cluster *c)
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
            newC = (ruleCluster*) vSettings->clusters->at(0).get();

            QColor sV = getColorFromSize(newC->size());

            customGraphicEllipseObject* cRect =
                    new customGraphicEllipseObject(mainBRect,sV, newC);

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

                RSES_CircularTreemap(c1BRect,(ruleCluster*)newC->leftNode.get());
                RSES_CircularTreemap(c2BRect,(ruleCluster*)newC->rightNode.get());
            }
        }

        if(settings->stopCondition >= 2)
        {
            QVector<cluster*> sortedClusters;
            fillVectorWithClustersSortedBySize(&sortedClusters);

            QList<qreal> diameters;
            QList<QRect*> circlesBRects;

            qreal top;
            qreal left;

            qreal circleICenterX;
            qreal circleJCenterX;
            qreal circleICenterY;
            qreal circleJCenterY;

            qreal directionCoefficient;

            for(int i = 0; i < sortedClusters.size(); i++)
                diameters.append(mainBRect.width() * sortedClusters.at(i)->size() / settings->objectsNumber);

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
                QColor sV = getColorFromSize(sortedClusters.at(i)->size());

                customGraphicEllipseObject* cRect =
                    new customGraphicEllipseObject(*circlesBRects[i],sV, (ruleCluster*)sortedClusters.at(i));

                emit passGraphicsEllipseObject(cRect);

                if(sortedClusters.at(i)->size() > 1 &&
                   vSettings->visualizeAllHierarchyLevels)
                {
                    qreal d1 = circlesBRects[i]->width() * sortedClusters.at(i)->leftNode->size() / sortedClusters.at(i)->size();
                    qreal d2 = circlesBRects[i]->width() * sortedClusters.at(i)->rightNode->size() / sortedClusters.at(i)->size();

                    qreal top = circlesBRects[i]->center().y() - d1/2;
                    qreal left = circlesBRects[i]->left();

                    QRect* c1BRect =
                        new QRect(left,top,d1,d1);

                    top = circlesBRects[i]->center().y() - d2/2;
                    left = circlesBRects[i]->left() + circlesBRects[i]->width() - d2;

                    QRect* c2BRect =
                        new QRect(left,top,d2,d2);

                    RSES_CircularTreemap(c1BRect,(ruleCluster*)sortedClusters.at(i)->leftNode.get());
                    RSES_CircularTreemap(c2BRect,(ruleCluster*)sortedClusters.at(i)->rightNode.get());
                }
            }

        }

    }
    else
    {
        newC = (ruleCluster*)c;

        QColor sV = getColorFromSize(newC->size()); // sV -> shadeValue

        customGraphicEllipseObject* cRect =
            new customGraphicEllipseObject(mainBRect,sV, newC);

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
                RSES_CircularTreemap(c1BRect,(ruleCluster*)newC->leftNode.get());
                RSES_CircularTreemap(c2BRect,(ruleCluster*)newC->rightNode.get());
            }
            else
            {
                sV = getColorFromSize(newC->leftNode->size());

                customGraphicEllipseObject* object =
                   new customGraphicEllipseObject(*c1BRect,sV,(ruleCluster*)newC->leftNode.get());

                emit passGraphicsEllipseObject(object);

                sV = getColorFromSize(newC->rightNode->size());

                object = new customGraphicEllipseObject(*c2BRect,sV,(ruleCluster*)newC->rightNode.get());

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

void visualizationThread::fillVectorWithClustersSortedBySize(QVector<cluster *> *sortedClusters)
{
    // Bubble sort is implemented.
    // TODO TR: Implement more sublime sorting algorithm.

    // Fill temporary vector with unsorted clusters
    QVector<cluster*> unsortedClusters;

    foreach(std::shared_ptr<cluster> clusterPointer, *(vSettings->clusters))
        unsortedClusters.append(clusterPointer.get());

    int largestClusterIndex;

    // While there still are unsorted clusters
    while(unsortedClusters.size() > 0)
    {
        // Find largest unsorted cluster
        largestClusterIndex = findLargestClusterIndex(&unsortedClusters);

        if(largestClusterIndex < 0)
        {
            qDebug() << "An error occured during searching for largest cluster index during sorting.";
            return;
        }

        // Pop its value into sorted clusters vector
        sortedClusters->append(unsortedClusters.at(largestClusterIndex));
        unsortedClusters.removeAt(largestClusterIndex);
    }

}

int visualizationThread::findLargestClusterIndex(QVector<cluster *> *unsortedClusters)
{
    // Check for nullpointer
    if(unsortedClusters == NULL)
    {
        qDebug() << "Unsorted clusters pointer is NULL.";
        return -1;
    }

    // Check if given vector is empty
    if(!unsortedClusters->isEmpty())
    {
        // If not find largest cluster
        int largestClustersIndex = 0, largestClustersSize = unsortedClusters->at(0)->size();

        for(int clustersIndex = 1; clustersIndex < unsortedClusters->size(); ++clustersIndex)
        {
            if(unsortedClusters->at(clustersIndex)->size() > largestClustersSize)
            {
                largestClustersIndex = clustersIndex;
                largestClustersSize = unsortedClusters->at(clustersIndex)->size();
            }
        }

        return largestClustersIndex;
    }

    qDebug() << "Unsorted clusters vector was empty.";
    return -1;
}

qreal visualizationThread::pointsEuklideanDistance(QPoint p1, QPoint p2)
{
    qreal x = (double) p1.x() - p2.x();
    qreal y = (double) p1.y() - p2.y();

    return (double) qSqrt((qPow(x,2) + qPow(y,2)));
}
