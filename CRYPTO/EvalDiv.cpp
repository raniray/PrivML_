/**
** Class :  EvalDiv
 * Authors : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 28/11/2018
** Purpose : Compute the division result of two integers (ciphers)
 *
**/


#include <thread>
#include "EvalDiv.h"

/**
** Function : EvalDiv
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 05/11/2018
 *
** Purpose : This is the constructor of the class, it is used to initialise the operands
 *
 * Parameters :
 * a : first operand ( Cipher)
 * b : second operand (Cipher)
 * dtpkc : used to get the DTPKC parameters
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

EvalDiv ::EvalDiv(Cipher a, Cipher b, DTPKC dtpkc)
{
    this->a=a;
    this->b=b;
    this->dtpkc=dtpkc;
}

/**
** Function : EvalDiv_U1-step1
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 28/11/2018
 *
** Purpose : The first step of the secure division protocol : This part of the code is run by U1
 *
 * Parameters :
 * none
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

std::vector<Cipher>
EvalDiv ::EvalDiv_U1_step1()
{
   /**
    * state : seed used to generate blinding values
    * S1 : rx a
    * S2 : -rx a
    * S3 : rx b
    * S4 : -rx b
    * S5 : Partially decrypted value of S9
    * S6 : - rx * ry *b
    * S7 : Partially decrypted value of S3
    * S8 : rx * ry *b
    * S9 : rx*a+rx*ry*b
    */

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

    S1.T1=mpz_class_powm(a.T1,rx,dtpkc.n2);
    S1.T2=mpz_class_powm(a.T2,rx,dtpkc.n2);


    S2.T1=mpz_class_powm(b.T1,rx,dtpkc.n2);
    S2.T2=mpz_class_powm(b.T2,rx,dtpkc.n2);


    S3.T1=mpz_class_powm(S2.T1,ry,dtpkc.n2);
    S3.T2=mpz_class_powm(S2.T2,ry,dtpkc.n2);
    S3.Pub=a.Pub;


    // rx*a+rx*ry*b
    S4.T1 = S3.T1*S1.T1;
    S4.T2 = S3.T2*S1.T2;

    // Partially decrypt S1 and S2 before sending them to U2
    S5=dtpkc.PSdec0(S4);
    S6=dtpkc.PSdec0(S2);

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();
    bdwMU = 4 * mpz_size(S4.T1.get_mpz_t())+ mpz_size(S4.T2.get_mpz_t())* sizeof(mp_limb_t);


    return {S4,S2,S5,S6};

}

/**
** Function : EvalDiv_U2-step2
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 05/11/2018
 *
** Purpose : The second step of the secure division protocol : This part of the code is run by U2
 *
 * Parameters :
 *
 * S1 : blinded value of [rx*a+rx*ry*b]
 * S2 : blinded value of [rx*b]
 * S3 : partially decrypted value of S1 using SK0
 * S4 : partially decrypted value of S2 using SK1
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

Cipher
EvalDiv ::EvalDiv_U2_step2(Cipher S1,Cipher S2,Cipher S3,Cipher S4)
{
    // S5 : partially decrypted value of S3 using SK1
    // S6 : partially decrypted value of S4 using SK1
    // S7 : The encrypted value of (a/b + ry)

    auto begin = chrono::high_resolution_clock::now();
    mpz_class S5, S6;
    Cipher S7;

    // Partially decrypt S1 using SK1
    S5=dtpkc.PSdec1(S1,S3);
    S6=dtpkc.PSdec1(S2,S4);

    // Compute a/b + ry
    S7=dtpkc.enc(S5/S6,a.Pub);


    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeSU = duration.count();
    bdwSU = mpz_size(S7.T1.get_mpz_t())+ mpz_size(S7.T2.get_mpz_t())* sizeof(mp_limb_t);

    return S7;

}

/**
** Function : EvalSqrt_U1-step3
 * Author : Talbi Rania
** Creation date : 05/11/2018
** Last Update : 05/11/2018
 *
** Purpose : The third step of the secure division protocol : This part of the code is run by U1
 *
 * Parameters :
 * S7 : [a/b+ry]pk
 *
 * Unit Test : (TEST/TestBuildingBlocks)
 *
**/

Cipher
EvalDiv ::EvalDiv_U1_step3(Cipher S7)
{


    auto begin = chrono::high_resolution_clock::now();
    Cipher S8, S88, S9;

    // encrypt ry
    if(!dtpkc.optim) {
        S8 = dtpkc.enc(ry,a.Pub);
    }else{
        S8 = dtpkc.CR2;
    }

    // compute -ry
    S88.T1=mpz_class_powm(S8.T1,dtpkc.n-1,dtpkc.n2);
    S88.T2=mpz_class_powm(S8.T2,dtpkc.n-1,dtpkc.n2);

    // Compute [a/b+ry-ry]
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
