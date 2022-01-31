/**
** Class :  EvalHBound
 * Authors : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
** Purpose : Compute the hoeffding bound
 *
**/


#include <thread>
#include "EvalHBound.h"


/**
** Function : EvalHBound
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : This is the constructor of the class, it is used to initialise the operands
 *
 * Parameters :
 * a : first operand ( log2(c)~2 * log ( 1/ delta) )
 * b : second operand (T : Total number of occurences )
 * dtpkc : used to get the DTPKC parameters
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

EvalHBound::EvalHBound(Cipher a, Cipher b, DTPKC dtpkc)
{
    this->a=a;
    this->b=b;
    this->dtpkc=dtpkc;
}

/**
** Function : EvalHBound_U1_step1
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : The first step of the secure HBound computation protocol : This part of the code is run by U1
 *
 * Parameters :
 * none
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

std::vector<Cipher> EvalHBound::EvalHBound_U1_step1()
{

    auto begin = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    Cipher S1, S11,S2;

    if(!dtpkc.optim) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));
        r = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randclear(state);
    }else{

        r = dtpkc.R1;
    }

    EvalDiv evalDiv(a,b,dtpkc);
    std::vector <Cipher> res = evalDiv.EvalDiv_U1_step1();
    S1 = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0],res[1], res[2], res[3]));

    // compute (log2(c)~2 * log ( 1/ delta) / 2* T) * - r
    S11.T1=mpz_class_powm(S1.T1,r,dtpkc.n2);
    S11.T2=mpz_class_powm(S1.T2,r,dtpkc.n2);
    S11.Pub=a.Pub;

    // Partially decrypt (log2(c)~2 * log ( 1/ delta) / 2* T) *  r
    S2=dtpkc.PSdec0(S11);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = 2* mpz_size(S11.T1.get_mpz_t())+ mpz_size(S11.T2.get_mpz_t())* sizeof(mp_limb_t);

    return {S11,S2};

}

/**
** Function : EvalHBound_U2_step2
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : The second step of the secure HBound computation protocol : This part of the code is run by U2
 *
 * Parameters :
 * res1 : results of the last step sent by U1
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

Cipher
EvalHBound::EvalHBound_U2_step2(std::vector<Cipher> res1)
{

    auto begin  = chrono::high_resolution_clock::now();

    mpz_class x;
    Cipher res;
    // log2(c)~2 * log ( 1/ delta) / 2* T) *  r
    x=dtpkc.PSdec1(res1[0],res1[1]);
    // Compute sqrt (log2(c)~2 * log ( 1/ delta) / 2* T) *  r )
    res = dtpkc.enc(mpz_class_sqrt(x),a.Pub);

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(res.T1.get_mpz_t())+ mpz_size(res.T2.get_mpz_t())* sizeof(mp_limb_t);

    return res;

}

/**
** Function : EvalHbound_U1_step3
 * Author : Talbi Rania
** Creation date : 27/11/2018
** Last Update : 27/11/2018
 *
** Purpose : The third step of the secure HBound computation protocol : This part of the code is run by U1
 *
 * Parameters :
 * res :  the result of the previous step sent by U2
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

Cipher
EvalHBound::EvalHbound_U1_step3(Cipher res2)
{

    auto begin = chrono::high_resolution_clock::now();
    Cipher blindVal;
    Cipher out;

    // sqrt (r)
    blindVal = dtpkc.enc(mpz_class_sqrt(r), a.Pub);

    // divide by sqrt (r) to get Hbound
    EvalDiv evalDiv(res2,blindVal,dtpkc);
    std::vector <Cipher> res = evalDiv.EvalDiv_U1_step1();

    out = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0],res[1], res[2], res[3]));



    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(out.T1.get_mpz_t())+ mpz_size(out.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return out;

}
