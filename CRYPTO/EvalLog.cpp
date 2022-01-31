#include <thread>
#include "EvalLog.h"
#include "EvalDiv.h"
#include "EvalDiv100.h"


EvalLog::EvalLog(Cipher a, int lambda, DTPKC dtpkc)
{
    this->u=a;
    this->lambda=lambda;
    this->dtpkc=dtpkc;
}


std::vector<Cipher>
EvalLog::EvalLog_U1_step1()
{

    auto begin = chrono::high_resolution_clock::now();
    gmp_randstate_t state;
    Cipher S1, S2;

    Cipher one = dtpkc.enc(10000, u.Pub);

    u.T1 = u.T1 * one.T1;
    u.T2 = u.T2 * one.T2;

    //cout << u << endl;
    if(!dtpkc.optim) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));

        r = mpz_class_get_z_bits(200);

        gmp_randclear(state);
    }else{
        r = dtpkc.R1;
    }

    S1.T1= mpz_class_powm(u.T1,r,dtpkc.n2);
    S1.T2= mpz_class_powm(u.T2,r,dtpkc.n2);
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
EvalLog::EvalLog_U2_step2(Cipher S1, Cipher S2)
{

    auto begin = chrono::high_resolution_clock::now();

    mpz_class S3;
    Cipher S4;
    S3=dtpkc.PSdec1(S1,S2);
    S4=dtpkc.enc(mpz_class_log(S3,lambda),u.Pub);
    S4.sign = 1;

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(S4.T1.get_mpz_t())+ mpz_size(S4.T2.get_mpz_t())* sizeof(mp_limb_t);


    return S4;

}



Cipher
EvalLog::EvalLog_U1_step3(Cipher S4)
{


    auto begin = chrono::high_resolution_clock::now();

    Cipher c, R;
    R=dtpkc.enc(mpz_class_log(r,lambda),u.Pub);

    c.T1=S4.T1*mpz_class_powm(R.T1,dtpkc.n-1,dtpkc.n2);
    c.T2=S4.T2*mpz_class_powm(R.T2,dtpkc.n-1,dtpkc.n2);
    c.Pub=u.Pub;



    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(S4.T1.get_mpz_t())+ mpz_size(S4.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return c;

}
