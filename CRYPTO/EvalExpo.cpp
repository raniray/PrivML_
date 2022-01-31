#include "EvalExpo.h"
#include "EvalDiv.h"
#include "EvalDiv100.h"
#include <thread>

EvalExpo::EvalExpo(Cipher a, int lambda, DTPKC dtpkc)
{
    this->u=a;
    this->lambda=lambda;
    this->dtpkc=dtpkc;
}


std::vector<Cipher>
EvalExpo::EvalSig_U1_step1()
{
    auto begin  = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    Cipher S1, S2;

    if(!dtpkc.optim) {
    gmp_randinit_default(state);
    gmp_randseed_ui(state,time(NULL));
    r=mpz_class_get_z_bits(300);
    gmp_randclear(state);

    }
    else{

        r = dtpkc.R1;

    }

    Cipher C_R = dtpkc.enc(mpz_class_log(r,1),u.Pub);

    //cout << "C_R" << C_R << endl;

    S1.T1=C_R.T1*u.T1;
    S1.T2=C_R.T2*u.T2;
    S1.Pub=u.Pub;
    S1.sign = u.sign;

    S2=dtpkc.PSdec0(S1);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = 2*mpz_size(S1.T1.get_mpz_t())+ mpz_size(S1.T2.get_mpz_t())* sizeof(mp_limb_t);


    return {S1,S2};

}

Cipher
EvalExpo::EvalSig_U2_step2(Cipher S1, Cipher S2)
{
    auto begin  = chrono::high_resolution_clock::now();

    mpz_class S3;
    Cipher S4;
    S3=dtpkc.PSdec1(S1,S2);


    S4=dtpkc.enc(mpz_class_exp(S3,lambda/100),u.Pub);

    //cout << "u " << u << endl;

    //cout << "S3 " << S3  << endl;

    //cout << "S4 " << S4  << endl;

    if (S1.sign == 1){

        S4.sign = 1;

    }else{

        S4.sign = 0;

    }


    auto en = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(S4.T1.get_mpz_t())+ mpz_size(S4.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return S4;

}



Cipher
EvalExpo::EvalSig_U1_step3(Cipher S4)
{

    auto begin  = chrono::high_resolution_clock::now();

    Cipher S5, S6, S7, S9, S8;

    S5 = dtpkc.enc(mpz_class_exp(mpz_class_log(r,1),1), u.Pub);

    EvalDiv100 evalDiv(S4,S5,dtpkc);
    std::vector <Cipher> res = evalDiv.EvalDiv_U1_step1();
    S6 = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0],res[1], res[2], res[3]));


    S6.sign = 1;


    auto en = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(S4.T1.get_mpz_t())+ mpz_size(S4.T2.get_mpz_t())* sizeof(mp_limb_t);


    return S6;

}
