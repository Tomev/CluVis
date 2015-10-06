#include "mainwindow.h"
#include "about.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QtCore>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QtGui>
#include <QDesktopWidget>
#include <QTime>

#include "customgraphicsrectobject.h"
#include "customgraphicellipseobject.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setSceneRect(0,0,0,0);

    ui->labelIsBaseGrouped->setText("");

    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Rozpoczęto działanie programu CluVis.";

    ui->textBrowserLog->setText(logText);

    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->actionSaveVisualization->setEnabled(false);
    ui->actionGenerateReport->setEnabled(false);

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint |
                   Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
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
    {
        delete clusteredRules[i];
    }

    for(int i = settings->objectsNumber; i >= 0; i--)
    {
        delete &joinedRules[i];
    }

    for(int i = settings->objectsNumber; i >= 0; i--)
    {
        delete &rules[i];
    }

    delete[] rules;
    delete[] joinedRules;
    delete[] clusteredRules;
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
        QString msgBoxText = "Nie wybrano nowej bazy wiedzy.";

        QString logText = "[" +tim->currentTime().toString() + "] ";
        logText+= "Nieudana próba wczytania bazy wiedzy. ";
        logText+= "Nie wybrano pliku.";

        QMessageBox::information(this,
                                "Nie wybrano pliku",
                                msgBoxText,
                                QMessageBox::Ok);

        ui->textBrowserLog->append(logText);

        return;
    }

    if(isRuleSet(KB) == false)
    {
        QString msgBoxText = "Wybrany plik nie jest bazą wiedzy. \n";
        msgBoxText += "Proszę wybrać inny plik.";

        QString logText = "[" +tim->currentTime().toString() + "] ";
        logText+= "Nieudana próba wczytania bazy wiedzy. ";
        logText+= "Wybrany plik nie jest bazą wiedzy.";

        QMessageBox::information(this,
                                "Wybrano nieprawidłowy plik",
                                msgBoxText,
                                QMessageBox::Ok);

        ui->textBrowserLog->append(logText);

        return;
    }

    scene->clear();
    ui->actionSaveVisualization->setEnabled(false);
    ui->actionGenerateReport->setEnabled(false);
    ui->labelIsBaseGrouped->setText("<b>(Niepogrupowana)</b>");
    areObjectsClustered = false;

    gSettings->objectBaseInfo = KB;

    OBName += "."+gSettings->objectBaseInfo.suffix();
    settings->objectsNumber = getObjectsNumber();

    ui->labelObjectBaseName->setText("<b>"+OBName+"</b>");
    ui->labelObjectsNumberValue->setText("<b>"+QString::number(settings->objectsNumber)+".</b>");
    ui->spinBoxStopConditionValue->setMaximum(settings->objectsNumber);
    ui->spinBoxStopConditionValue->setValue(1);

    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wczytano bazę wiedzy " + OBName + ".";

    ui->textBrowserLog->append(logText);
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

    QString OBPath = FD.getOpenFileName(this, tr("Wybierz bazę"),openPath,tr("Pliki tekstowe (*.txt);; Pliki reguł(*.rul)"));

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
        case settings->RSES_RULES_ID:
            return gSettings_RSES->getRSESRulesNumber(
                        gSettings->objectBaseInfo);
            break;
        default:
            ;
    }

    return 0;
}

