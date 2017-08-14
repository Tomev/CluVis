#include "rsesrulebasevalidator.h"

#include <qdebug.h>

rsesRuleBaseValidator::rsesRuleBaseValidator(QFileInfo* ruleBase)
  : ruleBase(ruleBase) {}

rsesRuleBaseValidator::~rsesRuleBaseValidator()
{
  delete ruleBase;
}

int rsesRuleBaseValidator::validate()
{
  QFile ruleBaseFile(ruleBase->absoluteFilePath());
  QTextStream stream(&ruleBaseFile);
  QString line;

  if(ruleBaseFile.open(QIODevice::ReadOnly))
  {
    line = stream.readLine();
  }

  qDebug() << line;

  ruleBaseFile.close();



  return 1;
}
