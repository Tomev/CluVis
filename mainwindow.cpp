#include "mainwindow.h"
#include "about.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <algorithm>
#include <QtCore>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QtGui>
#include <QDesktopWidget>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Automatically translate to english.
    translator = new QTranslator();
    QString transName = QApplication::applicationDirPath() + "/language/english.qm";

    if(!translator->load(transName))
        delete translator;
    else
        qApp->installTranslator(translator);

    settings = new generalSettings();
    gSettings = new groupingSettings_General();
    gSettings_RSES = new groupingSettings_RSESRules();
    vSettings = new visualizationSettings_general();
    vSettings_RSES = new visualizationSettings_RSESRules();
    tim = new QTime();

    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setSceneRect(0,0,0,0);

    ui->labelIsBaseGrouped->setText("");

    addLogMessage(tr("log.applicationStart"));
    // Rozpoczęto działanie programu CluVis.

    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->actionSaveVisualization->setEnabled(false);
    ui->actionGenerateReport->setEnabled(false);

    setWindowFlags(Qt::Window |
                   Qt::WindowTitleHint |
                   Qt::CustomizeWindowHint |
                   Qt::WindowMaximizeButtonHint |
                   Qt::WindowCloseButtonHint |
                   Qt::WindowMinimizeButtonHint
    );
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
    delete gThread;
    delete vThread;
    delete settings;
    delete gSettings;
    delete gSettings_RSES;
    delete vSettings;
    delete vSettings_RSES;

    for(int i = settings->objectsNumber; i >= 0; i--)
        delete clusters[i];

    delete[] clusters;
    delete cInfo;
    delete tim;
}

//GUI

//File

void MainWindow::on_actionLoadObjectBase_triggered()
{
    QFileInfo KB = selectObjectBase();
    QString OBName = KB.completeBaseName();

    if(OBName == "")
    {
        QString logText = tr("log.failedToLoadObjBase") + " ";
        // Nieudana próba wczytania bazy obiektów.
        logText+= tr("log.noFileSelected");
        // Nie wybrano pliku.

        addLogMessage(logText);

        return;
    }

    if(isRuleSet(KB) == false)
    {
        QString logText = tr("log.failedToLoadObjBase") + " ";
        // Nieudana próba wczytania bazy wiedzy.
        logText+= tr("log.selectedFileIsNotKnowledgeBase");
        // Wybrany plik nie jest bazą wiedzy.

        addLogMessage(logText);

        return;
    }

    scene->clear();
    ui->actionSaveVisualization->setEnabled(false);
    ui->actionGenerateReport->setEnabled(false);
    ui->labelIsBaseGrouped->setText(tr("bold.ungrouped")); // <b>(Niepogrupowana)</b>
    areObjectsClustered = false;

    gSettings->objectBaseInfo = KB;

    OBName += "."+gSettings->objectBaseInfo.suffix();
    settings->objectsNumber = getObjectsNumber();

    ui->labelObjectBaseName->setText("<b>"+OBName+"</b>");
    ui->labelObjectsNumberValue->setText("<b>"+QString::number(settings->objectsNumber)+".</b>");
    ui->spinBoxStopConditionValue->setMaximum(settings->objectsNumber);
    ui->spinBoxStopConditionValue->setValue(1);

    addLogMessage(tr("log.knowledgeBaseLoaded") + " " + OBName + ".");
    // Wczytano bazę wiedzy
}

QFileInfo MainWindow::selectObjectBase()
{
    QFileDialog FD;

    QString customOpenPath = "C:\\Users\\Tomek\\Desktop\\Rules";
    QString defaultOpenPath = "C:\\";
    QString openPath;

    if(QDir(customOpenPath).exists())
        openPath = customOpenPath;
    else
        openPath = defaultOpenPath;

    QString OBPath = FD.getOpenFileName(
        this,
        tr("FD.selectKnowledgeBase"),
        openPath,
        tr("FD.RSESRules.fileTypes")
    );
    // Wybierz bazę wiedzy
    // Pliki reguł(*.rul);; Pliki tekstowe (*.txt)

    return QFileInfo(OBPath);
}

bool MainWindow::isRuleSet(QFileInfo base)
{
    QString line;

    QFile KB(base.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        line = in.readLine();


        if(line.contains("RULE_SET"))
            return true;
    }

    return false;
}

int MainWindow::getObjectsNumber()
{
    switch(settings->dataTypeID)
    {
        case generalSettings::RSES_RULES_ID:
            return gSettings_RSES->getRSESRulesNumber(
                        gSettings->objectBaseInfo);
            break;
        default:
            ;
    }

    return -1;
}

