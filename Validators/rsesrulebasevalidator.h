#ifndef RSESRULEBASEVALIDATOR_H
#define RSESRULEBASEVALIDATOR_H

#include "validator.h"

#include <QFileInfo>

class rsesRuleBaseValidator : public validator
{
  public:
    rsesRuleBaseValidator(QFileInfo* ruleBase);
    ~rsesRuleBaseValidator();
    int validate();

  private:
    QFileInfo* ruleBase;

};

#endif // RSESRULEBASEVALIDATOR_H
