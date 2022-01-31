#include "TESTElementaryOps.h"
#include "../ML/VFDT/VFDT_Classifier.h"
#include "../CRYPTO/EvalThreashold.h"
#include "../CRYPTO/EvalLrUpdate.h"
#include "../CRYPTO/EvalLog.h"
#include <chrono>
using namespace std;


void TESTElementaryOps::run (bool deserialize, int keysize, int prec, int error, string filename, bool optim ) {

    DTPKC dtpkc;
    mpz_class pkey, skey, newPkey, newSkey, r, C2;
    mpz_class A, B, C_2;
    DTPKC::Cipher C_A, C_B, C, C_1, S;
    gmp_randstate_t randstate;
    int N =1;//= 10;
    double init, encrypt, decrypt, dec1, dec2, expo, mult, gmpRandom, gmpExp, gmpSqrt, gmpLog, gmpMult, gmpDiv,gmpSub;

    /***
     * Initialisation
     */
    keysize = 1024;
    auto begin = chrono::high_resolution_clock::now();
    for(int i=0; i <N ; i++)
    {
        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));
        std::chrono::milliseconds delay(1000);
        dtpkc.keygen(prec,randstate, delay, keysize, error, optim, 200 );

        // Generate public key
        dtpkc.getKey(pkey,skey);
        dtpkc.updatePkw(pkey);
        //cout << "Generated dtpkc parameters " << endl;
    }
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    init = duration.count()/N;

    dtpkc.getKey(newPkey,newSkey);
    dtpkc.updatePkw(newPkey);

    r=mpz_class_get_z_bits(200);

    //Encryption


    A = 10740;//mpz_class_get_z_bits(15);
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    B = 10740;// mpz_class_get_z_bits(15);
    cout << "A= " << A << endl;

    // Encrypt operands
    N=1;
    begin = chrono::high_resolution_clock::now();
    for(int i=0; i <N ; i++) {
        C_A = dtpkc.enc(A, pkey);
    }
    end = chrono::high_resolution_clock::now();
    duration = end  - begin ;
    encrypt = duration.count()/N;

    cout << "CA= " << C_A << endl;

    //return;

    C_B = dtpkc.enc(B,pkey);


    C_A = dtpkc.enc(A,pkey);

    // Partial decryption part 1
N=100;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {
    C_1 = dtpkc.PSdec0(C_A);
}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
dec1 = duration.count()/N;

// Partial decryption part 2
N=100;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {
    C_2 = dtpkc.PSdec1(C_A, C_1);
}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
dec2 = duration.count()/N;

cout << C_2 << endl;
// Expo
N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {
    if(!optim) {

        fastpow2(S.T1, C_A.T1, r, dtpkc.n2);

        fastpow2(S.T2, C_A.T2, r, dtpkc.n2);

    } else{

        fastpow(S.T1, C_A.T1, r, dtpkc.n2);

        fastpow(S.T2, C_A.T2, r, dtpkc.n2);
    }
}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
expo = duration.count()/N;

    N = 1000;
if(!optim) {
// Mult

    begin = chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {


        fastmul2(C.T1, C_A.T1, C_B.T1);
        fastmul2(C.T2, C_A.T2, C_B.T2);

    }
    end = chrono::high_resolution_clock::now();
    duration = end - begin;
    mult = duration.count() / N;
}else{


    begin = chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {

            fastmul(C.T1, C_A.T1, C_B.T1);
            fastmul(C.T2, C_A.T2, C_B.T2);

    }
    end = chrono::high_resolution_clock::now();
    duration = end - begin;
    mult = duration.count() / N;


}
/*
mpz_class r1,r2;

// gmp random number generation
N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {

   r1=mpz_class_get_z_bits(200);
}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
gmpRandom = duration.count()/N;*/


/*

// gmp number expo
N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {

    mpz_class_exp(r1,1);

}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
gmpExp = duration.count()/N;



// gmp number log
N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {

    mpz_class_log(r1,1);

}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
gmpLog = duration.count()/N;



// gmp number sqrt
N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {

    mpz_class_sqrt(r1);

}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
gmpSqrt = duration.count()/N;


r2=mpz_class_get_z_bits(200);

// gmp number Mult
N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {

    r1*r2;

}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
gmpMult= duration.count()/N;*/



// gmp number div
/*N=1000;
begin = chrono::high_resolution_clock::now();
for(int i=0; i <N ; i++) {

    r1/r2;

}
end = chrono::high_resolution_clock::now();
duration = end  - begin ;
gmpDiv= duration.count()/N;*/



// gmp number div
 /*   N=10000;
    begin = chrono::high_resolution_clock::now();
    for(int i=0; i <N ; i++) {

        C_B.T1*mpz_class_powm(C_A.T1,dtpkc.n-1,dtpkc.n2);
        C_B.T2*mpz_class_powm(C_A.T2,dtpkc.n-1,dtpkc.n2);


    }
    end = chrono::high_resolution_clock::now();
    duration = end  - begin ;
    gmpSub= duration.count()/N;*/


//cout << "gmpSub = " <<  gmpSub<< endl;
//cout << "gmpDiv = " <<  gmpDiv<< endl;
//cout << "gmpMult = " <<  gmpMult<< endl;
//cout << "gmpSqrt = " <<  gmpSqrt << endl;
//cout << "gmpLog = " <<  gmpLog << endl;
//cout << "gmpExp = " <<  gmpExp << endl;
//cout << "gmpRandom = " <<  gmpRandom << endl;
//cout << "init = " <<  init << endl;
cout << "enc = " <<  encrypt << endl;
//cout << "dec = " <<  decrypt << endl;
cout << "dec1 = " <<  dec1 << endl;
cout << "dec2 = " <<  dec2 << endl;
cout << "SMult = " <<  expo << endl;
cout << "SAdd = " <<  mult << endl;

}

