/**
** Class :  EvalAddPow2
 * Authors : Talbi Rania
** Creation date : 26/11/2018
** Last Update : 26/11/2018
** Purpose : Compute A + B^2
 *
**/

#ifndef DAPPLE_EVALADDPOW2_H
#define DAPPLE_EVALADDPOW2_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "../UTIL/math_util.hh"
#include <vector>


class EvalAddPow2{


    /**
     *  ***************Attributes **************
     *  A : First operand : old sum of sqaures
     *  B : Second operand : new value to be added
     *  r : blinding value
     */

private :
    Cipher  A, B;
    DTPKC dtpkc;
    mpz_class r;

public  :

    double timeSU, timeMU;
    double bdwSU,bdwMU;

    /******* Functions **********/


    public :
    EvalAddPow2(Cipher A, Cipher B , DTPKC dtpkc);


    public :
    std :: vector < Cipher> EvalAddPow2_U1_step1();


    public :
    Cipher EvalAddPow2_U2_step2(std::vector<Cipher> res1);


    public :
    Cipher EvalAddPow2_U1_step3(Cipher res2);

};


#endif //DAPPLE_EVALADDPOW2_H
