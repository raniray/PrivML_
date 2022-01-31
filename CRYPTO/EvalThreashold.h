#ifndef DAPPLE_EVALTHREASHOLD_H
#define DAPPLE_EVALTHREASHOLD_H
#include "DTPKC.h" // Header of the DTPKC cryptosystem's class
#include "../UTIL/math_util.hh"
#include <vector>


class EvalThreashold{

    private :
    Cipher  A, B;
    double x;
    DTPKC dtpkc;


    public :
    double timeSU, timeMU;
    double bdwSU,bdwMU;


    /******* Functions **********/

    public :
    EvalThreashold(Cipher A, Cipher B , double x, DTPKC dtpkc);


    public :
    Cipher  eval();

};


#endif
