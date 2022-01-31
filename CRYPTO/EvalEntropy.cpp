/**
** Class :  EvalEntropy
 * Authors : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
** Purpose : Compute the approximate value of entropy
 *
 * Remark : Pecision * 10000
 *
**/

#include <thread>
#include "EvalEntropy.h"



/**
** Function : EvalEntropy
 * Author : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
 *
** Purpose : This is the constructor of the class, it is used to initialise the operands
 *
 * Parameters :
 * T : entire set size (cipher)
 * Tk : subsets sizes (vector of ciphers)
 * dtpkc : used to get the DTPKC parameters
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

EvalEntropy ::EvalEntropy(Cipher T, std::vector<Cipher> Tk, DTPKC dtpkc)
{
    this->T=T;
    this->Tk=Tk;
    this->dtpkc=dtpkc;
    this->prec=DTPKC::precision;
}

/**
** Function : EvalEntropy_U1-step1
 * Author : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
 *
** Purpose : The first step of the secure entropy protocol : This part of the code is run by U1
 *
 * Parameters :
 * none
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

std::vector < Cipher>
EvalEntropy::EvalEntropy_U1_step1()
{

    auto begin = chrono::high_resolution_clock::now();

    gmp_randstate_t state;
    Cipher S1, S2, S4, S5, S11, S22, S111, S1111,inter, inter2;
    int j = 0;

    std::vector<Cipher> S6K (4+Tk.size()*2);


    // Generate two random numbers : used for multiplicative cryptographic blinding


    if(!dtpkc.optim) {
        gmp_randinit_default(state);
        gmp_randseed_ui(state,time(NULL));
        rx = mpz_class_get_z_bits(dtpkc.blindVal);
        ry = mpz_class_get_z_bits(dtpkc.blindVal);
        gmp_randclear(state);
    }
    else{
        rx = dtpkc.R1;
        ry = dtpkc.R2;
    }



    // -rx*T
    S1.T1=mpz_class_powm(T.T1,rx,dtpkc.n2);
    S1.T2=mpz_class_powm(T.T2,rx,dtpkc.n2);

    // -ry*rx*T
    S1111.T1=mpz_class_powm(S1.T1,ry,dtpkc.n2);
    S1111.T2=mpz_class_powm(S1.T2,ry,dtpkc.n2);


    // -rx*T
    S22.T1=mpz_class_powm(T.T1,rx,dtpkc.n2);
    S22.T2=mpz_class_powm(T.T2,rx,dtpkc.n2);



    for (int i=0;i<Tk.size();i++)
    {

        // -rx*Tk
        inter.T1=mpz_class_powm(Tk[i].T1,rx,dtpkc.n2);
        inter.T2=mpz_class_powm(Tk[i].T2,rx,dtpkc.n2);

        S6K[j]=inter;

        // Partially decrypt rx*Tk
        inter= dtpkc.PSdec0(inter);
        j++;
        S6K[j]=inter;
        j++;

    }

    // Partially decrypt rx*ry T and rx*T
    S4=dtpkc.PSdec0(S1111);
    S5=dtpkc.PSdec0(S22);

    S6K[j]=S1111;
    j++;

    S6K[j]=S4;
    j++;

    S6K[j]=S22;
    j++;

    S6K[j]=S5;
    j++;




    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = S6K.size() * mpz_size( S6K[0].T1.get_mpz_t())+ mpz_size(S6K[0].T2.get_mpz_t())* sizeof(mp_limb_t);

    return S6K;

}


/**
** Function : EvalEntropy_U2-step2
 * Author : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
 *
** Purpose : The second step of the secure entropy protocol : This part of the code is run by U2
 *
 * Parameters :
 * S6K : results of the previous step sent by U1
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/


Cipher

EvalEntropy::EvalEntropy_U2_step2(std::vector<Cipher> S6K)
{

    auto begin = chrono::high_resolution_clock::now();

    int i=0, j=0 ;
    std::vector<mpz_class> S7K (2+Tk.size());
    mpz_class inter;
    mpz_class sumOfLogs;
    mpz_class logrxryS;
    Cipher blinded_result;

    for (int i=0; i< S6K.size() - 1;i=i+2)
    {
        // Partially decrypt Tk*r
        inter=dtpkc.PSdec1(S6K[i],S6K[i+1]);
        S7K[j]=inter;
        j++;

        if ( i < S6K.size()-4 )
        {
            // sum of Tk*rlog2(rTk)
            //mpz_class_log2_10(inter) returns log2 (rTk) * 100
            sumOfLogs = sumOfLogs + inter * mpz_class_log2_10(inter);
        }

    }


    // Compute sum of logs / rT
    sumOfLogs = sumOfLogs * prec;
    sumOfLogs = sumOfLogs /  S7K[S7K.size()-1];

    // Compyte log(rx)+log(ry)+log(T) - sum(Tklog(Tk))/T + log(rx))
    logrxryS = mpz_class_log2_10 (S7K[S7K.size()-2]) * prec;

    // Entropy * 10000 + log(ry)
    blinded_result = dtpkc.enc(logrxryS - sumOfLogs, T.Pub);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(blinded_result.T1.get_mpz_t())+ mpz_size(blinded_result.T2.get_mpz_t())* sizeof(mp_limb_t);


    return blinded_result;

}


/**
** Function : EvalEntropy_U1_step3
 * Author : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
 *
** Purpose : The third step of the secure entropy protocol : This part of the code is run by U1
 *
 * Parameters :
 * blinded result : results of the previous step sent by U2  : Entropy * 10000 + log(ry)
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

Cipher
EvalEntropy ::EvalEntropy_U1_step3(Cipher blinded_result)
{

    auto begin = chrono::high_resolution_clock::now();

    Cipher S8, S88, S9;

    // encrypt log(ry)
    S8 = dtpkc.enc(prec* mpz_class_log2_10 (ry),T.Pub);

    // compute -log(ry)
    S88.T1=mpz_class_powm(S8.T1,dtpkc.n-1,dtpkc.n2);
    S88.T2=mpz_class_powm(S8.T2,dtpkc.n-1,dtpkc.n2);

    // remove bliding value log(ry)
    S9.T1=blinded_result.T1*S88.T1;
    S9.T2=blinded_result.T2*S88.T2;
    S9.Pub=T.Pub;





    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU += duration.count();
    bdwMU += mpz_size(S9.T1.get_mpz_t())+ mpz_size(S9.T2.get_mpz_t())* sizeof(mp_limb_t);

    std::this_thread::sleep_for(dtpkc.delay);

    return S9;

}
