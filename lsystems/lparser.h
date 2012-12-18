#ifndef LPARSER_H
#define LPARSER_H

#include <QString>

#include "lsystem.h"

class LParser
{
public:
    LParser();

    void parseFile(QString fileName, LSystem &lsystem);
};

#endif // LPARSER_H
