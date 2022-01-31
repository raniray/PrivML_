#ifndef DAPPLE_EVALPROBA_H
#define DAPPLE_EVALPROBA_H
#include "DTPKC.h"
#include "EvalDiv100.h"
#include "../UTIL/math_util.hh"


class EvalProba{

    private :
    std::vector<Cipher> Tk;
    DTPKC dtpkc;
    mpz_class r;


    public:
    double timeSU, timeMU;
    double bdwSU,bdwMU;

    public :
    EvalProba(std::vector<Cipher> Tk, DTPKC dtpkc);


    public :
    std::vector < Cipher> EvalProba_U1_step1();


    public :
    Cipher EvalProba_U2_step2(std::vector<Cipher> res_step1);

    public :
    Cipher EvalProba_U1_step3(Cipher blinded_result);

};

#endif //DAPPLE_EVALPROBA_H
