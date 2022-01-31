
#include <thread>
#include "EvalAddPow2.h"



EvalAddPow2 ::EvalAddPow2(Cipher A, Cipher B , DTPKC dtpkc)
{

    this->A=A;
    this->B=B;
    this->dtpkc=dtpkc;
}


std::vector<Cipher>
EvalAddPow2::EvalAddPow2_U1_step1()
{

    /**
     * state : seed used to generate the blinding value
     * S1 : encrypted value of r
     * S2 : B + r
     * S3 : partially decrypted value of S2
     * res : results to be sent to U2
     */
    auto begin = chrono::high_resolution_clock::now();
    gmp_randstate_t state;
    Cipher S1, S2, S3;
    std::vector <Cipher> res(4);



    if(!dtpkc.optim) {
        r = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randinit_default(state);
        gmp_randseed_ui(state,time(NULL));
        gmp_randclear(state);
        S1=dtpkc.enc(r,A.Pub);
    } else {
        r = dtpkc.R1;
        S1=dtpkc.CR1;
    }




    // B + r
    S2.T1= S1.T1*B.T1;
    S2.T2= S1.T2*B.T2;
    S2.Pub= B.Pub;


    S3=dtpkc.PSdec0(S2);

    res[0]=S2;
    res[1]=S3;



    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU= duration.count();
    bdwMU = mpz_size(S2.T1.get_mpz_t())+ mpz_size(S2.T2.get_mpz_t())* sizeof(mp_limb_t)+ mpz_size(S3.T1.get_mpz_t())+ mpz_size(S3.T2.get_mpz_t())* sizeof(mp_limb_t);

    return res;


}


Cipher EvalAddPow2::EvalAddPow2_U2_step2(std::vector <Cipher> res1)
{

    auto begin = chrono::high_resolution_clock::now();

    mpz_class x;
    Cipher res;

    x=dtpkc.PSdec1(res1[0],res1[1]);

    res  =  dtpkc.enc(x*x,A.Pub);



    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(res.T1.get_mpz_t())+ mpz_size(res.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return res;


}

Cipher EvalAddPow2::EvalAddPow2_U1_step3(Cipher res2)
{
    auto begin = chrono::high_resolution_clock::now();

    Cipher S4, S5, S6, S7,C_R;
    S4.T1 = A.T1 * res2.T1;
    S4.T2 = A.T2 * res2.T2;

        if(!dtpkc.optim)
            C_R =  dtpkc.enc(r, A.Pub);
        else
            C_R = dtpkc.CR1;

        S5.T1=mpz_class_powm( C_R.T1,dtpkc.n-r,dtpkc.n2);
        S5.T2=mpz_class_powm( C_R.T2,dtpkc.n-r,dtpkc.n2);
        S5.Pub = A.Pub;


        S6.T1=mpz_class_powm( B.T1,dtpkc.n-(2*r),dtpkc.n2);
        S6.T2=mpz_class_powm( B.T2,dtpkc.n-(2*r),dtpkc.n2);
        S6.Pub = B.Pub;

        // A + (B+r)^2 - r^2 -2*B*r = A + B ^2
        S7.T1=S4.T1*S5.T1*S6.T1;
        S7.T2=S4.T2*S5.T2*S6.T2;
        S7.Pub=A.Pub;

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(S7.T1.get_mpz_t())+ mpz_size(S7.T2.get_mpz_t())* sizeof(mp_limb_t);

    return S7;

}
