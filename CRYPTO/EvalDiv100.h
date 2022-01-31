#ifndef DAPPLE_EVALDIV100_H
#define DAPPLE_EVALDIV100_H

#include "DTPKC.h" // Header of the DTPKC cryptosystem's class


class EvalDiv100 {


    private :
    Cipher a, b, c;
    DTPKC dtpkc;
    mpz_class rx, ry;

    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;

    /******* Functions **********/

    // Constructor : used to initialise the operands
    public :
    EvalDiv100(Cipher a,Cipher b, DTPKC dtpkc);

    // The first step of the secure division * 100 protocol : This part of the code is run by U1
    public :
    std::vector<Cipher> EvalDiv_U1_step1();

    // The second step of the secure division *100 protocol : This part of the code is run by U2
    public :
    Cipher EvalDiv_U2_step2(Cipher S1,Cipher S2,Cipher S3,Cipher S4);

    // The third step of the secure division *100 protocol : This part of the code is run by U1
    public :
    Cipher EvalDiv_U1_step3(Cipher S7);

};

#endif //DAPPLE_EVALDIV_H