void MainWindow::on_actionSaveVisualization_triggered()
{
    QFileDialog FD;
    QString savePath = "C:\\ANB\\";

    QString fileName = FD.getSaveFileName(
        this,
        tr("FD.save"),   // Zapisz
        savePath,
        tr("FD.png")      // *.png
     );

    if(fileName == "")
    {
        QString logText = tr("log.failedToSaveVisualization") + " ";
        // Nieudana próba zapisu obrazu.
        logText+= tr("log.fileNameNotSelected");
        // Nie wybrano nazwy pliku.

        addLogMessage(logText);

        return;
    }

    // Add .png to ensure proper format.
    // TODO: Add if to check if format is already added.

    fileName += ".png";

    QRect sceneRect(ui->graphicsView->frameGeometry().left() + 15,
                    ui->graphicsView->frameGeometry().top() + 55,
                    ui->graphicsView->frameGeometry().width() - 3,
                    ui->graphicsView->height()-3);


    QPixmap pixMap = QWidget::grab(sceneRect);
    pixMap.save(fileName);

    addLogMessage(
        QString(tr("log.visualizationSavedWithName"))
        .arg(fileName)
    );
    // Wizualizację zapisano pod nazwą %1.
}

void MainWindow::on_actionGenerateReport_triggered()
{
    generateReport();
}

void MainWindow::generateReport()
{
    QString filePath = getFilePath();

    if(filePath == "")
    {
        addLogMessage(tr("log.operationAborted"));
        // Przerwano operację.

        return ;
    }

    int fileType = getFileType(filePath.at(filePath.length()-1));

    QString reportContent = createReportContent(fileType);

    QFile report(filePath);

    addLogMessage(tr("log.reportGenerationStarted"));
    // Rozpoczęto generowanie raportu.

    if(!report.open(QFile::WriteOnly | QFile::Text) && report.exists())
    {
        QString reportInfo = tr("log.unableToOpenFileForSaving") + " ";
        // Nie można otworzyć pliku do zapisu.
        reportInfo += tr("log.operationAborted");
        addLogMessage(reportInfo);

        return ;
    }

    QTextStream outStream(&report);
    outStream.setCodec("UTF-8");
    outStream << reportContent;

    report.flush();
    report.close();

    addLogMessage(tr("log.reportGenerationFinished"));
    // Zakończono generowanie raportu.
}

QString MainWindow::getFilePath()
{
    QFileDialog FD;
    QString savePath = "C:\\ANB\\";

    QString filePath = FD.getSaveFileName(
        this,
        tr("FD.saveReport"),            //Zapisz raport
        savePath,
        tr("FD.reportFileTypes")        // Pliki xml (*.xml);; Pliki tekstowe (*.txt)
    );

    if(filePath == "")
    {
        QString logText = tr("log.failedToSaveReport") + " ";
        // Nieudana próba zapisu raportu.
        logText+= tr("log.fileNameNotSelected");

        addLogMessage(logText);

        return "";
    }

    return filePath;
}

QString MainWindow::getReportDirPath()
{
    QString path = "C:\\CluVis_Reports\\";

    return path;
}

void MainWindow::createPath(QString path)
{
    QDir().mkpath(path);
}

// File type is determined by it's name (*.xml/*.txt), so
// last letter of file's name is enough to detemine it's
// type. At least in this case.

int MainWindow::getFileType(QChar t)
{
    if(t=='l')
        return REPORT_TYPE_XML;
    if(t=='t')
        return REPORT_TYPE_TXT;

    return -1;
}

QString MainWindow::createReportContent(int type)
{
    QString content;

    switch(type)
    {
    case REPORT_TYPE_TXT:
        content = createTXTReportContent();
        break;
    case REPORT_TYPE_XML:
        content = createXMLReportContent();
        break;
    default:
        break;
    }

    return content;
}

// TODO: Reconsider txt report formatting.

