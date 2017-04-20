#ifndef VISUALIZATIONTHREAD_H
#define VISUALIZATIONTHREAD_H

#include <QTCore>

#include "generalincludes.h"
#include "visualizationincludes.h"

#include "enum_datatype.h"

class visualizationThread : public QThread
{
    Q_OBJECT

public:
    visualizationThread();
    visualizationThread(generalSettings* settings,
                        visualizationSettings_general* vSettings);
    ~visualizationThread();

    void run(ruleCluster *c = NULL);

signals:
    void passGraphicsRectObject(customGraphicsRectObject *o);
    void passMainEllipseRect(QRect*);
    void passGraphicsEllipseObject(customGraphicEllipseObject *o);

    void passLogMsg(QString);

private:

    generalSettings* settings;
    visualizationSettings_general* vSettings;

    int rectPadding;

    void visualize(ruleCluster *c);
        void visualizeRSESRules(ruleCluster *c);
            void RSES_RTSAD(QRect *rect = new QRect(0,0,0,0), ruleCluster *c = NULL);
                void RSES_RTSAD_PaintVertical(QRect rect, ruleCluster* c);
                void RSES_RTSAD_PaintHorizontal(QRect rect, ruleCluster* c);


                float widthScaled(int clusterSize, QRect rect, int objectsNumber);
                float heightScaled(int clusterSize, QRect rect, int objectsNumber);

                QColor getColorFromSize(int clusterSize);

            void RSES_CircularTreemap(QRect *rect = new QRect(0,0,0,0), ruleCluster *c = NULL);
                QRect getCircleBoundingRect(QRect* rect);
                void fillVectorWithClustersSortedBySize(QVector<cluster*> *sortedClusters);
                    int findLargestClusterIndex(QVector<cluster*> *unsortedClusters);

   qreal pointsEuklideanDistance(QPoint p1, QPoint p2);

};

#endif // VISUALIZATIONTHREAD_H
