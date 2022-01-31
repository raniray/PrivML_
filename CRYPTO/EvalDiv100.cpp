#include <thread>
#include "EvalDiv100.h"


EvalDiv100 ::EvalDiv100(Cipher a, Cipher b, DTPKC dtpkc)
{
    this->a=a;
    this->b=b;
    this->dtpkc=dtpkc;
}


std::vector<Cipher>
EvalDiv100 ::EvalDiv_U1_step1()
{

    auto begin = chrono::high_resolution_clock::now();
    gmp_randstate_t state;
    Cipher S1, S2,S3, S4, S5, S6, S7, S8, S9;

    // Generate two random numbers : used for multiplicative cryptographic blinding

    if(!dtpkc.optim) {

        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));
        rx = mpz_class_get_z_bits(dtpkc.blindVal);
        ry = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randclear(state);
    }
    else{
        rx = dtpkc.R1;
        ry = dtpkc.R2;
    }

    S2.T1=mpz_class_powm(a.T1,rx,dtpkc.n2);
    S2.T2=mpz_class_powm(a.T2,rx,dtpkc.n2);

    S4.T1=mpz_class_powm(b.T1,rx,dtpkc.n2);
    S4.T2=mpz_class_powm(b.T2,rx,dtpkc.n2);

    S6.T1=mpz_class_powm(S4.T1,ry,dtpkc.n2);
    S6.T2=mpz_class_powm(S4.T2,ry,dtpkc.n2);
    S6.Pub=a.Pub;


    S9.T1 = S6.T1*S2.T1;
    S9.T2 = S6.T2*S2.T2;

    S5=dtpkc.PSdec0(S9);
    S7=dtpkc.PSdec0(S4);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = 4 * mpz_size(S4.T1.get_mpz_t())+ mpz_size(S4.T2.get_mpz_t())* sizeof(mp_limb_t);

    return {S9,S4,S5,S7};

}

Cipher
EvalDiv100 ::EvalDiv_U2_step2(Cipher S1,Cipher S2,Cipher S3,Cipher S4)
{

    auto begin = chrono::high_resolution_clock::now();

    mpz_class S5, S6;
    Cipher S7;

    S5=dtpkc.PSdec1(S1,S3);
    S6=dtpkc.PSdec1(S2,S4);

    S7=dtpkc.enc((10000*S5)/S6,a.Pub);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(S7.T1.get_mpz_t())+ mpz_size(S7.T2.get_mpz_t())* sizeof(mp_limb_t);


    return S7;

}



Cipher
EvalDiv100 ::EvalDiv_U1_step3(Cipher S7)
{


    auto begin = chrono::high_resolution_clock::now();

    Cipher S8, S88, S9;

    S8 = dtpkc.enc(10000*ry,a.Pub);


    S88.T1=mpz_class_powm(S8.T1,dtpkc.n-1,dtpkc.n2);
    S88.T2=mpz_class_powm(S8.T2,dtpkc.n-1,dtpkc.n2);

    S9.T1=S7.T1*S88.T1;
    S9.T2=S7.T2*S88.T2;
    S9.Pub=a.Pub;





    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(S9.T1.get_mpz_t())+ mpz_size(S9.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return S9;

}