void MainWindow::on_actionSaveVisualization_triggered()
{
    QFileDialog FD;

    QString fileName = FD.getSaveFileName(this,"Zapisz","C:\\","*.png");

    if(fileName == "")
    {
        QString msgBoxText = "Nie wybrano nazwy pliku.\n";
        msgBoxText += "Obraz nie został zapisany.";

        QString logText = "[" +tim->currentTime().toString() + "] ";
        logText+= "Nieudana próba zapisu obrazu. ";
        logText+= "Nie wybrano nazwy pliku.";

        QMessageBox::information(this,
                                "Nie wybrano pliku",
                                msgBoxText,
                                QMessageBox::Ok);

        ui->textBrowserLog->append(logText);

        return;
    }

    fileName += ".png";

    QRect sceneRect(ui->graphicsView->frameGeometry().left() + 15,
                    ui->graphicsView->frameGeometry().top() + 55,
                    ui->graphicsView->frameGeometry().width()-3,
                    ui->graphicsView->height()-3);


    QPixmap pixMap = QWidget::grab(sceneRect);
    pixMap.save(fileName);

    QString logText = "[" +tim->currentTime().toString() + "] ";
    logText+= "Wizualizację zapisano pod nazwą " + fileName +".";

    ui->textBrowserLog->append(logText);
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
        gotLogText("Przerwano operację.");

        return ;
    }

    int fileType = getFileType(filePath.at(filePath.length()-1));

    QString reportContent = createReportContent(fileType);

    qDebug() << filePath;

    QFile report(filePath);

    gotLogText("Rozpoczęto generowanie raportu.");

    if(!report.open(QFile::WriteOnly | QFile::Text) && report.exists())
    {
        gotLogText("Nie można otworzyć pliku do zapisu. Przerwano operację.");

        return ;
    }

    QTextStream outStream(&report);
    outStream.setCodec("UTF-8");
    outStream << reportContent;

    report.flush();
    report.close();

    gotLogText("Zakończono generowanie raportu.");
}

