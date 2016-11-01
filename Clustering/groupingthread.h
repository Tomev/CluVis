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
    groupingThread(groupingSettings *settings);
    ~groupingThread();

    void run();

signals:
    void passClusters(cluster**);
    void passMDIData(qreal MDI, qreal maxMDI, int maxMDIClustersNumber);
    void passMDBIData(qreal MDBI, qreal maxMDBI, int maxMDBIClustersNumber);

    void passLogMsg(QString);

private:
    //Members
        //Variables

    groupingDataPreparator* grpDataPrep;

    bool wasGroupingCanceled;
    bool wasAverageRuleDenumeratorSet;

    qreal MDI, minMDI, MDBI, maxMDBI;
    int minMDIClustersNumber, maxMDBIClustersNumber;

    cluster** clusters;
    int nextClusterID, newClusterIdx;

    generalSettings* settings;
    groupingSettings_General* grpSettings;
    groupingSettings_Detailed* dGrpSettings;

    QHash<QString, attributeData*> attributes;

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

        qreal getClustersSimilarityValue(cluster* c1, cluster* c2);
            qreal getClustersAverageLinkValue(cluster* c1, cluster* c2);
            qreal getObjectsSimValue(cluster* c1, cluster* c2);
                qreal getObjectsGowersSimValue(cluster* c1, cluster* c2);
                qreal getObjectsSMCValue(cluster* c1, cluster* c2);
                qreal getObjectsWSMCValue(cluster* c1, cluster* c2);
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
