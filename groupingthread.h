#ifndef GROUPINGTHREAD_H
#define GROUPINGTHREAD_H

#include <QtCore>
#include <QProgressDialog>
#include <boost/shared_ptr.hpp>

#include "groupingsettingsincludes.h"
#include "generalincludes.h"

#include "attributedata.h"
#include "groupingsettings.h"

#include "groupingdatapreparator_rsesrules.h"

#include "enum_datatype.h"

typedef boost::shared_ptr<qreal> qreal_ptr;
typedef std::vector<qreal_ptr> clusterSimilarityData;
typedef boost::shared_ptr<clusterSimilarityData> simData;



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

    qreal MDI;
    qreal maxMDI;
    int maxMDIClustersNumber;
    qreal MDBI;
    qreal maxMDBI;
    int maxMDBIClustersNumber;

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

    void groupObjects();
        void groupRSESRules();
            void clusterRules();
                void fillAttributesValues(QString data, cluster* c);
                void fillDecisionAttributesValues(QString decision, ruleCluster* c);
            void fillSimMatrix(std::vector<simData> *simMatrix, int simMatrixSize);
            void updateSimMatrix(std::vector<simData> *simMatrix);

                qreal getClustersSimilarityValue(cluster* c1, cluster* c2);
                    qreal getClustersAverageLinkValue(cluster* c1, cluster* c2);
                    qreal getObjectsSimValue(cluster* c1, cluster* c2);
                        qreal getObjectsGowersSimValue(cluster* c1, cluster* c2);
                        qreal getObjectsSMCValue(cluster* c1, cluster* c2);
                        qreal getObjectsWSMCValue(cluster* c1, cluster* c2);

            void joinMostSimilarClusters(std::vector<simData> *simMatrix);
                void findHighestSimilarityIndexes(int* i, int* j, std::vector<simData> *simMatrix);
                cluster* joinClusters(cluster* c1, cluster* c2);
                void deleteClusterSimilarityData(unsigned int clusterId, std::vector<simData> *simMatrix);
                QString getLongerRule(QString r1, QString r2);
                    int getRuleAttributesNumber(QString r);
                QString getShorterRule(QString r1, QString r2);
            void stopGrouping();

     void countMDI(int size);
        qreal countLowestRSESInterclusterSimilarity(ruleCluster* c1, ruleCluster* c2);
            qreal countLowestRSESClusterRuleSimilarityValue(QString r, ruleCluster *c);

            qreal countClustersCompactness(ruleCluster *c, QString aRule);
     void countMDBI(int size);
        qreal countSimilaritySum(int size);

};

#endif // GROUPINGTHREAD_H