QString MainWindow::getFilePath()
{
    QFileDialog FD;

    QString filePath = FD.getSaveFileName(this,"Zapisz raport","C:\\",
                                          "Pliki xml (*.xml);; Pliki tekstowe (*.txt)");

    if(filePath == "")
    {
        QString msgBoxText = "Nie wybrano nazwy pliku.\n";
        msgBoxText += "Raport nie został zapisany.";

        QString logText = "Nieudana próba zapisu raportu. ";
        logText+= "Nie wybrano nazwy pliku.";

        gotLogText(logText);

        QMessageBox::information(this,
                                "Nie wybrano pliku",
                                msgBoxText,
                                QMessageBox::Ok);
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

QString MainWindow::createTXTReportContent()
{
    QString content = "===Raport===";

    content += "\n==Dane bazy==\n";
    content += "\nNazwa bazy: " + formatThickString(ui->labelObjectBaseName->text());
    content += "\nLiczba atrybutów: " + QString::number(gSettings->attributesNumber);
    content += "\nLiczba obiektów: " + QString::number(getObjectsNumber());
    content += "\nLiczba skupień: " + QString::number(settings->stopCondition);
    content += "\nSuma pokryciowa: " + QString::number(countCoverageSum());
    content += "\n\n==Ustawienia==\n";
    content += "\nWykorzystany algorytm grupowania: " + formatThickString(ui->labelAlgorithmGroupingValue->text());
    content += "\nWybrana miara podobieństwa obiektów: " + ui->comboBoxInterobjectDistanceMeasure->currentText();
    content += "\nWybrana miara podobieństwa skupień: " + ui->comboBoxInterclusterDistanceMeasure->currentText();
    content += "\nWybrany algorytm wizualizacji: " + ui->comboBoxAlgorithmVisualization->currentText();
    content += "\nGrupowana część reguły: " + ui->comboBoxRuleGroupedPart->currentText();
    content += "\n\n==Dane skupień==\n";
    content += "\nNajliczniejsza grupa: " + getRuleClusterName(findBiggestCluster());
    content += "\nNajmniej liczna grupa: " + getRuleClusterName(findSmallestCluster());
    content += "\n\n==Szczegóły skupień==\n";

    for(int i = 0; i < settings->stopCondition; i++)
    {
        content += "\n=Nazwa skupienia: " + getRuleClusterName(vSettings_RSES->clusteredRules[i]) + "=";
        content += "\n\tLiczba reguł w grupie: " + QString::number(vSettings_RSES->clusteredRules[i]->size());
        content += "\n\tPokrycie skupienia: "
                + QString::number((float)((countClusterCoverage(vSettings_RSES->clusteredRules[i])*100)/countCoverageSum())) + "%";
        content += "\n\tReprezentant skupienia: " + vSettings_RSES->clusteredRules[i]->representative;
    }

    content += "\n\n===Koniec===";

    return content;
}

QString MainWindow::formatThickString(QString s)
{
    s.resize(s.length()-4);
    s.remove(0,3);

    return s;
}

int MainWindow::countCoverageSum()
{
    int coverageSum = 0;

    for(int i = 0; i < getObjectsNumber(); i++)
        coverageSum += rules[i].support;

    return coverageSum;
}

ruleCluster* MainWindow::findSmallestCluster()
{
    ruleCluster* smallest = clusteredRules[0];

    for(int i = 1; i < settings->stopCondition; i++)
    {
        if(smallest->size() > clusteredRules[i]->size())
            smallest = clusteredRules[i];
    }

    return smallest;
}

ruleCluster* MainWindow::findBiggestCluster()
{
    ruleCluster* biggest = clusteredRules[0];

    for(int i = 1; i < this->settings->stopCondition; i++)
    {
        if(biggest->size() < clusteredRules[i]->size())
            biggest = clusteredRules[i];
    }

    return biggest;
}

QString MainWindow::getRuleClusterName(ruleCluster* c)
{
    QString name;

    if(c->rule == "")
        name = "J";
    else
        name = "R";

    name += QString::number(c->clusterID + 1);

    return name;
}

int MainWindow::countClusterCoverage(ruleCluster* c)
{
    if(c==NULL)
        return 0;

    if(c->rule != "")
        return c->support;

    return countClusterCoverage(c->leftNode) + countClusterCoverage(c->rightNode);
}

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
    QString result = "\t<Worksheet ss:Name=\"Raport\">\n";
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
    QString result = createXMLFileTableCell("Index", true);
    result += createXMLFileTableCell("Nazwa bazy", true);
    result += createXMLFileTableCell("Liczba atrybutów", true);
    result += createXMLFileTableCell("Liczba obiektów", true);
    result += createXMLFileTableCell("Liczba skupień", true);
    result += createXMLFileTableCell("Suma pokryciowa", true);
    result += createXMLFileTableCell("Wykorzystany algorytm grupowania", true);
    result += createXMLFileTableCell("Wybrana miara podobieństwa obiektów", true);
    result += createXMLFileTableCell("Wybrana miara podobieństwa skupień", true);
    result += createXMLFileTableCell("Wybrany algorytm wizualizacji", true);
    result += createXMLFileTableCell("Grupowana część reguły", true);
    result += createXMLFileTableCell("Najliczniejsza grupa", true);
    result += createXMLFileTableCell("Najmniej liczna grupa", true);

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
    QString result = createXMLFileTableCell("1", true);
    result += createXMLFileTableCell(formatThickString(ui->labelObjectBaseName->text()), false);
    result += createXMLFileTableCell(QString::number(gSettings->attributesNumber), false);
    result += createXMLFileTableCell(QString::number(getObjectsNumber()), false);
    result += createXMLFileTableCell(QString::number(settings->stopCondition), false);
    result += createXMLFileTableCell(QString::number(countCoverageSum()), false);
    result += createXMLFileTableCell(formatThickString(ui->labelAlgorithmGroupingValue->text()), false);
    result += createXMLFileTableCell(ui->comboBoxInterobjectDistanceMeasure->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxInterclusterDistanceMeasure->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxAlgorithmVisualization->currentText(), false);
    result += createXMLFileTableCell(ui->comboBoxRuleGroupedPart->currentText(), false);
    result += createXMLFileTableCell(getRuleClusterName(findBiggestCluster()), false);
    result += createXMLFileTableCell(getRuleClusterName(findSmallestCluster()), false);

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
    result += createXMLFileTableCell("Index", true);
    result += createXMLFileTableCell("Nazwa skupienia", true);
    result += createXMLFileTableCell("Liczba reguł w grupie", true);
    result += createXMLFileTableCell("Pokrycie skupienia", true);
    result += createXMLFileTableCell("Reprezentant skupienia", true);
    result += "\t\t\t</Row>\n";

    return result;
}

QString MainWindow::createXMLFileClustersDataContent()
{
    QString result = "";

    for(int i = 0; i < settings->stopCondition; i++)
    {
        result += "\t\t\t<Row>\n";
        result += createXMLFileTableCell(QString::number(i), false);
        result += createXMLFileTableCell(getRuleClusterName(vSettings_RSES->clusteredRules[i]), false);
        result += createXMLFileTableCell(QString::number(vSettings_RSES->clusteredRules[i]->size()), false);
        result += createXMLFileTableCell(
                    QString::number((float)((countClusterCoverage(vSettings_RSES->clusteredRules[i])*100)/countCoverageSum())) + "%", false);
     //   result += createXMLFileTableCell(vSettings_RSES->clusteredRules[i]->representative, false);
        result += "\t\t\t</Row>\n";
    }

    return result;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

//Help

void MainWindow::on_actionAbout_triggered()
{
    About a;
    a.exec();

    gotLogText("Włączono informacje o programie.");
}

//Rest

void MainWindow::on_pushButtonGroup_clicked()
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wczytytuję ustawienia grupowania...";

    ui->textBrowserLog->append(logText);

    setGroupingSettings();

    logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Sprawdzam poprawność ustawień...";

    ui->textBrowserLog->append(logText);

    if(areSettingsCorrect())
    {
        logText = "[" + tim->currentTime().toString() + "] ";
        logText += "Ustawienia poprawne.";

        ui->textBrowserLog->append(logText);

        groupObjects();
    }
}

void MainWindow::on_pushButtonVisualize_clicked()
{    
    if(areObjectsClustered)
    {
        QString logText = "[" + tim->currentTime().toString() + "] ";
        logText += "Wczytytuję ustawienia wizualizacji...";

        ui->textBrowserLog->append(logText);

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

        logText = "[" + tim->currentTime().toString() + "] ";
        logText += "Ustawienia wizualizacji wczytane.";

        ui->textBrowserLog->append(logText);

        visualize();
    }
    else
    {
        QString msgBoxText = "Nie pogrupowano obiektów.\n";
        msgBoxText += "Proszę najpierw pogrupować obiekty.";

        QString logText = "[" +tim->currentTime().toString() + "] ";
        logText+= "Nieudana próba generowania wizualizacji. ";
        logText+= "Nie pogrupowano obiektów.";

        QMessageBox::information(this,
                                "Nie pogrupowano obiektów",
                                msgBoxText,
                                QMessageBox::Ok);

        ui->textBrowserLog->append(logText);
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

    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wczytano ustawienia ogólne.";

    ui->textBrowserLog->append(logText);

    switch(settings->dataTypeID)
    {
    case settings->RSES_RULES_ID:

        gSettings_RSES->groupingPartID =
                ui->comboBoxRuleGroupedPart->currentIndex();

        gThread = new groupingThread(gSettings_RSES, gSettings, settings);

        connect(gThread,SIGNAL(passRules(ruleCluster*)),
                this,SLOT(getRules(ruleCluster*)));
        connect(gThread,SIGNAL(passJoinedRules(ruleCluster*)),
                this,SLOT(getJoinedRules(ruleCluster*)));
        connect(gThread,SIGNAL(passClusteredRules(ruleCluster**)),
                this,SLOT(getClusteredRules(ruleCluster**)));
        connect(gThread,SIGNAL(passLogMsg(QString)),
                this,SLOT(gotLogText(QString)));
        connect(gThread,SIGNAL(passMDI(qreal)),
                this,SLOT(gotMDI(qreal)));
        connect(gThread,SIGNAL(passMDBI(qreal)),
                this,SLOT(gotMDBI(qreal)));


        logText = "[" + tim->currentTime().toString() + "] ";
        logText += "Wczytano ustawienia szczególne dla RSES Rules.";

        ui->textBrowserLog->append(logText);

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

    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wczytano ustawienia ogólne.";

    ui->textBrowserLog->append(logText);

    switch(settings->dataTypeID)
    {
    case settings->RSES_RULES_ID:

        vSettings_RSES->clusteredRules = clusteredRules;

        vThread = new visualizationThread(settings, vSettings, vSettings_RSES);

        logText = "[" + tim->currentTime().toString() + "] ";
        logText += "Wczytano ustawienia szczególne.";

        ui->textBrowserLog->append(logText);

        break;

    default:
        ;
    }
}

//Checker

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
        QString msgBoxText = "Nie wybrano bazy.\n";
        msgBoxText += "Proszę wczytać bazę.";

        QString logText = "[" +tim->currentTime().toString() + "] ";
        logText+= "Nieudana próba generowania skupień. ";
        logText+= "Nie wybrano bazy.";

        QMessageBox::information(this,
                                "Nie wczytano bazy",
                                msgBoxText,
                                QMessageBox::Ok);

        ui->textBrowserLog->append(logText);

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
            QString msgBoxText = "Niepoprawny warunek stopu.\n";
            msgBoxText += "Proszę ustawić poprawny warunek stopu.";

            QString logText = "[" +tim->currentTime().toString() + "] ";
            logText+= "Nieudana próba generowania skupień. ";
            logText+= "Warunek stopu nie może być większy od liczby obiektów w bazie.";

            QMessageBox::information(this,
                                    "Niepoprawny warunek stopu",
                                    msgBoxText,
                                    QMessageBox::Ok);

            ui->textBrowserLog->append(logText);

            return false;
        }
    }
    else
    {
        QString msgBoxText = "Niepoprawny warunek stopu.\n";
        msgBoxText += "Proszę ustawić poprawny warunek stopu.";

        QString logText = "[" +tim->currentTime().toString() + "] ";
        logText+= "Nieudana próba generowania skupień. ";
        logText+= "Warunek stopu nie może być mniejszy od jedynki.";

        QMessageBox::information(this,
                                "Niepoprawny warunek stopu",
                                msgBoxText,
                                QMessageBox::Ok);

        ui->textBrowserLog->append(logText);

        return false;
    }
}

