#ifndef DAPPLE_EVALLRUPDATE_H
#define DAPPLE_EVALLRUPDATE_H

#include "DTPKC.h" // Header of the DTPKC cryptosystem's class


class EvalLrUpdate {


    private :
    std::vector<Cipher> a, b, c;
    int alpha;
    DTPKC dtpkc;
    mpz_class r1, r2, r3;

    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;

    /******* Functions **********/

    public :
    EvalLrUpdate(std::vector<Cipher> a ,std::vector<Cipher> b, std::vector<Cipher> c, int alpha,  DTPKC dtpkc);

    public :
    std::vector<Cipher> EvalLrUpdate_U1_step1();

    public :
    Cipher EvalLrUpdate_U2_step2(std::vector<Cipher> val );

    public :
    Cipher EvalLrUpdate_U1_step3(Cipher val );

};

#endif
