#ifndef DAPPLE_EVALSTANDARDDIV_H
#define DAPPLE_EVALSTANDARDDIV_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "EvalDiv.h"
#include "EvalMult.h"
#include "EvalSub.h"

class EvalStandardDiv{


private :
    Cipher A, B, C;
    DTPKC dtpkc;
    mpz_class r;

public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;

    /******* Functions **********/


    public :
    EvalStandardDiv(Cipher A,Cipher B, Cipher C, DTPKC dtpkc);


    public :
    std::vector<Cipher> EvalStandardDiv_U1_step1();


    public :
    Cipher EvalStandardDiv_U2_step2(std::vector<Cipher> res1);


    public :
    Cipher EvalStandardDiv_U1_step3(Cipher res2);

};


#endif //DAPPLE_EvalStandardDiv_H