//End GUI

//Functionality

void MainWindow::groupObjects()
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Rozpoczynam grupowanie...";

    ui->textBrowserLog->append(logText);

    logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Blokuję przyciski na czas grupowania...";

    ui->textBrowserLog->append(logText);

    ui->pushButtonVisualize->setEnabled(false);
    ui->pushButtonGroup->setEnabled(false);

    areObjectsClustered = false;

    logText = "[" +tim->currentTime().toString() + "] ";
    logText+= "Rozpoczynam wątek grupujący...";

    ui->textBrowserLog->append(logText);

    tim->start();
    gThread->run();
    int timeElapsed = tim->elapsed();

    logText = "[" +tim->currentTime().toString() + "] ";
    logText += "Wątek grupujący zakończony w czasie ";
    logText += QString::number(timeElapsed);
    logText += " ms.";

    ui->textBrowserLog->append(logText);

    ui->pushButtonVisualize->setEnabled(true);
    ui->pushButtonGroup->setEnabled(true);

    logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Przyciski odblokowane.";

    ui->textBrowserLog->append(logText);

    if(areObjectsClustered)
    {
        QMessageBox mb;
        QString infoText;

        infoText = "Liczba otrzymanych skupień: " + QString::number(settings->stopCondition) + ".\n";
        infoText += "MDI otrzymanych skupień: " + QString::number(MDI) + ".\n";
        infoText += "MDBI otrzymanych skupień: " + QString::number(MDBI) + ".\n";
        infoText += "Czy chcesz wygenerować wizualizację struktury grup?";

        mb.setWindowTitle("Grupowanie zakończone");
        mb.setText("Grupowanie zakończone.");
        mb.setInformativeText(infoText);
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        mb.setButtonText(QMessageBox::Yes, "Tak");
        mb.setButtonText(QMessageBox::No, "Nie");
        mb.setDefaultButton(QMessageBox::Yes);

        if(mb.exec() == QMessageBox::Yes)
            on_pushButtonVisualize_clicked();
    }
}

