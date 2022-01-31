#ifndef DAPPLE_PPSTATS_H
#define DAPPLE_PPSTATS_H

#include <iostream>
#include <map>
#include <iterator>
#include "../IO/PPRecord.h"
#include <gmpxx.h>
#include <gmp.h>
#include <omp.h>
#include "../../UTIL/util_gmp_rand.h"
#include "../../UTIL/num_th_alg.hh"
#include "../../UTIL/mpz_class.hh"
#include "../../CRYPTO/DTPKC.h"
#include "../../CRYPTO/EvalAdd.h"
#include "../../CRYPTO/EvalMult.h"
#include "../../CRYPTO/EvalDiv100.h"
#include "../../CRYPTO/EvalProba.h"
#include "../../CRYPTO/EvalLog.h"

class PPStats {


public :

    typedef std::map<Cipher,std::map<Cipher,Cipher>> st;
    typedef std::map<Cipher,std::map<Cipher,Cipher>> stf;
    std :: map <int, st > dataAVC;
    std::map <Cipher, Cipher> classOccurrence;
    Cipher totSampleNum;
    int classNumber, classAtt;
    std::map <Cipher, Cipher> priorProba;
    std :: map <int, stf > conditionalProba;
    std::map <Cipher, Cipher> LogpriorProba;
    std :: map <int, stf > LogconditionalProba;
    static DTPKC dtpkc;
    static Cipher one;
    static Cipher zero;

    /******* Functions **********/

public :
    ~PPStats(){
    dataAVC.clear();
    classOccurrence.clear();
    priorProba.clear();
    conditionalProba.clear();

    }
public :
    PPStats(int classNumber, int classAtt);

public :
    void InitStats();

public :
    Cipher Classify(PPRecord * r);

public :
    void UpdateClassifier(PPRecord * r);

public :
    void  UpdateStatistics(PPRecord * r);

public :
    void incrementClassOccurence(PPRecord * r);

public :
    void updateDataAVC (PPRecord * r, int i );
public :
    void print();
public :
    void h();
};

#endif //DAPPLE_PPSTATS_H
