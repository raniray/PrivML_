/**
** Class :  EvalDiv
 * Authors : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 28/11/2018
** Purpose : Compute the approximate value of the division of two integers
 *
 *
**/

#ifndef DAPPLE_EVALDIV_H
#define DAPPLE_EVALDIV_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class


class EvalDiv {


    /**
     *  ***************Attributes **************
     *  a : Encrypted value of the first operand
     *  b : Encrypted  value of the second operand
     *  c : Encryption of the division result
     *  rx, ry : random values used for cryptographic blinding
     */

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
    EvalDiv(Cipher a,Cipher b, DTPKC dtpkc);

    // The first step of the secure division protocol : This part of the code is run by U1
    public :
    std::vector<Cipher> EvalDiv_U1_step1();

    // The second step of the secure division protocol : This part of the code is run by U2
    public :
    Cipher EvalDiv_U2_step2(Cipher S1,Cipher S2,Cipher S3,Cipher S4);

    // The third step of the secure division protocol : This part of the code is run by U1
    public :
    Cipher EvalDiv_U1_step3(Cipher S7);

};

#endif //DAPPLE_EVALDIV_H
