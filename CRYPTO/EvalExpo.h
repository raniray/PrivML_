#ifndef DAPPLE_EVALEXPO_H
#define DAPPLE_EVALEXPO_H

#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "../UTIL/math_util.hh"

class EvalExpo {


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
    EvalExpo(Cipher a,int b, DTPKC dtpkc);


    public :
    std::vector<Cipher> EvalSig_U1_step1();


    public :
    Cipher EvalSig_U2_step2(Cipher S1,Cipher S2);


    public :
    Cipher EvalSig_U1_step3(Cipher S7);
};

#endif //DAPPLE_EVALSIGMOID_H
