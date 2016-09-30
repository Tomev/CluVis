#ifndef CLUSTERINFO_RSESRULES_H
#define CLUSTERINFO_RSESRULES_H

#include <QDialog>
#include "Clustering/clusters.h"

namespace Ui {
class clusterInfo_RSESRules;
}

class clusterInfo_RSESRules : public QDialog
{
    Q_OBJECT

public:
    explicit clusterInfo_RSESRules(QWidget *parent = 0);
    clusterInfo_RSESRules(ruleCluster* c);
    ~clusterInfo_RSESRules();

signals:
    void passClusterToVisualize(ruleCluster*);

private slots:
    void on_pushButtonVisualize_clicked();

private:
    Ui::clusterInfo_RSESRules *ui;

    ruleCluster c;
};

#endif // CLUSTERINFO_RSESRULES_H
