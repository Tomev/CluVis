#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QTimer>
#include <memory>

#include "../generalsettings.h"

#include "../Clustering/groupingthread.h"
#include "../Visualization/visualizationthread.h"

#include "../Clustering/clusterinfoincludes.h"

#include "../Interference/clusterInterferencer.h"
#include "../Interference/classicalInterferencer.h"

enum languagesId
{
    english = 0,
    polish = 1
};

enum reportTypesId
{
    XmlReportId = 0,
    TxtReportId = 1
};

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:

  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:  

    void on_actionLoadObjectBase_triggered();


    void on_actionSaveVisualization_triggered();
    void on_actionGenerateReport_triggered();
    void on_actionMergeReports_triggered();
    void on_actionExit_triggered();

    //Help
    void on_actionAbout_triggered();

    //Main Buttons
    void on_pushButtonGroup_clicked();
    void on_pushButtonVisualize_clicked();

    //Rest
    void gotMDIData(qreal MDI, qreal maxMDI, int maxMDIClustersNumber);
    void gotMDBIData(qreal MDBI, qreal maxMDBI, int maxMDBIClustersNumber);

    void getGraphicsRectObject(customGraphicsRectObject *object);
    void getGraphicsEllipseObject(customGraphicEllipseObject *object);
    void gotMainEllipseRect(QRect* r);

    void gotRuleClusterToVisualize(ruleCluster* c);
    void showRuleInfo(ruleCluster* c);

    void gotLogText(QString);

    void on_actionPolish_triggered();

    void on_actionEnglish_triggered();

    void on_checkBoxVisualizeAllHierarchyLevels_clicked();

    void on_comboBoxAlgorithmVisualization_currentIndexChanged(int index);

    void on_pushButtonVisualizeVisualizator_clicked();

    void on_checkBoxVisualizeAllHierarchyLevelsVisualizator_clicked();

    void on_comboBoxAlgorithmVisualizationVisualizator_currentIndexChanged(int index);

    void on_spinBoxInterObjectMarginVisualizator_valueChanged(int arg1);

    void on_spinBoxInterObjectMargin_valueChanged(int arg1);

    void on_pushButtonStandard_clicked();

    void on_pushButtonInterfere_clicked();

    void on_actionLoadFactsBase_triggered();

private:
    clusterInterferencer ruleInterferencer;
    classicalInterferencer classicInterferencer;

    QTranslator* translator;

    Ui::MainWindow *ui;

    QString raportMessage;

    bool areObjectsClustered;

    QGraphicsScene* scene;

    groupingThread* gThread = 0;
    visualizationThread* vThread;

    generalSettings* settings;
    groupingSettings_General* gSettings;
    groupingSettings_Detailed* dGrpSettings;

    visualizationSettings_general* vSettings;

    std::vector<std::shared_ptr<cluster>> clusters;

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
        QString getOpenPath();
    void loadObjectsBase(QFileInfo OB);
      bool canBaseBeLoaded(QFileInfo OB);
        bool wasFileSelected(QString OB);
        bool isRuleSet(QFileInfo base);

    void generateReport(QString path = "");
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
        void saveVisualization(QString path);
        void createPath(QString path);
        int countCoverageSum();
        int countRuleLength(QString rule);
        int countAllNodes();
        int countUngroupedObjects();
        cluster* findBiggestCluster();
        cluster* findSmallestCluster();
        int getBiggestRepresentativeLength();
        int getSmallestRepresentativeLength();
        qreal getAverageRepresentativeLength();
        void getReportsMainContentFromLocation(QString reportsDirPath, QString* content);
        void translate(int lang);

    //Rest
    //Functions

    void addLogMessage(QString msg);

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
