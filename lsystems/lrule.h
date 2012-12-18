#ifndef LRULE_H
#define LRULE_H

#include <QString>
#include <QList>

class LRule
{
public:
    LRule(QString left, QList<QChar> &right);
    LRule(LRule &rule);
    ~LRule();

    QString left();
    QList<QChar> &right();
    int rightLength();
    QString toString();

private:
    QString _left;
    QList<QChar> _right;
};

#endif // LRULE_H
