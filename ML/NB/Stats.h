#ifndef DAPPLE_STATS_H
#define DAPPLE_STATS_H

#include <iostream>
#include <map>
#include <iterator>
#include "../IO/Record.h"
#include <gmpxx.h>
#include <gmp.h>
#include "../../UTIL/util_gmp_rand.h"// Useful functions for random Big integers generation
#include "../../UTIL/num_th_alg.hh" // Useful for random big primes generation
#include "../../UTIL/mpz_class.hh" // Extension to the GMP C++ interface

class Stats {

public :

    typedef std::map<int,std::map<int,int>> st;
    typedef std::map<int,std::map<int,float>> stf;

    std :: map <int, st > dataAVC;
    std::map <int, int> classOccurrence;
    int totSampleNum, classNumber, classAtt;
    std::map <int, float> priorProba;
    std :: map <int, stf > conditionalProba;
    std::map <int, float> LogpriorProba;
    std :: map <int, stf > LogconditionalProba;
    int gaussCte;

    static bool initialised;

    /******* Functions **********/

public :
    Stats(int classNumber, int classAtt);

public :
    void InitStats(int laplace);


public :
    int Classify(Record * r);

public :
    void UpdateClassifier();

public :
    void  UpdateStatistics(Record * r);

public :
    void incrementClassOccurence(Record * r);

public :
    void updateDataAVC (Record * r, int i );

public :
    void print();

};

#endif //DAPPLE_STATS_H
