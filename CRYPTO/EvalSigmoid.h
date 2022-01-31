#ifndef DAPPLE_EVALSIGMOID_H
#define DAPPLE_EVALSIGMOID_H

#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "../UTIL/math_util.hh"

class EvalSigmoid {


    private :
    Cipher u;
    int lambda;
    DTPKC dtpkc;
    mpz_class r, r2;


    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;


    /******* Functions **********/


    public :
    EvalSigmoid(Cipher a,int b, DTPKC dtpkc);


    public :
    std::vector<Cipher> EvalSig_U1_step1();


    public :
    Cipher EvalSig_U2_step2(Cipher S1,Cipher S2);


    public :
    Cipher EvalSig_U1_step3(Cipher S7);

    public :
    float sigmoid (mpz_class u, int prec );
};

#endif //DAPPLE_EVALSIGMOID_H
