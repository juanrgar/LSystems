#include "lparser.h"

LParser::LParser()
{
}

void LParser::parseFile(QString fileName, LSystem &lsystem)
{
    // Koch Curve
//    lsystem.addVariable('F');

//    lsystem.addConstant('+');
//    lsystem.addConstant('-');

//    QString axiom("F");
//    lsystem.setAxiom(axiom);

//    QList<QChar> rr;
//    rr.append('F');
//    rr.append('+');
//    rr.append('F');
//    rr.append('-');
//    rr.append('-');
//    rr.append('F');
//    rr.append('+');
//    rr.append('F');
//    LRule r("F", rr);
//    lsystem.addRule(r);



    lsystem.addVariable('F');

    lsystem.addConstant('+');
    lsystem.addConstant('-');

    QString axiom("F--F--F");
    lsystem.setAxiom(axiom);

    QList<QChar> rr;
    rr.append('F');
    rr.append('+');
    rr.append('F');
    rr.append('-');
    rr.append('-');
    rr.append('F');
    rr.append('+');
    rr.append('F');
    LRule r("F", rr);
    lsystem.addRule(r);


    // Sierpinski Gasket
//    lsystem.addVariable('A');
//    lsystem.addVariable('B');

//    lsystem.addConstant('+');
//    lsystem.addConstant('-');

//    QString axiom("A");
//    lsystem.setAxiom(axiom);

//    QList<QChar> rr1;
//    rr1.append('B');
//    rr1.append('-');
//    rr1.append('A');
//    rr1.append('-');
//    rr1.append('B');
//    LRule r1("A", rr1);
//    lsystem.addRule(r1);

//    QList<QChar> rr2;
//    rr2.append('A');
//    rr2.append('+');
//    rr2.append('B');
//    rr2.append('+');
//    rr2.append('A');
//    LRule r2("B", rr2);
//    lsystem.addRule(r2);

    // Dragon Curve
//    lsystem.addVariable('X');
//    lsystem.addVariable('Y');

//    lsystem.addConstant('F');
//    lsystem.addConstant('+');
//    lsystem.addConstant('-');

//    QString axiom("FX");
//    lsystem.setAxiom(axiom);

//    QList<QChar> rr1;
//    rr1.append('X');
//    rr1.append('-');
//    rr1.append('Y');
//    rr1.append('F');
//    LRule r1("X", rr1);
//    lsystem.addRule(r1);

//    QList<QChar> rr2;
//    rr2.append('F');
//    rr2.append('X');
//    rr2.append('+');
//    rr2.append('Y');
//    LRule r2("Y", rr2);
//    lsystem.addRule(r2);

    // Koch Cuadratic Island
//    lsystem.addVariable('F');

//    lsystem.addConstant('+');
//    lsystem.addConstant('-');

//    QString axiom("F-F-F-F");
//    lsystem.setAxiom(axiom);

//    QList<QChar> rr;
//    rr.append('F');
//    rr.append('-');
//    rr.append('F');
//    rr.append('+');
//    rr.append('F');
//    rr.append('+');
//    rr.append('F');
//    rr.append('F');
//    rr.append('-');
//    rr.append('F');
//    rr.append('-');
//    rr.append('F');
//    rr.append('+');
//    rr.append('F');
//    LRule r("F", rr);
//    lsystem.addRule(r);

    // Pretty Square
//    lsystem.addVariable('F');

//    lsystem.addConstant('+');
//    lsystem.addConstant('-');

//    QString axiom("F-F-F-F");
//    lsystem.setAxiom(axiom);

//    QList<QChar> rr;
//    rr.append('F');
//    rr.append('F');
//    rr.append('-');
//    rr.append('F');
//    rr.append('-');
//    rr.append('-');
//    rr.append('F');
//    rr.append('-');
//    rr.append('F');
//    LRule r("F", rr);
//    lsystem.addRule(r);
}
