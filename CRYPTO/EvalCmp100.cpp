#include <thread>
#include "EvalCmp100.h"


EvalCmp100 ::EvalCmp100(Cipher A, Cipher B , DTPKC dtpkc)
{
    this->A=A;
    this->B=B;
    this->dtpkc=dtpkc;
}

std::vector<Cipher>
EvalCmp100::EvalCmp_U1_step1() {


    auto begin = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    Cipher S1, S2, S3, S4;
    std::vector<Cipher> res(4);


    Cipher C_R;

    if(!dtpkc.optim) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));

        r = mpz_class_get_z_bits(dtpkc.blindVal);
        r2 = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randclear(state);
        C_R = dtpkc.enc(r2, A.Pub);
    }else{
        r = dtpkc.R1;
        r2 = dtpkc.R2;
        C_R = dtpkc.CR2;
    }

            S1.T1 = mpz_class_powm(A.T1, r*100, dtpkc.n2);
            S1.T2 = mpz_class_powm(A.T2, r*100, dtpkc.n2);

            // A*r
            S1.T1 = S1.T1 *C_R.T1;
            S1.T2 =  S1.T2 *C_R.T2;
            S1.Pub = A.Pub;

            S3 = dtpkc.PSdec0(S1);
            res[0] = S1;
            res[1] = S3;


            S2.T1 = mpz_class_powm(B.T1, r, dtpkc.n2);
            S2.T2 = mpz_class_powm(B.T2, r, dtpkc.n2);

            // B*r
            S2.T1 = S2.T1 *C_R.T1;
            S2.T2 =  S2.T2 *C_R.T2;
            S2.Pub = A.Pub;



            S4 = dtpkc.PSdec0(S2);

            res[2] = S2;
            res[3] = S4;


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = res.size() * mpz_size(res[0].T1.get_mpz_t())+ mpz_size(res[0].T2.get_mpz_t())* sizeof(mp_limb_t);
    return res;
}


/**
** Function : EvalCmp_U2_step2
 * Author : Talbi Rania
** Creation date : 25/11/2018
** Last Update : 25/11/2018
 *
** Purpose : The second step of the secure EvalCmp_U2_step2 protocol : This part of the code is run by U2
 *
 * Parameters :
 * res 1 : results sent by U1 from the previous steps
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/


bool EvalCmp100::EvalCmp_U2_step2(std::vector <Cipher> res1)
{

    /**
     * x: A*r
     * B: B*r
     */
    auto begin = chrono::high_resolution_clock::now();
    mpz_class x, y, out;

            x = dtpkc.PSdec1(res1[0], res1[1]);
            y = dtpkc.PSdec1(res1[2], res1[3]);



    // return the comparison output

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = 1;
    std::this_thread::sleep_for(dtpkc.delay);
    return  x < y ;

}

