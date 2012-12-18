#include "lrule.h"

LRule::LRule(QString left, QList<QChar> &right)
{
    _left = left;
    _right.append(right);
}

LRule::LRule(LRule &rule)
{
    _left = rule.left();
    _right.clear();
    _right.append(rule.right());
}

LRule::~LRule()
{
}

QString LRule::left()
{
    return _left;
}

QList<QChar> &LRule::right()
{
    return _right;
}

int LRule::rightLength()
{
    return _right.length();
}

QString LRule::toString()
{
    QString rep(_left + "-->");
    QList<QChar>::const_iterator it = _right.constBegin();
    while (it != _right.constEnd()) {
        rep.append(QString(*it));
        ++it;
    }

    return rep;
}