QString MainWindow::createTXTReportContent()
{
    QString content = "===" + tr("report.report") + "===";
    //===Raport===
    content += "\n==" + tr("report.knowledgeBaseInformation") + "==\n";
    // Informacje o bazie wiedzy
    content += "\n" + tr("report.nameOfBase") + ": "
            + formatThickString(ui->labelObjectBaseName->text());
    // Nazwa bazy
    content += "\n" + tr("report.attributesNumber") + ": "
            + QString::number(gSettings->attributesNumber);
    // Liczba atrybutów
    content += "\n" + tr("report.objectsNumber") + ": "
            + QString::number(getObjectsNumber());
    // Liczba obiektów
    content += "\n" + tr("report.clustersNumber") + ": "
            + QString::number(settings->stopCondition);
    // Liczba skupień
    content += "\n" + tr("report.coverageSum") + ": "
            + QString::number(countCoverageSum());
    // Suma pokryciowa
    content += "\n\n==" + tr("report.settings") + "==\n";
    // Ustawienia
    content += "\n" + tr("report.groupingAlgorithmUsed") + ": "
            + formatThickString(ui->labelAlgorithmGroupingValue->text());
    // Wykorzystany algorytm grupowania
    content += "\n" + tr("report.objectsSimilarityMeasure") + ": "
            + ui->comboBoxInterobjectDistanceMeasure->currentText();
    // Wybrana miara podobieństwa obiektów
    content += "\n" + tr("report.clustersSimilarityMeasure") + ": "
            + ui->comboBoxInterclusterDistanceMeasure->currentText();
    // Wybrana miara podobieństwa skupień
    content += "\n" + tr("report.selectedVisualizationAlgorithm") + ": "
            + ui->comboBoxAlgorithmVisualization->currentText();
    // Wybrany algorytm wizualizacji
    content += "\n" + tr("report.groupedRulePart") + ": "
            + ui->comboBoxRuleGroupedPart->currentText();
    // Grupowana część reguły
    content += "\n\n==" + tr("report.clustersInformation") + "==\n";
    // Dane skupień
    content += "\n" + tr("report.biggestCluster") + ": "
            + findBiggestCluster()->name();
    // Najliczniejsza grupa
    content += "\n" + tr("report.smallestCluster") + ": "
            + findSmallestCluster()->name();
    // Najmniej liczna grupa
    content += "\n\n==" + tr("report.clustersDetails") + "==\n";
    // Szczegóły skupień

    for(int i = 0; i < settings->stopCondition; i++)
    {
        content += "\n=" + tr("report.clustersName") + ": "
                + clusters[i]->name() + "=";
        // Nazwa skupienia
        content += "\n\t" + tr("report.clustersSize") + ": "
                + QString::number(clusters[i]->size());
        // Liczba reguł w grupie
        content += "\n\t" + tr("report.clustersCoverage") + ": "
                + QString::number((((ruleCluster*)clusters[i])->support*100)/countCoverageSum()) + "%";
        // Pokrycie skupienia
        content += "\n\t" + tr("report.clustersRepresentative") + ": "
                + ((ruleCluster*)clusters[i])->representative;
        // Reprezentant skupienia
    }

    content += "\n\n===" + tr("report.end") + "===";
    // Koniec

    return content;
}

QString MainWindow::formatThickString(QString s)
{
    s.resize(s.length()-4);
    s.remove(0,3);

    return s;
}

ruleCluster* MainWindow::findSmallestCluster()
{
    ruleCluster* smallest = ((ruleCluster*)clusters[0]);

    for(int i = 1; i < settings->stopCondition; ++i)
    {
        if(smallest->size() > clusters[i]->size())
            smallest = ((ruleCluster*)clusters[i]);
    }

    return smallest;
}

ruleCluster* MainWindow::findBiggestCluster()
{
    ruleCluster* biggest = ((ruleCluster*)clusters[0]);

    for(int i = 1; i < this->settings->stopCondition; i++)
    {
        if(biggest->size() < clusters[i]->size())
            biggest = ((ruleCluster*)clusters[i]);
    }

    return biggest;
}

int MainWindow::countUngroupedObjects()
{
    int result = 0;

    for(int i = 0; i < settings->stopCondition; ++i)
    {
        //If object is ungrouped it's size is equal to 1;
        if(clusters[i]->size()==1)
            ++result;
    }

    return result;
}

int MainWindow::countAllNodes()
{
    // Basic number of nodes in dendrogram.
    return 2 * settings->objectsNumber - settings->stopCondition;
}

int MainWindow::countCoverageSum()
{
    int coverageSum = 0;

    for(int i = 0; i < settings->stopCondition; ++i)
        coverageSum += ((ruleCluster*)clusters[i])->support;

    return coverageSum;
}

int MainWindow::countRuleLength(QString rule)
{
    return std::count(rule.begin(), rule.end(), '&')+1;
}

// TODO: Add additional classes for report generation.
// TODO: Investigate the issue with opening xml report in MS Excel.

QString MainWindow::createXMLReportContent()
{
    QString content = "";

    content += createXMLFileHeader();
    content += createXMLFileWorkbook();
    content += createXMLFileDocumentProperties();
    content += createXMLFileExcelWorkbook();
    content += createXMLFileStyles();
    content += createXMLFileTable();
    content += createXMLFileFooter();

    return content;
}

QString MainWindow::createXMLFileHeader()
{
    QString result = "<?xml version=\"1.0\"?>\n";
    result += "<?mso-application progid=\"Excel.Sheet\"?>\n";

    return result;
}

