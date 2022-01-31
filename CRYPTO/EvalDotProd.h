#ifndef DAPPLE_EVALDOTPROD_H
#define DAPPLE_EVALDOTPROD_H

#include "DTPKC.h" // Header of the DTPKC cryptosystem's class


class EvalDotProd {


    private :
    std::vector<Cipher> a, b;
    int size;
    DTPKC dtpkc;
    mpz_class r1, r2, r3;

    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;

    /******* Functions **********/

    public :
    EvalDotProd(std::vector<Cipher> a ,std::vector<Cipher> b, int size,  DTPKC dtpkc);

    public :
    std::vector<Cipher> EvalDotProd_U1_step1();

    public :
    Cipher EvalDotProd_U2_step2(std::vector<Cipher> val );

    public :
    Cipher EvalDotProd_U1_step3(Cipher val );

};

#endif
