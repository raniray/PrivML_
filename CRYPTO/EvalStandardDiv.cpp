
#include <thread>
#include "EvalStandardDiv.h"


EvalStandardDiv::EvalStandardDiv(Cipher a, Cipher b, Cipher c, DTPKC dtpkc)
{
    this->A=a;
    this->B=b;
    this->C=c;
    this->dtpkc=dtpkc;
}


std::vector<Cipher> EvalStandardDiv::EvalStandardDiv_U1_step1()
{
    auto begin = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    Cipher S1, S2,S3, o1, o2, o;

    // o1: A/B
    EvalDiv evalDiv(A,B,dtpkc);
    vector<Cipher> res = evalDiv.EvalDiv_U1_step1();
    o1 = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0],res[1],res[2],res[3]));

    // C*C
    EvalMult evalMult(C,C,dtpkc);
    o2 =evalMult.EvalMult_U1_step3(evalMult.EvalMult_U2_step2(evalMult.EvalMult_U1_step1()));

    // o1 - o2
    EvalSub evalSub(o1,o2,dtpkc);
    o = evalSub.EvalSub_U1();

    if(!dtpkc.optim) {
        // Generate cryptographic blinding value
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));
        r = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randclear(state);
    }else{
        r = dtpkc.R1;
    }

    // compute r *(o1 -o2)
    S1.T1=mpz_class_powm(o.T1,r,dtpkc.n2);
    S1.T2=mpz_class_powm(o.T2,r,dtpkc.n2);

    // Partially decrypt r *(o1 -o2)
    S3=dtpkc.PSdec0(S1);

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = 2*mpz_size(S1.T1.get_mpz_t())+ mpz_size(S1.T2.get_mpz_t())* sizeof(mp_limb_t);


    return {S1,S3};

}

Cipher
EvalStandardDiv::EvalStandardDiv_U2_step2(std::vector<Cipher> res1)
{

    auto begin = chrono::high_resolution_clock::now();

    mpz_class x;
    Cipher out;

    x=dtpkc.PSdec1(res1[0],res1[1]);

    out  = dtpkc.enc(mpz_class_sqrt(x),A.Pub);

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(out.T1.get_mpz_t())+ mpz_size(out.T2.get_mpz_t())* sizeof(mp_limb_t);

    return out;

}


Cipher
EvalStandardDiv::EvalStandardDiv_U1_step3(Cipher res2)
{
    auto begin = chrono::high_resolution_clock::now();
    Cipher blindVal;
    // compute sqrt of r
    blindVal = dtpkc.enc(mpz_class_sqrt(r), A.Pub);

    // remove blinding value
    EvalDiv evalDiv(res2,blindVal,dtpkc);
    std::vector <Cipher> res = evalDiv.EvalDiv_U1_step1();
    Cipher out = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0],res[1], res[2], res[3]));


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(out.T1.get_mpz_t())+ mpz_size(out.T2.get_mpz_t())* sizeof(mp_limb_t);


    std::this_thread::sleep_for(dtpkc.delay);

    return  out;

}
