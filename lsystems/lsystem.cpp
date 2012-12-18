#include "lsystem.h"

LSystem::LSystem()
{
}

LSystem::~LSystem()
{
}

void LSystem::addVariable(QChar var)
{
    _variables.append(var);
}

void LSystem::addConstant(QChar konst)
{
    _constants.append(konst);
}

void LSystem::setAxiom(QString axiom)
{
    _axiom = axiom;
}

void LSystem::addRule(LRule &rule)
{
    LRule *rulep = new LRule(rule);
    _rules.append(rulep);
}

void LSystem::clear()
{
    _variables.clear();
    _constants.clear();
    _rules.clear();
    _symbols.clear();
}

size_t LSystem::symbolsW()
{
    return (4 + _maxRuleLength());
}

size_t LSystem::symbolsH()
{
    return _nSymbols();
}

cl_char *LSystem::symbolsBuf()
{
    int w = 4 + _maxRuleLength();
    int h = _nSymbols();
    cl_char *buf = (cl_char *) malloc(sizeof(cl_char) * w * h);

    _assignSymbols();

    int i = 0;
    int j;

    QList<LRule *>::const_iterator itr = _rules.constBegin();
    while (itr != _rules.constEnd()) {
        int rlen = (*itr)->rightLength();
        buf[i * w + 0] = rlen;

        j = 1;
        QList<QChar>::const_iterator itc = (*itr)->right().constBegin();
        while (itc != (*itr)->right().constEnd()) {
            int rep = _symbols[*itc];
            buf[i * w + j] = rep;

            ++j;
            ++itc;
        }

        ++i;
        ++itr;
    }

    QList<QChar>::const_iterator itc = _constants.constBegin();
    while (itc != _constants.constEnd()) {
        buf[i * w + 0] = 1;
        buf[i * w + 1] = _symbols[*itc];

        ++i;
        ++itc;
    }

    return buf;
}

cl_char *LSystem::axiomBuf()
{
    int len = _axiom.length();
    cl_char *buf = (cl_char *) malloc(sizeof(cl_char) * len);

    int i = 0;
    QString::const_iterator it = _axiom.constBegin();
    while (it != _axiom.constEnd()) {
        int rep = _symbols[*it];
        buf[i] = rep;

        ++i;
        ++it;
    }

    return buf;
}

int LSystem::axiomLength()
{
    return _axiom.length();
}

cl_float *LSystem::matricesBuf(float angle)
{
    const float PI = atan(1.0)*4.0;
    int w = 3 * 3 + 1;
    int h = _nSymbols();
    cl_float *buf = (cl_float *) malloc(sizeof(cl_float) * w * h);
    angle = angle * PI / 180.0f;
    cl_float forward[] = {1.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f,
                          0.1f, 0.0f, 1.0f};
    cl_float left[] = {cos(angle), sin(angle), 0.0f,
                       -sin(angle), cos(angle), 0.0f,
                       0.0f, 0.0f, 1.0f};
//                       0.5 * (1 - cos(angle)) + 0.5 * sin(angle),
//                       0.5 * (1 - cos(angle)) - 0.5 * sin(angle), 1.0f};
    cl_float right[] = {cos(angle), -sin(angle), 0.0f,
                        sin(angle), cos(angle), 0.0f,
                        0.0f, 0.0f, 1.0f};
//                        0.5 * (1 - cos(angle)) + 0.5 * sin(angle),
//                        0.5 * (1 - cos(angle)) - 0.5 * sin(angle), 1.0f};

    cl_float identity[] = {1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 1.0f};

    QList<QChar> symbols;
    symbols.append(_variables);
    symbols.append(_constants);
    QList<QChar>::const_iterator it = symbols.constBegin();
    while (it != symbols.constEnd()) {
        int row = _symbols[*it];

        switch(it->toAscii()) {
        case 'F': // Move forward w = (1, 0)
        case 'G':
        case 'A':
        case 'B':
            buf[row * w] = 1.0f;
            memcpy(&buf[row * w + 1], forward, sizeof(cl_float) * 9);
            break;
        case '+': // Turn left
            buf[row * w] = 0.0f;
            memcpy(&buf[row * w + 1], left, sizeof(cl_float) * 9);
            break;
        case '-': // Turn right
            buf[row * w] = 0.0f;
            memcpy(&buf[row * w + 1], right, sizeof(cl_float) * 9);
            break;
        default:
            buf[row * w] = 0.0f;
            memcpy(&buf[row * w + 1], identity, sizeof(cl_float) * 9);
            break;
        }

        ++it;
    }

    return buf;
}

QString LSystem::toString()
{
    QString rep("Variables: ");
    QList<QChar>::const_iterator its = _variables.constBegin();
    while (its != _variables.constEnd()) {
        rep.append(QString(*its) + " ");
        ++its;
    }
    rep.append("\nConstants: ");
    its = _constants.constBegin();
    while (its != _constants.constEnd()) {
        rep.append(QString(*its) + " ");
        ++its;
    }
    rep.append("\nAxiom: " + _axiom);
    rep.append("\nRules:\n");
    QList<LRule *>::const_iterator itr = _rules.constBegin();
    while (itr != _rules.constEnd()) {
        rep.append((*itr)->toString() + "\n");
        ++itr;
    }

    return rep;
}

int LSystem::_maxRuleLength()
{
    int ret = 0;

    QList<LRule *>::const_iterator it = _rules.constBegin();
    while (it != _rules.constEnd()) {
        int len = (*it)->rightLength();
        ret = qMax(ret, len);
        ++it;
    }

    return ret;
}

int LSystem::_nSymbols()
{
    return _variables.length() + _constants.length();
}

void LSystem::_assignSymbols()
{
    int idx = 0;

    QList<QChar>::const_iterator it = _variables.constBegin();
    while (it != _variables.constEnd()) {
        _symbols.insert(*it, idx);

        ++idx;
        ++it;
    }

    it = _constants.constBegin();
    while (it != _constants.constEnd()) {
        _symbols.insert(*it, idx);

        ++idx;
        ++it;
    }
}
