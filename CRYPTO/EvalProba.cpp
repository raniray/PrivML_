#include "EvalProba.h"
#include <thread>

EvalProba::EvalProba(std::vector<Cipher> Tk, DTPKC dtpkc)
{
    this->Tk=Tk;
    this->dtpkc=dtpkc;
}


std::vector <Cipher>
EvalProba::EvalProba_U1_step1()
{
    auto begin = chrono::high_resolution_clock::now();
    gmp_randstate_t state;
    Cipher S1, S2, S4, S5, S11, S22, S111, S1111,inter, inter2;
    int j = 0;

    std::vector<Cipher> S6K (Tk.size()*2);

    if(!dtpkc.optim) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));

        r = mpz_class_get_z_bits(dtpkc.blindVal);

        gmp_randclear(state);

    }else{
        r = dtpkc.R1;
    }
    for (int i=0;i<Tk.size();i++)
    {

        inter.T1=mpz_class_powm(Tk[i].T1,dtpkc.n-r,dtpkc.n2);
        inter.T2=mpz_class_powm(Tk[i].T2,dtpkc.n-r,dtpkc.n2);


        inter2.T1=mpz_class_powm(inter.T1,dtpkc.n-1,dtpkc.n2);
        inter2.T2=mpz_class_powm(inter.T2,dtpkc.n-1,dtpkc.n2);
        inter2.Pub=Tk[0].Pub;

        S6K[j]=inter2;
        inter= dtpkc.PSdec0(inter2);
        j++;
        S6K[j]=inter;
        j++;

    }



    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = S6K.size()*mpz_size(S6K[0].T1.get_mpz_t())+ mpz_size(S6K[0].T2.get_mpz_t())* sizeof(mp_limb_t);


    return S6K;

}

Cipher
EvalProba::EvalProba_U2_step2(std::vector<Cipher> S6K)
{

    auto begin = chrono::high_resolution_clock::now();
    std::vector<mpz_class> S7K (2+Tk.size());
    mpz_class inter;
    mpz_class res;
    Cipher blinded_result;

    res = dtpkc.PSdec1(S6K[0],S6K[1]);

    for (int i=2; i< S6K.size()-1; i=i+2)
    {
        // Partially decrypt Tk*r
        inter=dtpkc.PSdec1(S6K[i],S6K[i+1]);
        res = res * inter;

    }



    blinded_result = dtpkc.enc(res, S6K[0].Pub);

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(blinded_result.T1.get_mpz_t())+ mpz_size(blinded_result.T2.get_mpz_t())* sizeof(mp_limb_t);


    return blinded_result;

}


Cipher
EvalProba :: EvalProba_U1_step3(Cipher blinded_result)
{

    auto begin = chrono::high_resolution_clock::now();
    Cipher S8, S88, S9;

    mpz_class res = r;
    for (int i=0; i < Tk.size() -1; i++)
        res =res*r;

    S8 = dtpkc.enc(res,Tk[0].Pub);

    EvalDiv100 evalDiv100(blinded_result,S8, dtpkc);

    vector<Cipher> res1 = evalDiv100.EvalDiv_U1_step1();
    S9= evalDiv100.EvalDiv_U1_step3(evalDiv100.EvalDiv_U2_step2(res1[0], res1[1], res1[2], res1[3]));


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(S9.T1.get_mpz_t())+ mpz_size(S9.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return S9;

}
