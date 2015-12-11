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
    void on_actionLoadObjectBase_triggered();
    void on_actionSaveVisualization_triggered();
    void on_actionGenerateReport_triggered();
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
    void getClusters(cluster** c);
    void gotMDIData(qreal MDI, qreal maxMDI, int maxMDIClustersNumber);
    void gotMDBIData(qreal MDBI, qreal maxMDBI, int maxMDBIClustersNumber);

    void getGraphicsRectObject(customGraphicsRectObject *object);
    void getGraphicsEllipseObject(customGraphicEllipseObject *object);
    void gotMainEllipseRect(QRect* r);

    void gotRuleClusterToVisualize(ruleCluster* c);
    void showRuleInfo(ruleCluster* c);

    void gotLogText(QString);

private:

    enum Constants
    {
        REPORT_TYPE_XML = 0,
        REPORT_TYPE_TXT = 1
    };

    Ui::MainWindow *ui;

    QString raportMessage;

    bool areObjectsClustered;

    QGraphicsScene* scene;

    groupingThread* gThread;
    visualizationThread* vThread;

    generalSettings* settings;
    groupingSettings_General* gSettings;
    groupingSettings_RSESRules* gSettings_RSES;
    visualizationSettings_general* vSettings;
    visualizationSettings_RSESRules* vSettings_RSES;

    cluster** clusters;

    clusterInfo_RSESRules* cInfo;

    qreal MDI;
    qreal maxMDI;
    int maxMDIClustersNumber;
    qreal MDBI;
    qreal maxMDBI;
    int maxMDBIClustersNumber;
    QTime* tim;

    int getObjectsNumber();

    //File
    QFileInfo selectObjectBase();
    bool isRuleSet(QFileInfo base);

    void generateReport();
        QString getFilePath();
        QString getReportDirPath();
        int getFileType(QChar t);
        QString createReportContent(int type);
            QString createTXTReportContent();
                QString formatThickString(QString s);
            QString createXMLReportContent();
                QString createXMLFileHeader();
                QString createXMLFileWorkbook();
                QString createXMLFileDocumentProperties();
                QString createXMLFileExcelWorkbook();
                QString createXMLFileStyles();
                QString createXMLFileFooter();
                QString createXMLFileTable();
                QString createXMLFileTableHeader();
                QString createXMLFileTableCell(QString data, bool isHeader);
                QString createXMLFileTableContent();
                QString createXMLFileClustersData();
                QString createXMLFileClustersDataHeader();
                QString createXMLFileClustersDataContent();
        void createPath(QString path);
        int countCoverageSum();
        int countRuleLength(QString rule);
        int countAllNodes();
        int countUngroupedObjects();
        ruleCluster* findBiggestCluster();
        ruleCluster* findSmallestCluster();

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
