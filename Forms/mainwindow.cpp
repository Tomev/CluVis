#include "mainwindow.h"
#include "about.h"
#include "ui_mainwindow.h"
#include "Validators/rsesrulebasevalidator.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <QtCore>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QtGui>
#include <QDesktopWidget>
#include <QTime>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    settings = new generalSettings();
    gSettings = new groupingSettings_General();
    vSettings = new visualizationSettings_general();
    tim = new QTime();

    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setSceneRect(0,0,0,0);

    ui->labelIsBaseGrouped->setText("");

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

    /*
     * Automatically translate to english.
     *
     * Initializing with new QTranslator, so delete at the begining of transale
     * method won't crash.
     */

    translator = new QTranslator();
    translate(english);

    addLogMessage(tr("log.applicationStart"));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
    delete settings;
    delete gSettings;
    delete dGrpSettings;
    delete vSettings;
}

//File

void MainWindow::on_actionLoadObjectBase_triggered()
{
    QFileInfo KB = selectObjectBase();

    //rsesRuleBaseValidator fileValidator(&KB);

    //fileValidator.validate();

    loadObjectsBase(KB);
}

QFileInfo MainWindow::selectObjectBase()
{
    QFileDialog FD;

    QString openPath = getOpenPath();

    QString OBPath = FD.getOpenFileName(
        this,
        tr("FD.selectKnowledgeBase"),
        openPath,
        tr("FD.RSESRules.fileTypes")
    );

    return QFileInfo(OBPath);
}

QString MainWindow::getOpenPath()
{
    // TR TODO: Create settings file to store user settings.

    QString customOpenPath = "D:\\Dysk Google\\Rules";
    QString defaultOpenPath = "C:\\";

    if(QDir(customOpenPath).exists()) return customOpenPath;
    else return defaultOpenPath;
}

void MainWindow::loadObjectsBase(QFileInfo OB)
{
  QString OBName = OB.completeBaseName();

  if(!canBaseBeLoaded(OB)) return;

    if(isRuleSet(OB) == false)
    {
        QString logText = tr("log.failedToLoadObjBase") + " ";
        logText+= tr("log.selectedFileIsNotKnowledgeBase");

        addLogMessage(logText);

        return;
    }

    ui->actionSaveVisualization->setEnabled(false);
    ui->actionGenerateReport->setEnabled(false);
    ui->labelIsBaseGrouped->setText(tr("bold.ungrouped"));
    areObjectsClustered = false;

    gSettings->objectBaseInfo = OB;

    OBName += "."+gSettings->objectBaseInfo.suffix();
    settings->objectsNumber = getObjectsNumber();

    ui->labelObjectBaseName->setText("<b>"+OBName+"</b>");
    ui->labelObjectsNumberValue->setText("<b>"+QString::number(settings->objectsNumber)+".</b>");
    ui->spinBoxStopConditionValue->setMaximum(settings->objectsNumber);
    ui->spinBoxStopConditionValue->setValue(1);

    addLogMessage(tr("log.knowledgeBaseLoaded") + " " + OBName + ".");
}

bool MainWindow::canBaseBeLoaded(QFileInfo OB)
{
  QString OBName = OB.completeBaseName();

  return  wasFileSelected(OBName) ||
          isRuleSet(OB);
}

bool MainWindow::wasFileSelected(QString OBName)
{
  if(OBName == "")
  {
      qDebug() << tr("log.fileNotLoaded");

      return false;
    }

  return true;
}

bool MainWindow::isRuleSet(QFileInfo base)
{
    QString line;

    QFile KB(base.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        line = in.readLine();

        if(line.contains("RULE_SET")) return true;
    }

    return false;
}

int MainWindow::getObjectsNumber()
{
    switch(settings->dataTypeID)
    {
        case RSESRulesId:
            return groupingSettings_RSESRules::getRSESRulesNumber(
                    gSettings->objectBaseInfo);
        default:
            return -1;
    }

    return -1;
}

void MainWindow::on_actionSaveVisualization_triggered()
{
    QFileDialog FD;

    QString savePath = "C:/";

    QString filePath = FD.getSaveFileName(
        this,
        tr("FD.save"),   // Zapisz
        savePath,
        tr("FD.png")      // *.png
     );

    saveVisualization(filePath);

}

