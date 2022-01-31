/**
** Class :  EvalCmp
 * Authors : Talbi Rania
** Creation date : 25/11/2018
** Last Update : 25/11/2018
** Purpose : Compare two ciphers
 *
**/

#ifndef DAPPLE_EVALCMP_H
#define DAPPLE_EVALCMP_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "../UTIL/math_util.hh"
#include <vector>


class EvalCmp{


    /**
     *  ***************Attributes **************
     *  A : first operand
     *  B : second operand
     *  dtpkc : used to get the DTPKC parameters
     *  r : random value used for cryptographic blinding
     */

    private :
    Cipher  A, B;
    DTPKC dtpkc;
    mpz_class r, r2;


public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;


    /******* Functions **********/

    // Constructor : used to initialise the operands
    public :
    EvalCmp(Cipher A, Cipher B , DTPKC dtpkc);

    // The first step of the secure comparison protocol : This part of the code is run by U1
    public :
    std :: vector < Cipher> EvalCmp_U1_step1();

    // The second step of the secure comparison protocol : This part of the code is run by U2
    public :
    bool EvalCmp_U2_step2(std::vector<Cipher> res1);

};

#endif //DAPPLE_EVALCMP_H