void MainWindow::visualize()
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Rozpoczynam wizualizowanie...";

    ui->textBrowserLog->append(logText);

    logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Blokuję przyciski na czas wizualizowania.";

    ui->pushButtonVisualize->setEnabled(false);
    ui->pushButtonGroup->setEnabled(false);

    logText = "[" +tim->currentTime().toString() + "] ";
    logText+= "Rozpoczynam wątek wizualizujący...";

    ui->textBrowserLog->append(logText);

    tim->start();
    vThread->run();
    int timeElapsed = tim->elapsed();

    logText = "[" +tim->currentTime().toString() + "] ";
    logText += "Wątek wizualizujący zakończony w czasie ";
    logText += QString::number(timeElapsed);
    logText += " ms.";

    ui->textBrowserLog->append(logText);

    if(vSettings->visualizationAlgorithmID == vSettings->RT_SLICE_AND_DICE_ID)
        ui->graphicsView->fitInView(scene->itemsBoundingRect());
    if(vSettings->visualizationAlgorithmID == vSettings->CIRCULAR_TREEMAP_ID)
        ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wizualizacja wycentrowana.";

    ui->textBrowserLog->append(logText);

    ui->pushButtonVisualize->setEnabled(true);
    ui->pushButtonGroup->setEnabled(true);

    logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Przyciski odblokowane.";

    ui->textBrowserLog->append(logText);

    QString msgBoxText = "Wizualizowanie zakończone!";

    QMessageBox::information(this,
                            "Proces zakończony",
                            msgBoxText,
                            QMessageBox::Ok);

    ui->actionSaveVisualization->setEnabled(true);
    ui->actionGenerateReport->setEnabled(true);
}

