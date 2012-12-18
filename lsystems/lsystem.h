#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <QList>
#include <QChar>
#include <QString>
#include <QHash>
#include <QDebug>
#include <OpenCL/opencl.h>
#include <cmath>

#include "lrule.h"

class LSystem
{
public:
    LSystem();
    ~LSystem();

    void addVariable(QChar var);
    void addConstant(QChar konst);
    void setAxiom(QString axiom);
    void addRule(LRule &rule);
    void clear();
    size_t symbolsW();
    size_t symbolsH();
    cl_char *symbolsBuf();
    cl_char *axiomBuf();
    int axiomLength();
    cl_float *matricesBuf(float angle);

    QString toString();

private:
    QList<QChar> _variables;
    QList<QChar> _constants;
    QString _axiom;
    QList<LRule *> _rules;
    QHash<QChar, int> _symbols;

    int _maxRuleLength();
    int _nSymbols();
    void _assignSymbols();
};

#endif // LSYSTEM_H
