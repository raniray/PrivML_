#include <thread>
#include "EvalDotProd.h"
#include "EvalDiv.h"


EvalDotProd::EvalDotProd(std::vector<Cipher> a, std::vector<Cipher> b, int size,
                           DTPKC dtpkc)
{
    this->a=a;
    this->b=b;
    this->size=size;
    this->dtpkc=dtpkc;
}


std::vector<Cipher> EvalDotProd::EvalDotProd_U1_step1()
{
    auto begin = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    std::vector<Cipher> res;

    if(!dtpkc.optim) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state, time(NULL));

        r1 = mpz_class_get_z_bits(dtpkc.blindVal);
        r2 = mpz_class_get_z_bits(dtpkc.blindVal);

        gmp_randclear(state);
    }else{


        r1 = dtpkc.R1;
        r2 = dtpkc.R2;

    }

    for (int i=0; i< size ; i++)
    {
        Cipher inter, inter2;

        inter.T1 = mpz_class_powm(a[i].T1,r1,dtpkc.n2);
        inter.T2 = mpz_class_powm(a[i].T2,r1,dtpkc.n2);
        inter.Pub= a[i].Pub;
        inter.sign = a[i].sign;

        inter2.T1 = mpz_class_powm(b[i].T1,r2,dtpkc.n2);
        inter2.T2 = mpz_class_powm(b[i].T2,r2,dtpkc.n2);
        inter2.Pub= b[i].Pub;
        inter2.sign = b[i].sign;

        res.push_back(inter);
        res.push_back(dtpkc.PSdec0(inter));

        res.push_back(inter2);
        res.push_back(dtpkc.PSdec0(inter2));

    }

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = res.size() * mpz_size(res[0].T1.get_mpz_t())+ mpz_size(res[0].T2.get_mpz_t())* sizeof(mp_limb_t);

    return res;


}

Cipher EvalDotProd::EvalDotProd_U2_step2(std::vector<Cipher> val)
{

    auto begin = chrono::high_resolution_clock::now();

    mpz_class res;
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state,time(NULL));

    if(!dtpkc.optim) {
        r3 = mpz_class_get_z_bits(dtpkc.blindVal);
    }
    else{
        r3 = dtpkc.R3;
    }

    gmp_randclear(state);

    //#pragma omp parallel for shared (val, res, r3) schedule(static)
    for (int i=0; i<val.size(); i+=4)
    {
        mpz_class inter  = dtpkc.PSdec1(val[i], val[i+1]) * dtpkc.PSdec1(val[i+2], val[i+3]);

        if ((val[i].sign == 1 && val[i + 2].sign == 0) || (val[i].sign == 0 && val[i + 2].sign == 1)) {

            inter = -inter;
        }

        //#pragma omp critical
        {
            res = res + inter/r3;
        };
    }

    Cipher out;


    if (res < 0) {
        out = dtpkc.enc(-res, a[0].Pub);
        out.sign = 0;
    } else {
        out = dtpkc.enc(res, a[0].Pub);
        out.sign = 1;
    }


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(out.T1.get_mpz_t())+ mpz_size ( out.T2.get_mpz_t())* sizeof(mp_limb_t);


    return out;

}


Cipher EvalDotProd::EvalDotProd_U1_step3(Cipher val)
{

    auto begin = chrono::high_resolution_clock::now();

    Cipher S1;

    S1 = dtpkc.enc(r1*r2/r3,a[0].Pub);

    EvalDiv evalDiv(val,S1,dtpkc);
    std::vector <Cipher> res = evalDiv.EvalDiv_U1_step1();
    Cipher out = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0],res[1], res[2], res[3]));
    out.sign = val.sign;


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(out.T1.get_mpz_t())+ mpz_size ( out.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return out;

}
