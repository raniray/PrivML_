/**
** Class :  SkeySwitch
 * Authors : Talbi Rania
** Creation date : 26/11/2018
** Last Update : 26/11/2018
** Purpose : Change the cryptographic key of a cipher
 *
**/

#include <thread>
#include "SkeySwitch.h"


SkeySwitch::SkeySwitch(Cipher A, mpz_class pkey, DTPKC dtpkc)
{
    this->A=A;
    this->pkey=pkey;
    this->dtpkc=dtpkc;
}

std::vector<Cipher>
SkeySwitch ::SkeySwitch_U1_step1()
{
    auto begin = chrono::high_resolution_clock::now();
    gmp_randstate_t state;
    Cipher S1, S2, S3, S4;
    std::vector <Cipher> res(4);

    if(!dtpkc.optim) {
        // Generate two random numbers : used for multiplicative cryptographic blinding
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));

        r = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randclear(state);

        S1 = dtpkc.enc(r, A.Pub);

    }else{
        r = dtpkc.R1;
        S1 = dtpkc.CR1;
    }
    //A+r
    S2.T1= A.T1*S1.T1;
    S2.T2= A.T2*S1.T2;
    S2.Pub= A.Pub;

    // Partially decrypt A+r
    S3=dtpkc.PSdec0(S2);


    res[0]=S2;
    res[1]=S3;


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = 2*mpz_size(res[0].T1.get_mpz_t())+ mpz_size(res[0].T2.get_mpz_t())* sizeof(mp_limb_t);


    return res;

}

Cipher SkeySwitch::SkeySwitch_U2_step2(std::vector<Cipher> res1)
{
    auto begin = chrono::high_resolution_clock::now();
    mpz_class x;
    Cipher out;

    // Partially decrypt  A+r
    x=dtpkc.PSdec1(res1[0],res1[1]);

    out = dtpkc.enc(x,pkey);

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(out.T1.get_mpz_t())+ mpz_size(out.T2.get_mpz_t())* sizeof(mp_limb_t);


    return  out;

}

Cipher SkeySwitch::SkeySwitch_U1_step3(Cipher res2)
{
    auto begin = chrono::high_resolution_clock::now();
    Cipher S4, S5, res;

    S4 = dtpkc.enc(r,pkey);


    S5.T1=mpz_class_powm(S4.T1,dtpkc.n-1,dtpkc.n2*dtpkc.n2);
    S5.T2=mpz_class_powm(S4.T2,dtpkc.n-1,dtpkc.n2*dtpkc.n2);
    S5.Pub=pkey;


    res.T1 = S5.T1 * res2.T1;
    res.T2 = S5.T2 * res2.T2;
    res.Pub = pkey;




    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(res.T1.get_mpz_t())+ mpz_size(res.T2.get_mpz_t())* sizeof(mp_limb_t);


    std::this_thread::sleep_for(dtpkc.delay);

    return  res;

}

