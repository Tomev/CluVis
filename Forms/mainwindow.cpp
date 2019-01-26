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

static std::vector<QString> clusteringSimilarityMeasuresShotrcuts
  = {"SL", "CL", "CoL", "AL", "B", "G"};

static std::vector<QString> representativeGenerationMethodShortcuts
  = {"Threshold", "Lower", "Upper", "Weighted"};

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

    //addLogMessage(tr("log.applicationStart"));
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

long MainWindow::getClusteringTreeLevel()
{
  /*
  long  clusteringTreeLevel = 0,
        currentClusterTreeLevel = 0,
        leftNodeTreeSize = 0,
        rightNodeTreeSize = 0;

  for(std::shared_ptr<cluster> c : clusters)
  {
    if(!c->hasBothNodes())
    {
      currentClusterTreeLevel = 0;
    }
    else
    {
      leftNodeTreeSize = getClustersTreeLevel(c->leftNode, clusteringTreeLevel);
      rightNodeTreeSize = getClustersTreeLevel(c->rightNode, clusteringTreeLevel);

      currentClusterTreeLevel = rightNodeTreeSize > leftNodeTreeSize ?
          rightNodeTreeSize : leftNodeTreeSize;
    }

    clusteringTreeLevel = clusteringTreeLevel > currentClusterTreeLevel ?
        clusteringTreeLevel : currentClusterTreeLevel;
  }
  */
  long clusteringTreeLevel = 0;
  long clusterTreeLevel = 0;

  for(auto c : clusters)
  {
    clusterTreeLevel = getClustersTreeLevel(c);

    if(clusterTreeLevel > clusteringTreeLevel)
      clusteringTreeLevel = clusterTreeLevel;
  }


  return clusteringTreeLevel;
}