QString MainWindow::createXMLFileWorkbook()
{
    QString result = "<Workbook\n";
    result += "\txmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n";
    result += "\txmlns:o=\"urn:schemas-microsoft-com:office:office\"\n";
    result += "\txmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n";
    result += "\txmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n";
    result += "\txmlns:html=\"http://www.w3.org/TR/REC-html40\">\n";

    return result;
}

QString MainWindow::createXMLFileDocumentProperties()
{
    QString name = getenv("USERNAME");

    QString date = QDate::currentDate().toString("dd-MM-yyyy");
    date += "T";
    date += QTime::currentTime().toString("hh:mm:ss");
    date += "Z";

    QString result = "\t<DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\">\n";
    result += "\t\t<Author>"+ name +"</Author>\n";
    result += "\t\t<Created>"+ date +"</Created>\n";
    result += "\t\t<Application>CluVis</Application>\n";
    result += "\t</DocumentProperties>\n";

    return result;
}

QString MainWindow::createXMLFileExcelWorkbook()
{
    QString result = "\t<ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
    result += "\t\t<ProtectStructure>False</ProtectStructure>\n";
    result += "\t\t<ProtectWindows>False</ProtectWindows>\n";
    result += "\t</ExcelWorkbook>\n";

    return result;
}

QString MainWindow::createXMLFileStyles()
{
    QString result = "\t<Styles>\n";
    result += "\t\t<Style ss:ID=\"Header\">\n";
    result += "\t\t\t<Font ss:Bold=\"1\"/>\n";
    result += "\t\t</Style>\n";
    result += "\t</Styles>\n";

    return result;
}

QString MainWindow::createXMLFileTable()
{
    QString result = "\t<Worksheet ss:Name=\"Report\">\n";
    result += "\t\t<Table ss:ExpandedColumnCount=\"2\" ss:ExpandedRowCount=\"5\"\n";
    result += "\t\t\t x:FullColumns=\"1\" x:FullRows=\"1\">\n";
    result += "\t\t\t<Row>\n";
    result += createXMLFileTableHeader();
    result += "\t\t\t</Row>\n";
    result += "\t\t\t<Row ss:Index=\"1\">\n";
    result += createXMLFileTableContent();
    result += "\t\t\t</Row>\n";
    result += "\t\t\t<Row>\n";
    result += createXMLFileTableCell("==Dane skupień==",false);
    result += "\t\t\t</Row>\n";
    result += createXMLFileClustersData();
    result += "\t\t</Table>\n";

    return result;
}

QString MainWindow::createXMLFileFooter()
{
    QString result = "\t\t<WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
    result += "\t\t</WorksheetOptions>\n";
    result += "\t</Worksheet>\n";
    result += "</Workbook>";

    return result;
}

QString MainWindow::createXMLFileTableHeader()
{
    QString result = createXMLFileTableCell(tr("report.index"), true);
    result += createXMLFileTableCell(tr("report.nameOfBase"), true);
    result += createXMLFileTableCell(tr("report.attributesNumber"), true);
    result += createXMLFileTableCell(tr("report.objectsNumber"), true);
    result += createXMLFileTableCell(tr("report.nodesNumber"), true);
    result += createXMLFileTableCell(tr("report.clustersNumber"), true);
    result += createXMLFileTableCell(tr("report.coverageSum"), true);
    result += createXMLFileTableCell(tr("report.selectedVisualizationAlgorithm"), true);
    result += createXMLFileTableCell(tr("report.groupingAlgorithmUsed"), true);
    result += createXMLFileTableCell(tr("report.objectsSimilarityMeasure"), true);
    result += createXMLFileTableCell(tr("report.clustersSimilarityMeasure"), true);
    result += createXMLFileTableCell(tr("report.groupedRulePart"), true);
    result += createXMLFileTableCell(tr("report.biggestCluster"), true);
    result += createXMLFileTableCell(tr("report.smallestCluster"), true);
    result += createXMLFileTableCell(tr("report.ungroupedRules"), true);
    result += createXMLFileTableCell(tr("report.MDI"), true);
    result += createXMLFileTableCell(tr("report.MDBI"), true);
    result += createXMLFileTableCell(tr("report.maxMDI"), true);
    result += createXMLFileTableCell(tr("report.maxMDIClustersNumber"), true);
    result += createXMLFileTableCell(tr("report.maxMDBI"), true);
    result += createXMLFileTableCell(tr("report.maxMDBIClustersNumber"), true);
    return result;
}

QString MainWindow::createXMLFileTableCell(QString data, bool isHeader)
{
    int width =  data.length();
    QString result = "\t\t\t\t<Cell ss:Width=\"";
    result += QString::number(width) + "\"";

    if(isHeader)
        result += " StyleID=\"Header\"";

    result += ">\n\t\t\t\t\t<Data ss:Type=\"String\">";
    result += data;
    result += "</Data>\n";
    result += "\t\t\t\t</Cell>\n";

    return result;
}

