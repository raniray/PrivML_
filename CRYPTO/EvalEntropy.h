/**
** Class :  EvalEntropy
 * Authors : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
** Purpose : Compute the approximate value of entropy
 *
 * Remark : Pecision * 10000
 *
**/

#ifndef DAPPLE_EVALENTROPY_H
#define DAPPLE_EVALENTROPY_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "../UTIL/math_util.hh"


class EvalEntropy{


    /**
     *  ***************Attributes **************
     *  T : Encrypted value of the size of the set
     *  Tk : Encrypted  value of the size of the k th set
     *  c : Encryption of the entropy result
     *  rx, ry : random values used for cryptographic blinding
     */

    private :
    Cipher T;
    std::vector<Cipher> Tk;
    DTPKC dtpkc;
    mpz_class rx, ry;
    int prec;

    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;

    /******* Functions **********/

    // Constructor : used to initialise the operands
    public :
    EvalEntropy(Cipher T,std::vector<Cipher> Tk, DTPKC dtpkc);

    // The first step of the secure entropy protocol : This part of the code is run by U1
    public :
    std::vector < Cipher> EvalEntropy_U1_step1();

    // The second step of the secure entropy protocol : This part of the code is run by U2
    public :
    Cipher EvalEntropy_U2_step2(std::vector<Cipher> res_step1);

    // The third step of the secure entropy protocol : This part of the code is run by U1
    public :
    Cipher EvalEntropy_U1_step3(Cipher blinded_result);

};

#endif //DAPPLE_EVALENTROPY_H
