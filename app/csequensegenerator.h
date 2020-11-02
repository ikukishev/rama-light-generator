#ifndef CSEQUENSEGENERATOR_H
#define CSEQUENSEGENERATOR_H
#include <fstream>
#include <QString>

class CLightSequence;

class CSequenseGenerator
{   
    CSequenseGenerator() = default;
public:
    static bool generateLms( const CLightSequence* sequense );

};

#endif // CSEQUENSEGENERATOR_H
