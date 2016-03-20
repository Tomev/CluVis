#include "clusterinfo_rsesrules.h"
#include "ui_clusterinfo_rsesrules.h"

#include <QDebug>

clusterInfo_RSESRules::clusterInfo_RSESRules(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clusterInfo_RSESRules)
{
    ui->setupUi(this);
}

clusterInfo_RSESRules::clusterInfo_RSESRules(ruleCluster* c) :
    ui(new Ui::clusterInfo_RSESRules)
{
    ui->setupUi(this);
    this->c = *c;

    QString title = tr("clusterInfo.clusterInfo") + c->name();

    this->setWindowTitle(title);

    ui->labelRulesNumberValue->setText("<b>" + QString::number(c->size())+ "</b>");
    ui->labelGroupRepresentativeValue->setText("<b>" + c->representative()+ "</b>");

    QStringList cAttributes = c->premiseAttributes.toList();

    ui->labelConditionAttributesValue->setText("<b>" + cAttributes.join(", ")+ "</b>");

    QStringList dAttributes = c->decisionAttributes.toList();

    ui->labelDecisionAttributesValue->setText("<b>" + dAttributes.join(", ")+ "</b>");
    ui->labelLongestRuleValue->setText("<b>" + c->getLongestRule()
                                       .replace("&", " & ")
                                       .replace("=>", " => ") + "</b>");
    ui->labelShortestRuleValue->setText("<b>" + c->getShortestRule()+ "</b>");
    ui->labelMostCommonDecisionValue->setText("<b>" + c->getMostCommonDecision()+ "</b>");
    ui->labelLeastCommonDecisionValue->setText("<b>" + c->getLeastCommonDecision()+ "</b>");

    ui->labelRuleSupportValue->setText("<b>" + QString::number(c->support) + "<b>");
}

clusterInfo_RSESRules::~clusterInfo_RSESRules()
{
    delete ui;
}

void clusterInfo_RSESRules::on_pushButtonVisualize_clicked()
{
    emit passClusterToVisualize(&c);
    this->close();
}
