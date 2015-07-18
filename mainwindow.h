#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QTimer>

#include "generalsettings.h"

#include "groupingthread.h"
#include "visualizationthread.h"

#include "clusterinfoincludes.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    //File
    void on_actionLoadObjectBase_triggered();\
    void on_actionSaveVisualization_triggered();
    void on_actionExit_triggered();

    //Help
    void on_actionAbout_triggered();

    //Settings
    //void on_pushButtonDetails_clicked();
    //void on_pushButtonGeneral_clicked();

    //Main Buttons
    void on_pushButtonGroup_clicked();
    void on_pushButtonVisualize_clicked();

    //Rest

    void getRules(ruleCluster* c);
    void getJoinedRules(ruleCluster* c);
    void getClusteredRules(ruleCluster** c);
    void gotMDI(qreal MDI);

    void getGraphicsRectObject(customGraphicsRectObject *object);
    void getGraphicsEllipseObject(customGraphicEllipseObject *object);
    void gotMainEllipseRect(QRect* r);

    void gotRuleClusterToVisualize(ruleCluster* c);
    void showRuleInfo(ruleCluster* c);

    void gotLogText(QString);

private:
    Ui::MainWindow *ui;

    bool areObjectsClustered = false;

    QGraphicsScene* scene;

    groupingThread* gThread;
    visualizationThread* vThread;

    generalSettings* settings = new generalSettings();
    groupingSettings_General* gSettings = new groupingSettings_General();
    groupingSettings_RSESRules* gSettings_RSES = new groupingSettings_RSESRules();
    visualizationSettings_general* vSettings = new visualizationSettings_general();
    visualizationSettings_RSESRules* vSettings_RSES = new visualizationSettings_RSESRules();

    ruleCluster* rules;
    ruleCluster* joinedRules;
    ruleCluster** clusteredRules;

    clusterInfo_RSESRules* cInfo;

    qreal MDI = 0;

    QTime* tim = new QTime();

    int getObjectsNumber();

    //File
    QFileInfo selectObjectBase();
    bool isRuleSet(QFileInfo base);

    //Rest
    //Functions

    void setGroupingSettings();
    bool areSettingsCorrect();
        bool isBaseLoaded();
        bool isStopConditionCorrect();
    void setVisualizationSettings();


    void groupObjects();
    void visualize();

    void resizeEvent(QResizeEvent *);
};

#endif // MAINWINDOW_H