QString MainWindow::createXMLFileTableContent()
{
    //Index
    QString result = createXMLFileTableCell("1", true);
    //Base's name
    result += createXMLFileTableCell(formatThickString(ui->labelObjectBaseName->text()), false);
    //Attributes number
    result += createXMLFileTableCell(QString::number(gSettings->attributesNumber), false);
    //Objects number
    result += createXMLFileTableCell(QString::number(getObjectsNumber()), false);
    //Nodes number
    result += createXMLFileTableCell(QString::number(countAllNodes()), false);
    //Clusters number
    result += createXMLFileTableCell(QString::number(settings->stopCondition), false);
    //Coverage sum
    result += createXMLFileTableCell(QString::number(countCoverageSum()), false);
    //Visualization algorithm
    result += createXMLFileTableCell(ui->comboBoxAlgorithmVisualization->currentText(), false);
    //Grouping data
    result += createXMLFileTableCell(formatThickString(ui->labelAlgorithmGroupingValue->text()), false);
    result += createXMLFileTableCell(ui->comboBoxInterobjectDistanceMeasure->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxInterclusterDistanceMeasure->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxRuleGroupedPart->currentText(), false);
    //Overall result data
    result += createXMLFileTableCell(findBiggestCluster()->name(), false);
    result += createXMLFileTableCell(findSmallestCluster()->name(), false);
    result += createXMLFileTableCell(QString::number(countUngroupedObjects()), false);
    //Indexes data
    result += createXMLFileTableCell(QString::number(MDI), false);
    result += createXMLFileTableCell(QString::number(MDBI), false);
    result += createXMLFileTableCell(QString::number(maxMDI), false);
    result += createXMLFileTableCell(QString::number(maxMDIClustersNumber), false);
    result += createXMLFileTableCell(QString::number(maxMDBI), false);
    result += createXMLFileTableCell(QString::number(maxMDBIClustersNumber), false);

    return result;
}

QString MainWindow::createXMLFileClustersData()
{
    QString result = createXMLFileClustersDataHeader();
    result += createXMLFileClustersDataContent();

    return result;
}

QString MainWindow::createXMLFileClustersDataHeader()
{
    QString result = "\t\t\t<Row>\n";
    result += createXMLFileTableCell(tr("report.index"), true);
    result += createXMLFileTableCell(tr("report.clustersName"), true);
    result += createXMLFileTableCell(tr("report.clustersSize"), true);
    result += createXMLFileTableCell(tr("report.clustersRulesPercent"), true); // Procent reguł w grupie [%]
    result += createXMLFileTableCell(tr("report.clustersNodesNumber"), true);
    result += createXMLFileTableCell(tr("report.clustersNodesNumberPercent"), true);
    result += createXMLFileTableCell(tr("report.clustersCoverage"), true);
    result += createXMLFileTableCell(tr("report.clustersCoveragePercent"), true);
    result += createXMLFileTableCell(tr("report.clustersRepresentative"), true);
    result += createXMLFileTableCell(tr("report.representativeLength"), true);
    result += "\t\t\t</Row>\n";

    return result;
}