void MainWindow::saveVisualization(QString path)
{
    if(path == "")
    {
        QString logText = tr("log.failedToSaveVisualization") + " ";
        logText+= tr("log.fileNameNotSelected");

        addLogMessage(logText);

        return;
    }

    if(path.right(4) != ".png")
        path += ".png";

    QRect sceneRect(ui->graphicsView->frameGeometry().left() + 15,
                    ui->graphicsView->frameGeometry().top() + 55,
                    ui->graphicsView->frameGeometry().width() - 3,
                    ui->graphicsView->height()-3);


    QPixmap pixMap = QWidget::grab(sceneRect);
    pixMap.save(path);

    addLogMessage(
        QString(tr("log.visualizationSavedWithName"))
        .arg(path)
    );
}

void MainWindow::on_actionGenerateReport_triggered()
{
    generateReport();
}

void MainWindow::generateReport(QString path)
{
    QString filePath = path;

    if(path == "")
         filePath = getFilePath();
    else
        filePath = path;

    if(filePath == "")
    {
        addLogMessage(tr("log.operationAborted"));

        return ;
    }

    int fileType = getFileType(filePath.at(filePath.length()-1));

    QString reportContent = createReportContent(fileType);

    QFile report(filePath);

    addLogMessage(tr("log.reportGenerationStarted"));

    if(!report.open(QFile::WriteOnly | QFile::Text) && report.exists())
    {
        QString reportInfo = tr("log.unableToOpenFileForSaving") + " ";
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

/*
 * File type is determined by it's name (*.xml / *.txt), so
 * last letter of file's name is enough to detemine it's
 * type. At least in this case.
 */

int MainWindow::getFileType(QChar t)
{
    if(t=='l')
        return XmlReportId;
    if(t=='t')
        return TxtReportId;

    return -1;
}

QString MainWindow::createReportContent(int type)
{
    QString content;

    switch(type)
    {
    case TxtReportId:
        content = createTXTReportContent();
        break;
    case XmlReportId:
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
            + ui->comboBoxInterObjectSimMeasure->currentText();
    // Wybrana miara podobieństwa obiektów
    content += "\n" + tr("report.clustersSimilarityMeasure") + ": "
            + ui->comboBoxInterClusterSimMeasure->currentText();
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
                + QString::number((((ruleCluster*)(clusters[i].get()))->support*100)/countCoverageSum()) + "%";
        // Pokrycie skupienia
        //content += "\n\t" + tr("report.clustersRepresentative") + ": "
                //+ ((ruleCluster*)clusters[i])->representative;
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

cluster *MainWindow::findSmallestCluster()
{
    ruleCluster* smallest = ((ruleCluster*)clusters[0].get());

    for(int i = 1; i < settings->stopCondition; ++i)
    {
        if(smallest->size() > clusters[i]->size())
            smallest = ((ruleCluster*)clusters[i].get());
    }

    return smallest;
}

int MainWindow::getBiggestRepresentativeLength()
{
    int biggestRepSize = 0;
    ruleCluster* c;

    for(int i = 0; i < settings->stopCondition; ++i)
    {
        c = static_cast<ruleCluster*>(clusters[i].get());

        if(biggestRepSize < countRuleLength(c->representative()))
            biggestRepSize = countRuleLength(c->representative());
    }

    return biggestRepSize;
}

int MainWindow::getSmallestRepresentativeLength()
{
    int smallestRepSize = 0;
    ruleCluster* c;

    for(int i = 0; i < settings->stopCondition; ++i)
    {
        c = static_cast<ruleCluster*>(clusters[i].get());

        if(smallestRepSize > countRuleLength(c->representative()))
            smallestRepSize = countRuleLength(c->representative());
    }

    return smallestRepSize;
}

qreal MainWindow::getAverageRepresentativeLength()
{
    qreal averageRepSize = 0;
    ruleCluster* c;

    for(int i = 0; i < settings->stopCondition; ++i)
    {
        c = static_cast<ruleCluster*>(clusters[i].get());
        averageRepSize += countRuleLength(c->representative());
    }

    averageRepSize /= settings->stopCondition;

    return averageRepSize;
}

cluster* MainWindow::findBiggestCluster()
{
    ruleCluster* biggest = ((ruleCluster*)clusters[0].get());

    for(int i = 1; i < this->settings->stopCondition; i++)
    {
        if(biggest->size() < clusters[i]->size())
            biggest = ((ruleCluster*)clusters[i].get());
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
        coverageSum += ((ruleCluster*)clusters[i].get())->support;

    return coverageSum;
}

int MainWindow::countRuleLength(QString rule)
{
    if(rule.startsWith('='))
        return 0;

    return rule.count(")&(")+1;
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
    result += createXMLFileTableCell(tr("report.biggestClusterSize"), true);
    result += createXMLFileTableCell(tr("report.biggestClusterRepLength"), true);
    result += createXMLFileTableCell(tr("report.smallestCluster"), true);
    result += createXMLFileTableCell(tr("report.ungroupedRules"), true);
    result += createXMLFileTableCell(tr("report.biggestRepSize"), true);
    result += createXMLFileTableCell(tr("report.smallestRepSize"), true);
    result += createXMLFileTableCell(tr("report.averageRepSize"), true);
    result += createXMLFileTableCell(tr("report.zeroRepresentativeClusterOccurence"), true);
    result += createXMLFileTableCell(tr("report.representativeCreationStrategy"), true);
    result += createXMLFileTableCell(tr("report.representativeCreationThreshold"), true);
    result += createXMLFileTableCell(tr("report.MDI"), true);
    result += createXMLFileTableCell(tr("report.MDBI"), true);
    result += createXMLFileTableCell(tr("report.minMDI"), true);
    result += createXMLFileTableCell(tr("report.minMDIClustersNumber"), true);
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
    result += createXMLFileTableCell(ui->comboBoxInterObjectSimMeasure->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxInterClusterSimMeasure->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxRuleGroupedPart->currentText(), false);
    //Overall result data
    result += createXMLFileTableCell(findBiggestCluster()->name(), false);
    result += createXMLFileTableCell(QString::number(findBiggestCluster()->size()), false);
    //result += createXMLFileTableCell(QString::number(countRuleLength(findBiggestCluster()->representative())), false);
    result += createXMLFileTableCell(findSmallestCluster()->name(), false);
    result += createXMLFileTableCell(QString::number(countUngroupedObjects()), false);
    // Representatives info
    result += createXMLFileTableCell(QString::number(getBiggestRepresentativeLength()), false);
    result += createXMLFileTableCell(QString::number(getSmallestRepresentativeLength()), false);
    result += createXMLFileTableCell(QString::number(getAverageRepresentativeLength()), false);
    result += createXMLFileTableCell(QString::number(gSettings->zeroRepresentativeClusterOccurence), false);
    result += createXMLFileTableCell(ui->comboBoxRepresentativeCreationStrategy->currentText(), false);
    result += createXMLFileTableCell(QString::number(gSettings->repTreshold), false);
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
    result += createXMLFileTableCell(tr("report.clustersRulesPercent"), true);
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
        //TODO: RECONSIDER
        result += createXMLFileTableCell(QString::number((((ruleCluster*)clusters[i].get())->support)),false);
        //Cluster's support percent
        result += createXMLFileTableCell(
                    QString::number((((ruleCluster*)clusters[i].get())->support*100)/countCoverageSum()), false);
        //Cluster's representative
        representativeHolder = ((ruleCluster*)clusters[i].get())->representative();
        result += createXMLFileTableCell(representativeHolder.replace("&","&amp;").replace("<","&lt;").replace(">", "&gt;"), false);
        //Cluster's representative length
        result += createXMLFileTableCell(QString::number(countRuleLength(((ruleCluster*)clusters[i].get())->representative())), false);

        result += "\t\t\t</Row>\n";
    }

    return result;
}

void MainWindow::on_actionMergeReports_triggered()
{
    // Select path to folders with reports

    // TODO: Change for editable dir.
    // TODO: D:/ANB/ must exists for this to work. Eliminate this problem.
    QString reportsDirPath;

    // Get dir path of reports folder
    reportsDirPath = QFileDialog::getExistingDirectory
                (
                    this,
                    tr("Select directory"),
                    "C:/",
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );

    // Return if no dir was selected
    if(reportsDirPath == "")
    {
        // TODO: Log that no dir path was selected.
        qDebug() << "No directory selected.";
        return;
    }

    qDebug() << "Directory selected: " + reportsDirPath + ".";

    // Generate report content
    // TODO: It's similar to generating XML report. Consider joining both of them.
    QString reportContent = "";

    reportContent += createXMLFileHeader();
    reportContent += createXMLFileWorkbook();
    reportContent += createXMLFileDocumentProperties();
    reportContent += createXMLFileExcelWorkbook();
    reportContent += createXMLFileStyles();
    reportContent += "\t<Worksheet ss:Name=\"Report\">\n";
    reportContent += "\t\t<Table ss:ExpandedColumnCount=\"2\" ss:ExpandedRowCount=\"5\"\n";
    reportContent += "\t\t\t x:FullColumns=\"1\" x:FullRows=\"1\">\n";
    reportContent += "\t\t\t<Row>\n";
    reportContent += createXMLFileTableHeader();
    reportContent += "\t\t\t</Row>\n";

    getReportsMainContentFromLocation(reportsDirPath, &reportContent);

    reportContent += "\t\t</Table>\n";
    reportContent += createXMLFileFooter();

    // Save report

    QFile collectiveReport(reportsDirPath + "/collectiveReport.xml");

    if(!collectiveReport.open(QFile::WriteOnly | QFile::Text) && collectiveReport.exists())
    {
        QString reportInfo = tr("log.unableToOpenFileForSaving") + " ";
        reportInfo += tr("log.operationAborted");
        addLogMessage(reportInfo);

        return;
    }

    QTextStream outStream(&collectiveReport);
    outStream.setCodec("UTF-8");
    outStream << reportContent;

    collectiveReport.flush();
    collectiveReport.close();

    qDebug() << "Collective report generated.";
}

void MainWindow::getReportsMainContentFromLocation(QString reportsDirPath, QString* content)
{
    QDir reportsDir(reportsDirPath);
    // Check if this dir has subdirs

    if(reportsDir.count() > 0)
    {
        // If so recursive call this function for each subdir
        QStringList subdirs = reportsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString subdirName, subdirs)
            getReportsMainContentFromLocation(reportsDirPath + "/" + subdirName, content);
    }

    // Find names of all reports in given dir
    QStringList reportsNames;
    QString reportsPath;

    QTextStream in;
    QString line = "";

    reportsNames = reportsDir.entryList({"*.xml"});

    // For each report on the list
    foreach(const QString reportName, reportsNames)
    {
        // TODO: Validate if given xml is indeed report in proper format
        reportsPath = reportsDirPath + "/" + reportName;

        // Read file line by line until row with data is found
        QFile reportsFile(reportsPath);

        // Return if file cannot be opened
        if(!reportsFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Cannot open file for reading.";
        }

        QTextStream in(&reportsFile);

        line = in.readLine();

        while(!in.atEnd())
        {
            if(!line.contains("<Row ss:Index=\"1\">"))
            {
                line = in.readLine();
                continue;
            }
            else
            {
                // When found add data to content
                while(!line.contains("</Row>"))
                {
                    *content += line + "\n";
                    line = in.readLine();
                }

                // Close row
                *content += line + "\n";

                break;
            }
        }

    }
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit(0);
}

//View

// TODO: There are some minor issues while changing laguague.
void MainWindow::on_actionEnglish_triggered()
{
    translate(english);
}

void MainWindow::on_actionPolish_triggered()
{
    translate(polish);
}

void MainWindow::translate(int lang)
{
    if(translator != NULL) delete translator;

    translator = new QTranslator();

    QString transName = QApplication::applicationDirPath();

    switch(lang)
    {
        case polish:
            transName += "/language/polish.qm";
            break;
        case english:
        default:
            transName += "/language/english.qm";
    }

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

    setGroupingSettings();

    addLogMessage(tr("log.validatingSettings"));

    if(areSettingsCorrect())
    {
        addLogMessage(tr("log.settingsCorrect"));

        groupObjects();
    }

    addLogMessage(tr("log.groupedObjectsReceived"));
    // Otrzymano pogrupowane obiekty.
    addLogMessage(tr("log.visualizationAvailable"));
    // Można przystąpić do wizualizacji.

    areObjectsClustered = true;
    ui->labelIsBaseGrouped->setText(tr("bold.grouped"));

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

void MainWindow::on_pushButtonVisualize_clicked()
{    
    if(areObjectsClustered)
    {
        addLogMessage(tr("log.loadingVisualizationSettings"));

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

        visualize();
    }
    else
    {
        QString logText = tr("log.visualizationGenerationFailed") + " ";
        logText+= tr("log.objectsNotGrouped");

        addLogMessage(logText);
    }
}

void MainWindow::setGroupingSettings()
{
    settings->dataTypeID = 0;
    settings->stopCondition =
            ui->spinBoxStopConditionValue->value();
    settings->clusters = &clusters;

    gSettings->groupingAlgorithmID = 0;
    gSettings->attributesFrequencyPercent =
            ui->spinBoxRepresentativeAttributePercent->value();
    gSettings->interObjectSimMeasureID =
            ui->comboBoxInterObjectSimMeasure->currentIndex();
    gSettings->interClusterSimMeasureID =
            ui->comboBoxInterClusterSimMeasure->currentIndex();
    gSettings->findBestClustering =
            ui->checkBoxSearchForBestIndexes->isChecked();
    gSettings->repTreshold =
            ui->spinBoxRepresentativeAttributePercent->value();
    gSettings->repCreationStrategyID = ui->comboBoxRepresentativeCreationStrategy->currentIndex();
    gSettings->inequityThreshold = ui->lineEdit_inequalityThreshold->text().toDouble();

    addLogMessage(tr("log.generalSettingsLoaded"));

    switch(settings->dataTypeID)
    {
        case RSESRulesId:
        {
            groupingSettings_RSESRules* temp = new groupingSettings_RSESRules();

            temp->groupedPartID =
                ui->comboBoxRuleGroupedPart->currentIndex();

            dGrpSettings = temp;

            if(gThread == 0)
              gThread = new groupingThread(dGrpSettings, gSettings, settings);
            else
              gThread->setSettings(dGrpSettings, gSettings, settings);

            addLogMessage(tr("log.rsesRules.detailedSettingsLoaded"));

            break;
        }
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
    vSettings->clusters = &clusters;

    addLogMessage(tr("log.generalSettingsLoaded"));

    switch(settings->dataTypeID)
    {
        case RSESRulesId:

            vThread = new visualizationThread(settings, vSettings);

            addLogMessage(tr("report.detailedSettingsLoaded"));

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
    {
        return true;
    }
    else
    {
        QString logText = QObject::tr("log.failedAttemptOfGrouping") + " ";
        logText+= tr("log.baseNotSelected");

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
            logText+= tr("log.stopConditionToHigh");

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

    addLogMessage(tr("log.blockingButtonsForOperation"));

    ui->pushButtonVisualize->setEnabled(false);
    ui->pushButtonGroup->setEnabled(false);

    areObjectsClustered = false;

    addLogMessage(tr("log.groupingThreadStarted"));

    tim->start();
    gThread->run();

    int timeElapsed = tim->elapsed();

    QString logText = QString(tr("log.groupingThreadFinishedIn"))
            .arg(QString::number(timeElapsed));

    addLogMessage(logText);

    ui->pushButtonVisualize->setEnabled(true);
    ui->pushButtonGroup->setEnabled(true);

    addLogMessage(tr("log.buttonsUnlocked"));
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

    if(vSettings->visualizationAlgorithmID == SliceAndDiceRTId)
        ui->graphicsView->fitInView(scene->itemsBoundingRect());
    if(vSettings->visualizationAlgorithmID == CircularTreemapId)
        ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    addLogMessage(tr("log.visualizationCentered"));
    // Wizualizacja wycentrowana.

    ui->pushButtonVisualize->setEnabled(true);
    ui->pushButtonGroup->setEnabled(true);

    addLogMessage(tr("log.buttonsUnlocked"));
    //Przyciski odblokowane.   

    ui->actionSaveVisualization->setEnabled(true);
    ui->actionGenerateReport->setEnabled(true);
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
        if(vSettings->visualizationAlgorithmID == SliceAndDiceRTId)
        {
            ui->graphicsView->fitInView(scene->itemsBoundingRect());
        }

        if(vSettings->visualizationAlgorithmID == CircularTreemapId)
        {
            ui->graphicsView->fitInView(scene->itemsBoundingRect(),Qt::KeepAspectRatio);
        }
    }
}

// New buttons

void MainWindow::on_checkBoxVisualizeAllHierarchyLevels_clicked()
{
    ui->checkBoxVisualizeAllHierarchyLevelsVisualizator->setCheckState
            (ui->checkBoxVisualizeAllHierarchyLevels->checkState());
}

void MainWindow::on_checkBoxVisualizeAllHierarchyLevelsVisualizator_clicked()
{
    ui->checkBoxVisualizeAllHierarchyLevels->setCheckState
            (ui->checkBoxVisualizeAllHierarchyLevelsVisualizator->checkState());
}

void MainWindow::on_comboBoxAlgorithmVisualization_currentIndexChanged(int index)
{
    ui->comboBoxAlgorithmVisualizationVisualizator->setCurrentIndex(index);
}

void MainWindow::on_comboBoxAlgorithmVisualizationVisualizator_currentIndexChanged(int index)
{
    ui->comboBoxAlgorithmVisualization->setCurrentIndex(index);
}

void MainWindow::on_pushButtonVisualizeVisualizator_clicked()
{
    if(areObjectsClustered)
    {
        addLogMessage(tr("log.loadingVisualizationSettings"));

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

        visualize();
    }
    else
    {
        QString logText = tr("log.visualizationGenerationFailed") + " ";
        logText+= tr("log.objectsNotGrouped");

        addLogMessage(logText);
    }
}

void MainWindow::on_spinBoxInterObjectMarginVisualizator_valueChanged(int arg1)
{
    ui->spinBoxInterObjectMargin->setValue(arg1);
}

void MainWindow::on_spinBoxInterObjectMargin_valueChanged(int arg1)
{
    ui->spinBoxInterObjectMarginVisualizator->setValue(arg1);
}

void MainWindow::on_pushButtonStandard_clicked()
{
    // TODO: Change for editable dir.
    // TODO: D:/ANB/ must exists for this to work. Eliminate this problem.
    QString baseDir = "D:/Dysk Google/Rules/",
            targetDir,
            kbsDirPath;

    QStringList kbNames;

    // Get dir path of KBs folder
    kbsDirPath = QFileDialog::getExistingDirectory
                (
                    this,
                    tr("Select directory"),
                    "D:/Dysk Google/Rules",
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );

    // Return if no dir was selected
    if(kbsDirPath == "")
    {
        // TODO: Log that no dir path was selected.
        return;
    }

    // Get KB names from that dir (only bases with extensions .rul are considered KBs)
    QDir kbsDir(kbsDirPath);
    kbNames = kbsDir.entryList({"*.rul"});

    // For each KB on the list
    for(int kbi = 0; kbi < kbNames.length(); ++kbi)
    {
        //
        QFileInfo KB(kbsDirPath + "/" + kbNames.at(kbi));

        loadObjectsBase(KB);

        // Set targetDir to point at current KB folder
        targetDir = baseDir + kbNames.at(kbi).split(".rul").at(0);

        ruleInterferencer.loadFactsFromPath(targetDir + ".fct");

        // Check if folder with the same name  as KB doesn't exist.
        QDir kbFolder(targetDir);

        if(! kbFolder.exists())
        {
            // If so create folder with name same as KB name.
            qDebug() << "creating folder " + targetDir;
            qDebug() << "creating commented out";
            //kbFolder.mkdir(targetDir);
        }
        else
        {
            // TODO: Communicate that folder exists.
            qDebug() << "NOT creating folder";
        }

        // Count number of iterations and get starting number
        int rulesNumberSqrt = qCeil(qSqrt(settings->objectsNumber));
        int rulesNumberOnePercent = qCeil(settings->objectsNumber/100.0);

        int numberOfIterationsDown = 0,
            numberOfIterationsUp = 0,
            numberOfIterations, start;

        // Get number of iterations
        for(int i = 0; rulesNumberSqrt - i * rulesNumberOnePercent > 0; ++i)
        {
          start = rulesNumberSqrt - i * rulesNumberOnePercent;
          ++numberOfIterationsUp;
        }
        for(int i = 0; rulesNumberSqrt + i * rulesNumberOnePercent > 0; ++i) ++numberOfIterationsDown;

        numberOfIterations = qMin(numberOfIterationsUp, numberOfIterationsDown);

        // As it was meant to go both ways, up and down from the sqrt
        numberOfIterations *= 2;

        // For each inter object similarity measure
        for(int osm = 0; osm < ui->comboBoxInterObjectSimMeasure->count(); ++osm)
        {
            // Reset target dir.
            targetDir = baseDir + kbNames.at(kbi).split(".rul").at(0);

            // Change index of object similarity measure combobox
            ui->comboBoxInterObjectSimMeasure->setCurrentIndex(osm);

            // Update dir to point inside folder of this similarity measure.
            targetDir += "/" + ui->comboBoxInterObjectSimMeasure->currentText();

            // Check if folder with the same name doesn't exist.
            QDir objectSimFolder(targetDir);

            if(! objectSimFolder.exists())
            {
                // If so create folder with name same as KB name.
                qDebug() << "creating folder " + targetDir;
                objectSimFolder.mkdir(targetDir);
            }
            else
            {
                // Communicate that folder exists.
                qDebug() << "NOT creating folder";
            }

            // For each inter cluster similarity measure
            for(int csm = 0; csm < ui->comboBoxInterClusterSimMeasure->count(); ++csm)
            {
                // Change index of cluster similarity measure combobox
                ui->comboBoxInterClusterSimMeasure->setCurrentIndex(csm);

                // For each representative creation strategy
                for(int rcs = 0; rcs < ui->comboBoxRepresentativeCreationStrategy->count(); ++rcs)
                {
                  // Change index of representative creation strategy
                  ui->comboBoxRepresentativeCreationStrategy->setCurrentIndex(rcs);

                  QList<int> thresholds;

                  // Check if rcs is threshold oriented or not
                  if(rcs == 0 || rcs == 3)
                  {
                    thresholds = {30, 25, 50, 75};
                  }
                  else
                    thresholds = {30};

                  // For each threshold
                  for(int tres : thresholds)
                  {
                    // Set this threshold
                    ui->spinBoxRepresentativeAttributePercent->setValue(tres);

                    int desiredClustersNumber = start + (numberOfIterations-1) * rulesNumberOnePercent;

                    while(desiredClustersNumber > settings->objectsNumber)
                      desiredClustersNumber -= rulesNumberOnePercent;

                    ui->spinBoxStopConditionValue->setValue(desiredClustersNumber);

                    setGroupingSettings();

                    qDebug() << "Grouping: "
                             << ui->comboBoxInterClusterSimMeasure->currentText()
                             << ", "
                             << ui->comboBoxInterObjectSimMeasure->currentText()
                             << ", "
                             << ui->comboBoxRepresentativeCreationStrategy->currentText()
                             << " "
                             << ui->spinBoxRepresentativeAttributePercent->value();

                    groupObjects();

                    on_pushButtonInterfere_clicked();

                    desiredClustersNumber -= rulesNumberOnePercent;

                    qDebug() << "Continuing grouping.";

                    while(desiredClustersNumber > 0)
                    {
                      qDebug() << desiredClustersNumber;

                      // Perform grouping for given clusters number with default settings.
                      ui->spinBoxStopConditionValue->setValue(desiredClustersNumber);

                      setGroupingSettings();

                      gThread->continueGrouping();

                      // Generate report of this grouping in dir.
                      //QString reportName = ui->comboBoxInterClusterSimMeasure->currentText() + " " + QString::number(desiredClustersNumber);
                      //generateReport(targetDir + "/" + reportName + ".xml");
                      on_pushButtonInterfere_clicked();

                      desiredClustersNumber -= rulesNumberOnePercent;
                    }

                    qDebug() << "Next measure.";
                  }
                }
            }
        }
    }

    qDebug() << "A splendid finish.";
}

void MainWindow::on_pushButtonInterfere_clicked()
{
  // GENERATING RANDOM FACTS BASE
  /*

  qDebug() << "Start";

  // Group objects to ensure attributes data is gathered
  setGroupingSettings();
  settings->stopCondition = settings->objectsNumber;
  groupObjects();

  qDebug() << "Objects grouped";

  ruleInterferencer.setGroupingThread(this->gThread);

  // Now I want to create a file that will store this clusters
  // rule as a fact.
  QString fileName = ui->labelObjectBaseName->text();
  fileName.chop(8);
  fileName.remove(0, 3);
  fileName += ".fct";
  //fileName += "_f.fct";

  QString path = "D:\\Dysk Google\\Rules\\";
  path += fileName;

  ruleInterferencer.generateRandomFactsBase(path, 100000);
  */

  qDebug() << "Interfering";

  QString fileName = ui->labelObjectBaseName->text();
  fileName.chop(8);
  fileName.remove(0, 3);
  fileName += ".csv";

  QString path = "D:\\Dysk Google\\Rules\\" + fileName;

  QFile file(path);

  if(!file.exists())
  {
    if(file.open(QIODevice::ReadWrite  | QIODevice::Append))
    {
      QTextStream stream(&file);

      stream << "First rule that could be fired,"
             << "Last rule that could be fired,"
             << "Number of facts,"
             << "Number of rules that could be fired,"
             << "Percent of rules that could be fired,"
             << "Was target set,"
             << "Is target achiveable,"
             << "Number of rules fired,"
             << "Most similiar rule,"
             << "Was target achieved,"

             << "Zero representative occurence,"
             << "Zero representatives number,"
             << "Was zero representative found during interference,"
             << "Zero representatives checked number,"

             << "Number of new facts,"
             << "Was rule fired,"
             << "Number of clusters searched,"
             << "Number of clusters,"
             << "Clustering Similarity Method,"
             << "Object Similarity Method,"
             << "Representative Generation Method,"
             << "Rep. Threshold,"
             << "Rules that could be fired,"
             << "Smallest cluster size,"

             << "Biggest cluster size,"
             << "Number of outliers,"
             << "Smallest representative length,"
             << "Average representative length,"
             << "Biggest representative length,"
             << "Number of attributes,"
             << "Number of rules,"
             << "Gini Index,"
             << "Bonferroni Index\n";
    }

    file.close();
  }

  gSettings->interClusterSimMeasureID = CentroidLinkId;

  ruleInterferencer.setGroupingThread(this->gThread);

  QList<int> factsPercents = {100, 75, 50, 25, 10, 1};

  for(int factPercent : factsPercents)
  {
    ruleInterferencer.factsBasePercents = {factPercent};

    ruleInterferencer.interfere();

    //qDebug() << ruleInterferencer.availableRuleIndexes;

    if(file.open(QIODevice::ReadWrite  | QIODevice::Append))
    {
      QTextStream stream(&file);

      stream  << ruleInterferencer.availableRuleIndexes.first() << ","
              << ruleInterferencer.availableRuleIndexes.last() << ","
              << ruleInterferencer.getNumberOfFacts() << ","
              << ruleInterferencer.getNumberOfRulesThatCanBeFired() << ","
              << qreal(100 * ruleInterferencer.getNumberOfRulesThatCanBeFired()
                      / settings->objectsNumber ) << ","
              << ruleInterferencer.target.size() << ","
              << ruleInterferencer.targetAchiveable <<","
              << ruleInterferencer.numberOfRulesFired << ","
              << ruleInterferencer.mostSimilarRule->name() << ","
              << ruleInterferencer.targetAchieved << ","

              << gSettings->zeroRepresentativeClusterOccurence << ","
              << gSettings->zeroRepresentativesNumber << ","
              << ruleInterferencer.zeroRepresentativeOccured << ","
              << ruleInterferencer.zeroRepresentativeOccurenceNumber << ","

              << ruleInterferencer.getNumberOfNewFacts() << ","
              << ruleInterferencer.wasRuleFired() << ","
              << ruleInterferencer.numberOfClustersSearched << ","
              << settings->stopCondition << ","
              << ui->comboBoxInterClusterSimMeasure->currentText() << ","
              << ui->comboBoxInterObjectSimMeasure->currentText() << ","
              << ui->comboBoxRepresentativeCreationStrategy->currentText() << ","
              << ui->spinBoxRepresentativeAttributePercent->text() << ","
              << ruleInterferencer.availableRuleIndexes.join(" & ") << ","
              << findSmallestCluster()->size() << ","

              << findBiggestCluster()->size() << ","
              << countUngroupedObjects() << ","
              << getSmallestRepresentativeLength() << ","
              << getAverageRepresentativeLength() << ","
              << getBiggestRepresentativeLength() << ","
              << settings->objectsNumber << ","
              << gSettings->attributesNumber << ","
              << gSettings->giniIndex << ","
              << gSettings->bonferroniIndex << "\n";
    }

    file.close();
  }

  qDebug() << "End interfering.";

}

void MainWindow::on_actionLoadFactsBase_triggered()
{
  QFileDialog FD;

  QString openPath = getOpenPath();

  QString FBPath = FD.getOpenFileName(
      this,
      tr("FD.selectKnowledgeBase"),
      openPath,
      "*.fct"
  );

  ruleInterferencer.loadFactsFromPath(FBPath);

}
