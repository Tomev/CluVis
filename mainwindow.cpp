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
    delete rules;
    delete joinedRules;
    delete clusteredRules;
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

    ui->labelObjectBaseName->setText("<b>"+OBName+".</b>");
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

    QString OBPath = FD.getOpenFileName(this, tr("Wybierz bazę"),openPath,tr("Files(*.txt *.rul)"));

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
    QString logText;
    QString filePath = getFilePath();
    QString reportContent = createReportContent();

    QFile report(filePath);
    QDir reportDir(getReportDirPath());

    if (!reportDir.exists())
        createDir(getReportDirPath());

    if(!report.open(QFile::WriteOnly | QFile::Text) && report.exists())
    {
        logText = "[" + tim->currentTime().toString() + "] ";
        logText += "Nie można otworzyć pliku do zapisu. Przerwano operację.";

        ui->textBrowserLog->append(logText);

        return ;
    }

    QTextStream outStream(&report);
    outStream << reportContent;

    report.flush();
    report.close();
}

QString MainWindow::getFilePath()
{
    QString path = "C:\\CluVis_Reports\\test.txt";

    return path;
}

QString MainWindow::getReportDirPath()
{
    QString path = "C:\\CluVis_Reports";

    return path;
}

void MainWindow::createDir(QString path)
{
    QDir().mkdir(path);
}

QString MainWindow::createReportContent()
{
    QString content = "";

    content += "Test Content";

    return content;
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

    QString logText = "[" +tim->currentTime().toString() + "] ";
    logText+= "Włączono informacje o programie.";

    ui->textBrowserLog->append(logText);
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

    //ui->graphicsView->centerOn(0, 0);

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
    QString logText = "[" + tim->currentTime().toString() + "] ";
    logText += "Wyświetlam informacje dotyczące wybranego skupienia.";

    ui->textBrowserLog->append(logText);

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
