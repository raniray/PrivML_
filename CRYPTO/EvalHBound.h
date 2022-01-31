/**
** Class :  EvalHBound
 * Authors : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
** Purpose : Compute the hoeffding bound
 *
**/


#ifndef DAPPLE_EVALHBOUND_H
#define DAPPLE_EVALHBOUND_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "EvalDiv.h"

class EvalHBound {

    private :
    Cipher a, b;
    DTPKC dtpkc;
    mpz_class r;

    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;


    /******* Functions **********/

    // Constructor : used to initialise the operands
    public :
    EvalHBound(Cipher a,Cipher b, DTPKC dtpkc);

    // The first step of the secure HBound computation protocol : This part of the code is run by U1
    public :
    std::vector<Cipher> EvalHBound_U1_step1();

    // The second step of the secure HBound computation protocol : This part of the code is run by U2
    public :
    Cipher EvalHBound_U2_step2(std::vector<Cipher> res1);

    // The third step of the secure HBound computation protocol : This part of the code is run by U1
    public :
    Cipher EvalHbound_U1_step3(Cipher res2);

};

#endif //DAPPLE_EVALHBOUND_H
