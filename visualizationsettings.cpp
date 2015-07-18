#include "visualizationsettings.h"

visualizationSettings::visualizationSettings(QObject *parent) : QObject(parent)
{

}

visualizationSettings::~visualizationSettings()
{

}

//Setters

void visualizationSettings::setGroupingAlgorithmId(int algorithm)
{
    groupingAlgorithmId = algorithm;
}

void visualizationSettings::setVisualizationAlgorithmId(int algorithm)
{
    visualizationAlgorithmId = algorithm;
}

void visualizationSettings::setKnowledgeBaseInfo(QFileInfo info)
{
    knowledgeBaseInfo = info;
}

//Getters

int visualizationSettings::getGroupingAlgorithmId()
{
    return groupingAlgorithmId;
}

int visualizationSettings::getVisualizationAlgorithmId()
{
    return visualizationAlgorithmId;
}

QFileInfo visualizationSettings::getKnowledgeBaseInfo(){
    return knowledgeBaseInfo;
}



