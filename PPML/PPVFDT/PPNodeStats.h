/**
** Class :  NodeStats
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : The class that contains the stats a given node : these stats are used to compute the splitting criterion and to achieve the classification
**/

#ifndef DAPPLE_PPNODESTATS_H
#define DAPPLE_PPNODESTATS_H

#include <iostream>
#include <map>
#include <iterator>
#include "../IO/PPRecord.h"
#include <gmpxx.h>
#include <gmp.h>
#include "../../UTIL/util_gmp_rand.h"// Useful functions for random Big integers generation
#include "../../UTIL/num_th_alg.hh" // Useful for random big primes generation
#include "../../UTIL/mpz_class.hh" // Extension to the GMP C++ interface
#include "../../CRYPTO/DTPKC.h"
#include "../../CRYPTO/EvalAdd.h"
#include "../../CRYPTO/EvalAddPow2.h"

class PPNodeStats {

public :

    typedef std::map<Cipher,std::map<Cipher,Cipher>> Stats;
    std :: map <int, Stats > dataAVC;
    std::map <Cipher,Cipher> classOccurrence;
    Cipher mostFrequentClass;
    bool diffClass; Cipher mostFrequentClassOccurrence;
    int  classAtt;
    Cipher totSampleNum;
    int classNumber;
    map <int,Cipher> sumOfSquares;
    map <int, Cipher> sumOfValues;
    Cipher one;
    Cipher zero;
    DTPKC dtpkc;




    /******* Functions **********/

public :
    PPNodeStats(int classAtt, int classNumber);

public :
    void updateDataAVC(PPRecord * r, int i);

public:
    void UpdateNodeStats(PPRecord * r);

public :
    void computeMostFrequentClass();

public :
    void incrementClassOccurence(PPRecord * r);

public :

     PPNodeStats(PPNodeStats * nodeStats);


public:
    ~PPNodeStats(){

        try {
            dataAVC.clear();
            classOccurrence.clear();
            sumOfSquares.clear();
            sumOfValues.clear();

        }catch (exception e )
        {
            cout << "Trying to remove node stats elements " << endl;
            cout << e.what() << endl;
        }
    }
};


#endif
