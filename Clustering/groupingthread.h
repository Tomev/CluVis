#ifndef GROUPINGTHREAD_H
#define GROUPINGTHREAD_H

#include <QtCore>
#include <QProgressDialog>
#include <memory>

#include "groupingsettingsincludes.h"
#include "generalincludes.h"

#include "attributedata.h"
#include "groupingsettings.h"

#include "groupingdatapreparator_rsesrules.h"

#include "enum_datatype.h"

typedef std::shared_ptr<qreal> qreal_ptr;
typedef std::vector<qreal_ptr> clusterSimilarityData;
typedef std::shared_ptr<clusterSimilarityData> simData;

class groupingThread : public QThread
{
    Q_OBJECT

public:
    groupingThread(groupingSettings_Detailed *dGrpSettings,
                   groupingSettings_General *groupingSettings,
                   generalSettings *settings);

    void setSettings(groupingSettings_Detailed *dGrpSettings,
                     groupingSettings_General *groupingSettings,
                     generalSettings *settings);

    groupingThread(groupingSettings *settings);
    ~groupingThread();

    void run();

    groupingSettings_General* grpSettings;
    QHash<QString, attributeData*> attributes;
    generalSettings* settings;

    qreal getClustersSimilarityValue(cluster* c1, cluster* c2);

    void continueGrouping();

    std::vector<simData> simMatrix;

    QVector<cluster*> clusters;
    //cluster** clusters;

signals:

    void passLogMsg(QString);



private:


    //Members
        //Variables

    groupingDataPreparator* grpDataPrep;

    bool wasGroupingCanceled;
    bool wasAverageRuleDenumeratorSet;

    qreal MDI, minMDI, MDBI, maxMDBI;
    int minMDIClustersNumber, maxMDBIClustersNumber;


    int nextClusterID, newClusterIdx;

    groupingSettings_Detailed* dGrpSettings;

    QProgressDialog* groupingProgress;
    QProgressDialog* creatingSimMatrixProgress;

    //Methods

    void initializeGroupingProgressbar();
    void initializeCreatingMatrixProgressbar();
    void initializeDataPreparator();

    void groupObjects();     
        void fillSimMatrix(std::vector<simData> *simMatrix, int simMatrixSize);
        void joinMostSimilarClusters(std::vector<simData> *simMatrix);
            void findHighestSimilarityIndexes(int* i, int* j, std::vector<simData> *simMatrix);
            cluster* joinClusters(cluster* c1, cluster* c2);
            void deleteClusterSimilarityData(unsigned int clusterId, std::vector<simData> *simMatrix);
        void updateSimMatrix(std::vector<simData> *simMatrix);


            qreal getClustersAverageLinkValue(cluster* c1, cluster* c2);
            qreal getObjectsSimValue(cluster* c1, cluster* c2);
                qreal getObjectsGowersSimValue(cluster* c1, cluster* c2);
                    qreal getGowersSimilarityMeasureNumericAttributesSimilarity
                        (const QString attribute, QHash<QString, QStringList*> c1Attributes, QHash<QString, QStringList*> c2Attributes);
                        qreal getAttributesNaiveAverageValue(QStringList* values);
                qreal getObjectsSMCValue(cluster* c1, cluster* c2);
                qreal getObjectsWMCValue(cluster* c1, cluster* c2);
                qreal getObjectsOFSimValue(cluster* c1, cluster* c2);
                qreal getObjectsIOFSimValue(cluster* c1, cluster* c2);
                qreal getObjectsGoodall1SimValue(cluster* c1, cluster* c2);
                    qreal countSampleProbabilityOfAttributesValue(QString attribute, QString value);
                    qreal countSecondSampleProbabilityOfAttributesValue(QString attribute, QString value);
                qreal getObjectsGoodall2SimValue(cluster* c1, cluster* c2);
                qreal getObjectsGoodall3SimValue(cluster* c1, cluster* c2);
                qreal getObjectsGoodall4SimValue(cluster* c1, cluster* c2);

        qreal countClustersCompactness(cluster *c);

     void countMDI(int size);
        qreal getMaxInterClusterSimilarity(int clustersNum);
        qreal getMinIntraClusterSimilarity(int clustersNum);

     void countMDBI(int size);
};

#endif // GROUPINGTHREAD_H
