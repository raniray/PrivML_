/**
** Class :  NodeStats
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : The class that contains the stats a given node : these stats are used to compute the splitting criterion and to achieve the classification
**/

#ifndef DAPPLE_NODESTATS_H
#define DAPPLE_NODESTATS_H

#include <iostream>
#include <map>
#include <iterator>
#include "../IO/Record.h"
#include <gmpxx.h>
#include <gmp.h>
#include "../../UTIL/util_gmp_rand.h"// Useful functions for random Big integers generation
#include "../../UTIL/num_th_alg.hh" // Useful for random big primes generation
#include "../../UTIL/mpz_class.hh" // Extension to the GMP C++ interface

class NodeStats {


public :

    typedef std::map<int,std::map<int,int>> st;
    std :: map <int, st > dataAVC;

    std::map <int, int> classOccurrence;
    int mostFrequentClass;
    bool diffClass; int mostFrequentClassOccurrence;
    int classAtt;
    int totSampleNum, classNumber;
    map <int,mpz_class> sumOfSquares;
    map <int, mpz_class> sumOfValues;


    /******* Functions **********/


public :
    NodeStats(int classAtt, int classNumber);


public :
    void updateDataAVC(Record * r, int i);

public :
   NodeStats(NodeStats * nodeStats);

public:
    void UpdateNodeStats(Record * r);


public :
    void computeMostFrequentClass();


public :
    void incrementClassOccurence(Record * r);

public :
    ~NodeStats(){

        dataAVC.clear();
        classOccurrence.clear();
        sumOfSquares.clear();
        sumOfValues.clear();
}

};

#endif //DAPPLE_NODESTATS_H