QString MainWindow::createXMLFileClustersDataContent()
{

    /* Representative holder is needed so the
     * std::replace doesn't change symbols like <, > or &
     * inside actual rule in cluster[i]. */

    QString result = "", representativeHolder;

    for(int i = 0; i < settings->stopCondition; i++)
    {
        result += "\t\t\t<Row>\n";

        //Index
        result += createXMLFileTableCell(QString::number(i+1), false);
        //Cluster's name
        result += createXMLFileTableCell(clusters[i]->name(), false);
        //Rules number
        result += createXMLFileTableCell(QString::number(clusters[i]->size()), false);
        //Rules percent
        result += createXMLFileTableCell(QString::number(clusters[i]->size()*100/settings->objectsNumber), false);
        //Nodes number
        result += createXMLFileTableCell(QString::number(clusters[i]->nodesNumber()),false);
        //Nodes percent
        result += createXMLFileTableCell(
                    QString::number(clusters[i]->nodesNumber()*100/countAllNodes()), false);
        //Cluster's support
        result += createXMLFileTableCell(QString::number((((ruleCluster*)clusters[i])->support)),false);
        //Cluster's support percent
        result += createXMLFileTableCell(
                    QString::number((((ruleCluster*)clusters[i])->support*100)/countCoverageSum()), false);
        //Cluster's representative
        representativeHolder = ((ruleCluster*)clusters[i])->representative;
        result += createXMLFileTableCell(representativeHolder.replace("&","&amp;").replace("<","&lt;").replace(">", "&gt;"), false);
        //Cluster's representative length
        result += createXMLFileTableCell(QString::number(countRuleLength(((ruleCluster*)clusters[i])->representative)), false);

        result += "\t\t\t</Row>\n";
    }

    return result;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

//View

// TODO: There are some minor issues while changing laguague.

void MainWindow::on_actionAngielski_triggered()
{
    delete translator;

    translator = new QTranslator();
    QString transName = QApplication::applicationDirPath() + "/language/english.qm";

    if(!translator->load(transName))
        delete translator;
    else
        qApp->installTranslator(translator);

    ui->retranslateUi(this);
}

void MainWindow::on_actionPolski_triggered()
{
    delete translator;

    translator = new QTranslator();
    QString transName = QApplication::applicationDirPath() + "/language/polish.qm";

    if(!translator->load(transName))
        delete translator;
    else
        qApp->installTranslator(translator);

    ui->retranslateUi(this);
}

//Help
void MainWindow::on_actionAbout_triggered()
{
    About a;
    a.exec();

    addLogMessage(tr("log.programmeInformationOpened"));
}

//Rest
void MainWindow::on_pushButtonGroup_clicked()
{
    addLogMessage(tr("log.loadingGroupingSettings"));
    // Wczytytuję ustawienia grupowania...

    setGroupingSettings();

    addLogMessage(tr("log.validatingSettings"));
    // Sprawdzam poprawność ustawień...

    if(areSettingsCorrect())
    {
        addLogMessage(tr("log.settingsCorrect"));
        // Ustawienia poprawne.

        groupObjects();
    }
}

void MainWindow::on_pushButtonVisualize_clicked()
{    
    if(areObjectsClustered)
    {
        addLogMessage(tr("log.loadingVisualizationSettings"));
        // Wczytytuję ustawienia wizualizacji...

        setVisualizationSettings();

        scene->clear();
        connect(vThread,SIGNAL(passGraphicsRectObject(customGraphicsRectObject*)),
                this,SLOT(getGraphicsRectObject(customGraphicsRectObject*)));
        connect(vThread,SIGNAL(passGraphicsEllipseObject(customGraphicEllipseObject*)),
                this,SLOT(getGraphicsEllipseObject(customGraphicEllipseObject*)));
        connect(vThread,SIGNAL(passLogMsg(QString)),
                this,SLOT(gotLogText(QString)));
        connect(vThread,SIGNAL(passMainEllipseRect(QRect*)),
                this,SLOT(gotMainEllipseRect(QRect*)));

        addLogMessage(tr("log.visualizationSettingsLoaded"));
        // Ustawienia wizualizacji wczytane.

        visualize();
    }
    else
    {
        QString logText = tr("log.visualizationGenerationFailed") + " ";
        // Nieudana próba generowania wizualizacji.
        logText+= tr("log.objectsNotGrouped");
        // Nie pogrupowano obiektów.

        addLogMessage(logText);
    }
}

void MainWindow::setGroupingSettings()
{
    settings->dataTypeID = 0;
    settings->stopCondition =
            ui->spinBoxStopConditionValue->value();

    gSettings->groupingAlgorithmID = 0;
    gSettings->interobjectDistanceMeasureID =
            ui->comboBoxInterobjectDistanceMeasure->currentIndex();
    gSettings->interclusterDistanceMeasureID =
            ui->comboBoxInterclusterDistanceMeasure->currentIndex();
    gSettings->findBestClustering =
            ui->checkBoxSearchForBestIndexes->isChecked();

    addLogMessage(tr("log.generalSettingsLoaded"));
    // Wczytano ustawienia ogólne.

    switch(settings->dataTypeID)
    {
        case generalSettings::RSES_RULES_ID:

        gSettings_RSES->groupingPartID =
            ui->comboBoxRuleGroupedPart->currentIndex();

        gThread = new groupingThread(gSettings_RSES, gSettings, settings);

        connect(gThread,SIGNAL(passClusters(cluster**)),
                this,SLOT(getClusters(cluster**)));
        connect(gThread,SIGNAL(passLogMsg(QString)),
                this,SLOT(gotLogText(QString)));
        connect(gThread,SIGNAL(passMDIData(qreal, qreal, int)),
                this,SLOT(gotMDIData(qreal, qreal, int)));
        connect(gThread,SIGNAL(passMDBIData(qreal, qreal, int)),
                this,SLOT(gotMDBIData(qreal, qreal, int)));

        addLogMessage(tr("log.rsesRules.detailedSettingsLoaded"));
        // Wczytano ustawienia szczególne dla RSES Rules.

        break;

        default:
        ;
    }
}

void MainWindow::setVisualizationSettings()
{    
    vSettings->visualizationAlgorithmID =
            ui->comboBoxAlgorithmVisualization->currentIndex();
    vSettings->visualizeAllHierarchyLevels =
            ui->checkBoxVisualizeAllHierarchyLevels->isChecked();

    int sceneWidth = ui->graphicsView->width() - 2;
    int sceneHeight = ui->graphicsView->height() - 2;

    vSettings->sceneRect = new QRect(0,0,sceneWidth,sceneHeight);

    addLogMessage(tr("log.generalSettingsLoaded"));

    switch(settings->dataTypeID)
    {
        case generalSettings::RSES_RULES_ID:

        vSettings_RSES->clusteredRules = ((ruleCluster**)clusters);

        vThread = new visualizationThread(settings, vSettings, vSettings_RSES);

        addLogMessage(tr("report.detailedSettingsLoaded"));
        // Wczytano ustawienia szczególne.

        break;

        default:
        ;
    }
}

bool MainWindow::areSettingsCorrect()
{
    return isBaseLoaded() && isStopConditionCorrect();
}

bool MainWindow::isBaseLoaded()
{
    if(gSettings->objectBaseInfo.exists())
        return true;
    else
    {
        QString logText = tr("log.failedAttemptOfGrouping") + " ";
        // Nieudana próba generowania skupień.
        logText+= tr("log.baseNotSelected");
        // Nie wybrano bazy.

        addLogMessage(logText);

        return false;
    }
}

bool MainWindow::isStopConditionCorrect()
{
    if(ui->spinBoxStopConditionValue->value() > 0)
    {
        if(ui->spinBoxStopConditionValue->value() <= settings->objectsNumber)
        {
            return true;
        }
        else
        {
            QString logText = tr("log.failedAttemptOfGrouping") + " ";
            // Nieudana próba generowania skupień.
            logText+= tr("log.stopConditionToHigh");
            // Warunek stopu nie może być większy od liczby obiektów w bazie.

            addLogMessage(logText);

            return false;
        }
    }
    else
    {
        QString logText = tr("log.failedAttemptOfGrouping") + " ";
        logText+= tr("log.stopConditionToSmall");
        // Warunek stopu nie może być mniejszy od jedynki.

        addLogMessage(logText);

        return false;
    }
}

//End GUI

//Functionality

void MainWindow::groupObjects()
{
    addLogMessage(tr("log.groupingStarted"));
    // Rozpoczynam grupowanie...

    addLogMessage(tr("log.blockingButtonsForOperation"));
    // Blokuję przyciski na czas operacji...

    ui->pushButtonVisualize->setEnabled(false);
    ui->pushButtonGroup->setEnabled(false);

    areObjectsClustered = false;

    addLogMessage(tr("log.groupingThreadStarted"));
    // Rozpoczynam wątek grupujący...

    tim->start();
    gThread->run();
    int timeElapsed = tim->elapsed();

    QString logText = QString(tr("log.groupingThreadFinishedIn"))
            .arg(QString::number(timeElapsed));
    // Wątek grupujący zakończony w czasie %1 ms.

    addLogMessage(logText);

    ui->pushButtonVisualize->setEnabled(true);
    ui->pushButtonGroup->setEnabled(true);

    addLogMessage(tr("log.buttonsUnlocked"));

    if(areObjectsClustered)
    {
        QMessageBox mb;
        QString infoText;

        infoText = tr("FD.clustersNumber") + ": "
                + QString::number(settings->stopCondition) + ".\n";
        // Liczba otrzymanych skupień
        infoText += tr("FD.clustersMDI") + ": "
                + QString::number(MDI) + ".\n";
        // MDI otrzymanych skupień
        infoText += tr("FD.clustersMDBI") + ": "
                + QString::number(MDBI) + ".\n";
        // MDBI otrzymanych skupień

        infoText += tr("FD.visualizeGrouping");
        // Czy chcesz wygenerować wizualizację struktury grup?

        mb.setWindowTitle(tr("FD.groupingFinished"));
        // Grupowanie zakończone
        mb.setText(tr("FD.groupingFinished") + ".");
        mb.setInformativeText(infoText);
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        mb.setButtonText(QMessageBox::Yes, tr("FD.yes"));
        mb.setButtonText(QMessageBox::No, tr("FD.no"));
        mb.setDefaultButton(QMessageBox::Yes);

        if(mb.exec() == QMessageBox::Yes)
            on_pushButtonVisualize_clicked();
    }
}

void MainWindow::visualize()
{
    addLogMessage(tr("log.visualizationStarted"));

    addLogMessage(tr("log.blockingButtonsForOperation"));

    ui->pushButtonVisualize->setEnabled(false);
    ui->pushButtonGroup->setEnabled(false);

    addLogMessage(tr("log.visualizationThreadStarted"));

    tim->start();
    vThread->run();
    int timeElapsed = tim->elapsed();

    QString logText = QString(tr("log.visualizationThreadFinishedIn"))
            .arg(QString::number(timeElapsed));
    // Wątek wizualizujący zakończony w czasie %1 ms.

    addLogMessage(logText);

    if(vSettings->visualizationAlgorithmID == vSettings->RT_SLICE_AND_DICE_ID)
        ui->graphicsView->fitInView(scene->itemsBoundingRect());
    if(vSettings->visualizationAlgorithmID == vSettings->CIRCULAR_TREEMAP_ID)
        ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    addLogMessage(tr("log.visualizationCentered"));
    // Wizualizacja wycentrowana.

    ui->pushButtonVisualize->setEnabled(true);
    ui->pushButtonGroup->setEnabled(true);

    addLogMessage(tr("log.buttonsUnlocked"));
    //Przyciski odblokowane.   

    QMessageBox::information(this,
                            tr("FD.visualizationFinished.title"),
                            tr("FD.visualizationFinished.msg"), // Wizualizowanie zakończone!
                            QMessageBox::Ok);

    ui->actionSaveVisualization->setEnabled(true);
    ui->actionGenerateReport->setEnabled(true);
}

void MainWindow::getClusters(cluster** c)
{
    addLogMessage(tr("log.groupedObjectsReceived"));
    // Otrzymano pogrupowane obiekty.
    addLogMessage(tr("log.visualizationAvailable"));
    // Można przystąpić do wizualizacji.

    clusters = c;

    areObjectsClustered = true;
    ui->labelIsBaseGrouped->setText(tr("bold.grouped"));
}

void MainWindow::gotMDIData(qreal MDI, qreal maxMDI, int maxMDIClustersNumber)
{
    this->MDI = MDI;
    this->maxMDI = maxMDI;
    this->maxMDIClustersNumber = maxMDIClustersNumber;
}

void MainWindow::gotMDBIData(qreal MDBI, qreal maxMDBI, int maxMDBIClustersNumber)
{
    this->MDBI = MDBI;
    this->maxMDBI = maxMDBI;
    this->maxMDBIClustersNumber = maxMDBIClustersNumber;
}

void MainWindow::getGraphicsRectObject(customGraphicsRectObject *object)
{
    scene->addItem(object);
    connect(object,SIGNAL(ruleClusterPicked(ruleCluster*)),
            this,SLOT(gotRuleClusterToVisualize(ruleCluster*)));
    connect(object,SIGNAL(getClusterInfo(ruleCluster*)),
            this,SLOT(showRuleInfo(ruleCluster*)));
}

void MainWindow::getGraphicsEllipseObject(customGraphicEllipseObject *object)
{
    scene->addItem(object);
    connect(object,SIGNAL(ruleClusterPicked(ruleCluster*)),
            this,SLOT(gotRuleClusterToVisualize(ruleCluster*)));
    connect(object,SIGNAL(getClusterInfo(ruleCluster*)),
            this,SLOT(showRuleInfo(ruleCluster*)));
}

void MainWindow::gotMainEllipseRect(QRect *r)
{
    scene->addEllipse(*r);
}

void MainWindow::gotRuleClusterToVisualize(ruleCluster *c)
{
    addLogMessage(tr("log.clusterSelectedForVisualization"));
    // Wybrano skupienie do wizualizacji. Wizualizuję...

    ruleCluster newC = *c;

    scene->clear();

    vThread->run(&newC);
}

void MainWindow::showRuleInfo(ruleCluster *c)
{
    addLogMessage(tr("log.showClustersInfo"));
    // Wyświetlam informacje dotyczące wybranego skupienia.

    cInfo = new clusterInfo_RSESRules(c);

    connect(cInfo,SIGNAL(passClusterToVisualize(ruleCluster*)),
            this,SLOT(gotRuleClusterToVisualize(ruleCluster*)));

    cInfo->exec();
}

void MainWindow::gotLogText(QString msg)
{
    addLogMessage(msg);
}

void MainWindow::addLogMessage(QString msg)
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += msg;

    ui->textBrowserLog->append(logText);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if(e->oldSize().width() != -1 && e->oldSize().height() != -1)
    {
        if(vSettings->visualizationAlgorithmID == vSettings->RT_SLICE_AND_DICE_ID)
        {
            ui->graphicsView->fitInView(scene->itemsBoundingRect());
        }

        if(vSettings->visualizationAlgorithmID == vSettings->CIRCULAR_TREEMAP_ID)
        {
            ui->graphicsView->fitInView(scene->itemsBoundingRect(),Qt::KeepAspectRatio);
        }
    }
}