long MainWindow::getClustersTreeLevel(std::shared_ptr<cluster> c)
{
  // If it's an object it's level is 1
  if(!c->hasBothNodes())
    return 0;

  long leftClusterTreeLevel = getClustersTreeLevel(c->leftNode);
  long rightClusterTreeLevel = getClustersTreeLevel(c->rightNode);

  return qMax(leftClusterTreeLevel, rightClusterTreeLevel) + 1;
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
    int smallestRepSize = INT_MAX;
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

    /*QString logText = QString(tr("log.groupingThreadFinishedIn"))
            .arg(QString::number(timeElapsed));*/

    //addLogMessage(logText);

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

std::vector<rule> MainWindow::getRulesFromRulesClusters()
{
  std::vector<rule> rules;

  QList<cluster*> ruleClusters;

  for(int i = 0; i < settings->stopCondition; ++i)
    ruleClusters.append(gThread->clusters[i]->getObjects());

  for(cluster* c : ruleClusters)
  {
    ruleCluster *rCluster = static_cast<ruleCluster*>(c);
    rules.push_back(rule());

    for(QString permiseAttribute : rCluster->premiseAttributes)
    {
      for(QString attributesValue : (*rCluster->attributesValues[permiseAttribute]))
        rules.back().premises[permiseAttribute.toStdString()].insert(attributesValue.toStdString());
    }

    for(QString decisionAttribute : rCluster->decisionAttributes)
    {
      for(QString attributesValue : (*rCluster->attributesValues[decisionAttribute]))
        rules.back().conclusions[decisionAttribute.toStdString()].insert(attributesValue.toStdString());
    }
  }

  return rules;
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
    clock_t start = clock();

    // TODO: Change for editable dir.
    // TODO: D:/ANB/ must exists for this to work. Eliminate this problem.
    QString baseDir = "C:/Rules/",
            targetDir,
            kbsDirPath;

    QStringList kbNames;

    // Get dir path of KBs folder
    kbsDirPath = QFileDialog::getExistingDirectory
                (
                    this,
                    tr("Select directory"),
                    "C:/Rules/",
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

        QString factBasePath = targetDir + ".fct";

        ruleInterferencer.loadFactsFromPath(factBasePath);
        classicInterferencer.loadFactsFromPath(factBasePath.toStdString());

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
            numberOfIterations, start = 0;

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
        for(int osm = SingleLinkId; osm < ui->comboBoxInterObjectSimMeasure->count(); ++osm)
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
            for(int csm = SingleLinkId; csm < ui->comboBoxInterClusterSimMeasure->count(); ++csm)
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
                    thresholds = {0};

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

                    while(desiredClustersNumber > 0)
                    {
                      qDebug() << "Continuing grouping to "
                               << desiredClustersNumber << " clusters.";

                      // Perform grouping for given clusters number with default settings.
                      ui->spinBoxStopConditionValue->setValue(desiredClustersNumber);

                      setGroupingSettings();

                      gThread->continueGrouping();

                      // Generate report of this grouping in dir.
                      //QString reportName = ui->comboBoxInterClusterSimMeasure->currentText() + " " + QString::number(desiredClustersNumber);
                      //generateReport(targetDir + "/" + reportName + ".xml");

                      qDebug() << "Grouped.";

                      on_pushButtonInterfere_clicked();

                      desiredClustersNumber -= rulesNumberOnePercent;
                    }

                    //qDebug() << "Next measure.";
                  }
                }
            }
        }
    }

    qDebug() << "A splendid finish.";
    long raportGenerationTime
        = (clock() - start) / (double) CLOCKS_PER_SEC;

    qDebug() << "Raport generation time: " << raportGenerationTime << " s.";
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

  //qDebug() << "Interfering";


  QString fileName = ui->labelObjectBaseName->text();
  fileName.chop(8);
  fileName.remove(0, 3);
  QString baseName = fileName;
  fileName += ".csv";

  // Multiple reports
  //QString path = "D:\\Dysk Google\\Rules\\" + fileName;

  // Signle report
  QString path = "C:\\Rules\\CluvisInterferenceReport.csv";


  QFile file(path);

  QList<int> factsPercents = {100, 75, 50, 25, 10, 1};
  QHash<int, int> numberOfRulesThatCouldBeFiredFromGivenFacts;

  // Create file if it doesn't exist. Add classical interference data.
  if(!file.exists())
  {
    if(file.open(QIODevice::ReadWrite  | QIODevice::Append))
    {
      QTextStream stream(&file);

      stream // Base
             << "Knowledge base name,"
             << "Number of attributes,"
             << "Number of rules,"
             // Clustering
             << "Clustering similarity measure,"
             << "Object similarity measure,"
             << "Representative generation method,"
             << "Representative threshold,"
             << "Grouping time [s],"
             << "Smallest cluster size,"
             << "Biggest cluster size,"
             << "Number of outliers,"
             << "Smallest representative length,"
             << "Average representative length,"
             << "Biggest representative length,"
             << "Zero representatives number,"
             << "Gini index,"
             << "Bonferroni index,"
             << "Number of clusters,"
             << "Number of levels in the tree,"
             // Interference
             << "Interference type,"
             << "Facts base percent,"
             << "Number of initial facts,"
             << "Number of rules that could initially be fired,"
             << "Was interference target set,"
             << "Was interference target initially confirmable,"
             << "Number of interference iterations,"
             << "Was any rule fired,"
             << "Number of rules that were fired,"
             << "Number of new facts,"
             << "Was interference target confirmed,"
             << "Was new knowledge explored,"
             << "Was zero representative met,"
             << "Zero representative occurence step,"
             << "Number of zero representatives found during interference,"
             << "Number of clusters searched,"
             << "Number of rules that could be fired,"
             << "Why wasn't interference target confirmed,"
             << "Interference time [s]\n";
    }

    file.close();

    for(int factPercent : factsPercents)
    {
      // For classical interferencer

      //qDebug() << "Setting rules to classic interferencer.";

      classicInterferencer.setRules(getRulesFromRulesClusters());

      //qDebug() << "Setting facts base percents to classic interfetencer.";

      classicInterferencer.setFactsBasePercent(factPercent);

      //qDebug() << "Interfering with classic interferencer.";

      classicInterferencer.interfere();

      //qDebug() << "Saving interference data from classic interferencer to report.";

      numberOfRulesThatCouldBeFiredFromGivenFacts[factPercent] =
          classicInterferencer.getNumberOfRulesFired();

      if(file.open(QIODevice::ReadWrite  | QIODevice::Append))
      {
        QTextStream stream(&file);

        stream // Base
               << baseName << "," // Knowledge base name
               << gSettings->attributesNumber << "," // Number of attributes
               << settings->objectsNumber << "," // Number of rules
               // Clustering
               << "Not applicable" << "," // Clustering similarity measure
               << "Not applicable" << "," // Object similarity measure
               << "Not applicable" << "," // Representative generation method
               << "Not applicable" << "," // Representative threshold
               << "Not applicable" << "," // Grouping time [s]
               << "Not applicable" << "," // Smallest cluster size
               << "Not applicable" << "," // Biggest cluster size
               << "Not applicable" << "," // Number of outliers
               << "Not applicable" << "," // Smallest representative len.
               << "Not applicable" << "," // Average rep. len.
               << "Not applicable" << "," // Biggest rep. len.
               << "Not applicable" << "," // Zero rep. num.
               << "Not applicable" << "," // Gini
               << "Not applicable" << "," // Bonferroni
               << "Not applicable" << "," // Number of clusters
               << "Not applicable" << "," // Number of levels in the tree
               // Interference
               << QString::fromStdString(
                    classicInterferencer.getInterferentionType()
                  ) << "," // Interference type
               << factPercent << "," // Facts base percent
               << classicInterferencer
                  .getInitialNumberOfFacts() << "," // Number of initial facts
               << classicInterferencer
                    .getNumberOfRulesThatCouldInitiallyBeFired() << "," // Number of rules that could initially be fired
               << classicInterferencer
                    .wasTargetSet() << "," // Was interference target set
               << classicInterferencer
                    .wasInterferenceTargetInitiallyConfirmable() << "," // Was interference target initially confirmable
               << classicInterferencer
                    .getNumberOfIterations() << "," // Number of interference iterations
               << classicInterferencer
                    .wasAnyRuleFired()<< "," // Was any rule fired
               << classicInterferencer
                    .getNumberOfRulesFired() << "," // Number of rules that were fired
               << classicInterferencer
                    .getNumberOfNewFacts() << "," // Number of new facts
               << classicInterferencer
                    .wasTargetAchieved(nullptr) << "," // Was interference target confirmed
               << (classicInterferencer.getNumberOfNewFacts() > 0) << "," // Was new knowledge explored
               << "Not applicable" << "," // Was zero rep. met
               << "Not applicable" << "," // Zero rep. occurence step
               << "Not applicable" << "," // Number of zero reps found during interference
               << classicInterferencer
                    .getNumberOfStructuresSearched() << "," // Number of clusters searched
               << numberOfRulesThatCouldBeFiredFromGivenFacts[factPercent] << "," // Number of rules that could be fired
               << QString::fromStdString(classicInterferencer.whyWasntTargetConfirmed()) << "," // Why wasn't interference target confirmed
               << classicInterferencer.getInterferenceTime() << "\n" ; // Interference time [s]
      }

      file.close();
    }
  }

  gSettings->interClusterSimMeasureID = CentroidLinkId;

  ruleInterferencer.setGroupingThread(this->gThread);
  std::vector<int> clusterInterferenceTypes;
  clusterInterferenceTypes.push_back(GREEDY);
  clusterInterferenceTypes.push_back(EXHAUSTIVE);

  // Run interference
  for(int factPercent : factsPercents)
  {
    for(int type : clusterInterferenceTypes)
    {
      qDebug() << "Running interference " << type << ".";

      ruleInterferencer.setInterferenceType(type);

      ruleInterferencer.factsBasePercent = factPercent;

      ruleInterferencer.interfere();

      //qDebug() << "Generating report.";

      // For rule interferencer
      if(file.open(QIODevice::ReadWrite  | QIODevice::Append))
      {
        QTextStream stream(&file);

        stream // Base
               << baseName << "," // Knowledge base name
               << gSettings->attributesNumber << "," // Number of attributes
               << settings->objectsNumber << "," // Number of rules
               // Clustering
               //<< ui->comboBoxInterClusterSimMeasure->currentText() << "," // Clustering similarity measure
               << clusteringSimilarityMeasuresShotrcuts.at(ui->comboBoxInterClusterSimMeasure->currentIndex()) // Clustering similarity measure (shortcut)
               << ui->comboBoxInterObjectSimMeasure->currentText() << "," // Object similarity measure
               //<< ui->comboBoxRepresentativeCreationStrategy->currentText() << "," // Representative generation method
               << representativeGenerationMethodShortcuts.at(ui->comboBoxRepresentativeCreationStrategy->currentIndex())
               << ui->spinBoxRepresentativeAttributePercent->value() << "," // Representative threshold
               << gThread->getGroupingTime() << "," // Grouping time [s]
               << findSmallestCluster()->size() << "," // Smallest cluster size
               << findBiggestCluster()->size() << "," // Biggest cluster size
               << countUngroupedObjects() << "," // Number of outliers
               << getSmallestRepresentativeLength() << "," // Smallest representative len
               << getAverageRepresentativeLength() << "," // Average rep. len.
               << getBiggestRepresentativeLength() << "," // Biggest rep. len.
               << gSettings->zeroRepresentativesNumber << "," // Zero rep. num.
               << gSettings->giniIndex << "," // Gini
               << gSettings->bonferroniIndex << "," // Bonferroni
               << settings->stopCondition << "," // Number of clusters
               << getClusteringTreeLevel() << "," // Number of levels in the tree
               // Interference
               << QString::fromStdString(
                    ruleInterferencer.getInterferenceType()
                  ) << "," // Interference type
               << factPercent << "," // Facts base percent
               << ruleInterferencer.getInitialNumberOfFacts() << "," // Number of initial facts
               << ruleInterferencer.initiallyFireableRuleIndexes.size() << "," // Number of rules that could initially be fired
               << (ruleInterferencer.target.size() > 0) << "," // Was interference target set
               << ruleInterferencer.targetAchiveable << "," // Was interference target initially confirmable
               << ruleInterferencer
                    .getNumberOfIterations() << "," // Number of interference iterations
               << ruleInterferencer
                    .wasRuleFired()<< "," // Was any rule fired
               << ruleInterferencer
                    .numberOfRulesFired << "," // Number of rules that were fired
               << ruleInterferencer
                    .getNumberOfNewFacts() << "," // Number of new facts
               << ruleInterferencer
                    .targetAchieved << "," // Was interference target confirmed
               << (ruleInterferencer.getNumberOfNewFacts() > 0) << "," // Was new knowledge explored
               << ruleInterferencer.zeroRepresentativeOccured << "," // Was zero rep. met
               << ruleInterferencer.zeroRepresentativeOccurenceStep << "," // Zero rep. occurence step
               << ruleInterferencer.zeroRepresentativeOccurenceNumber << "," // Number of zero reps found during interference
               << ruleInterferencer.numberOfClustersSearched << "," // Number of clusters searched
               << numberOfRulesThatCouldBeFiredFromGivenFacts[factPercent] << "," // Number of rules that could be fired
               << QString::fromStdString(ruleInterferencer.whyWasntTargetConfirmed()) << "," // Why wasn't interference target confirmed
               << ruleInterferencer.getInterferenceTime() << "\n" ; // Interference time [s]
      }

      file.close();
    }
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
