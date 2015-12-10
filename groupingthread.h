#ifndef GROUPINGTHREAD_H
#define GROUPINGTHREAD_H

#include <QtCore>
#include <QProgressDialog>


#include "groupingsettingsincludes.h"
#include "generalincludes.h"

class groupingThread : public QThread
{

    Q_OBJECT

public:
    groupingThread();
    groupingThread(groupingSettings_RSESRules *RSESSettings,
                   groupingSettings_General *groupingSettings,
                   generalSettings *settings);
    ~groupingThread();

    void run();

signals:
    void passRules(ruleCluster*);
    void passJoinedRules(ruleCluster*);
    void passClusteredRules(ruleCluster**);
    void passNewClusters(cluster**);
    void passMDI(qreal MDI);
    void passMDBI(qreal MDBI);

    void passLogMsg(QString);

private:
    //Members
        //Constans
        //Variables
    bool wasGroupingCanceled;
    bool wasAverageRuleDenumeratorSet;

    qreal MDI;
    qreal MDBI;

    cluster** newClusters;
    int nextClusterID;

    generalSettings* settings;
    groupingSettings_General* groupingSettings;
    groupingSettings_RSESRules* RSESSettings = NULL;

    RSESAttribute* attributes;

    QProgressDialog* groupingProgress;
    QProgressDialog* creatingSimMatrixProgress;

    //Methods

    void fillAttributesData();

    void groupObjects();
        void groupRSESRules();
            void clusterRules();
            qreal **createSimMatrix(int simMatrixSize);
                qreal countRSESClustersSimilarityValue(ruleCluster *c1, ruleCluster *c2);
                    qreal countRSESClusterRuleSimilarityValue(QString r, ruleCluster *c);
                        qreal countRSESRulesSimilarityValue(QString r1, QString r2);
                            qreal countRSESRulesGowersMeasureValue(QString r1, QString r2);
                            qreal countRSESRulesSimpleSimilarityValue(QString r1, QString r2);
                            qreal countRSESRulesWeightedSimilarityValue(QString r1, QString r2);
                            QStringList getRuleGroupedPart(QString r);
                                QStringList prepareAttribute(QString a);
                                    QString removeBraces(QString a);
            qreal findHighestSimilarity(qreal **simMatrix, int simMatrixSize);
            cluster* joinClusters(cluster* c1, cluster* c2);
            void joinMostSimilarClusters(qreal **simMatrix, int simMatrixSize, qreal highestSim);
                QString getLongerRule(QString r1, QString r2);
                    int getRuleAttributesNumber(QString r);
                QString getShorterRule(QString r1, QString r2);
                QString createAverageRule(ruleCluster* c);
            void stopGrouping();

     void countMDI();
        qreal countLowestRSESInterclusterSimilarity(ruleCluster* c1, ruleCluster* c2);
            qreal countLowestRSESClusterRuleSimilarityValue(QString r, ruleCluster *c);

            qreal countClusterDispersion(ruleCluster *c, QString aRule, bool recursion);
     void countMDBI();
        qreal countSimilaritySum();

};

#endif // GROUPINGTHREAD_H