/* GENERAL I DETAILS SETTINGS
void MainWindow::on_pushButtonDetails_clicked()
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wyświetlam ustawienia szczególne.";

    ui->textBrowserLog->append(logText);

    ui->stackedWidgetSettings->setCurrentIndex(1);
}

void MainWindow::on_pushButtonGeneral_clicked()
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wyświetlam ustawienia ogólne.";

    ui->textBrowserLog->append(logText);

    ui->stackedWidgetSettings->setCurrentIndex(0);
}
*/

void MainWindow::getRules(ruleCluster *c)
{   
    rules = c;
}

void MainWindow::getJoinedRules(ruleCluster *c)
{
    joinedRules = c;
}

void MainWindow::getClusteredRules(ruleCluster **c)
{
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Otrzymano pogrupowane reguły. Można przystąpić do wizualizacji.";

    ui->textBrowserLog->append(logText);

    clusteredRules = c;

    areObjectsClustered = true;
    ui->labelIsBaseGrouped->setText("<b>(Pogrupowana)</b>");
}

void MainWindow::gotMDI(qreal MDI)
{
    this->MDI = MDI;
}

void MainWindow::gotMDBI(qreal MDBI)
{
    this->MDBI = MDBI;
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
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wybrano skupienie do wizualizacji. Wizualizuję...";

    ui->textBrowserLog->append(logText);

    ruleCluster newC = *c;

    scene->clear();

    vThread->run(&newC);
}

void MainWindow::showRuleInfo(ruleCluster *c)
{
    gotLogText("Wyświetlam informacje dotyczące wybranego skupienia.");

    cInfo = new clusterInfo_RSESRules(c);

    connect(cInfo,SIGNAL(passClusterToVisualize(ruleCluster*)),
            this,SLOT(gotRuleClusterToVisualize(ruleCluster*)));

    cInfo->exec();
}

void MainWindow::gotLogText(QString msg)
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
