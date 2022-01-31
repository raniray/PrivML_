#include<iostream>
#include "../CRYPTO/DTPKC.h"
#include "../UTIL/mpz_class.hh"
#include "../UTIL/num_th_alg.hh"
#include "TESTDTPKC.h"

using namespace std;




void TESTDTPKC::run () {

    // Generate public parameters of DTPKC

    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    DTPKC dtpkc;
    std::chrono::milliseconds delay(1000);
    dtpkc.keygen(1000, randstate, delay, 1024,600);

    cout << "Pkw=" << dtpkc.pkw.get_str() << endl;

    // Generate public key
    mpz_class pkey, skey;
    dtpkc.getKey(pkey,skey);
    dtpkc.updatePkw(pkey);

    cout << "Pkey=" << pkey.get_str() << endl;
    cout << "Skey=" << skey.get_str() << endl;
    cout << "Updated Pkw=" << dtpkc.pkw.get_str() << endl;

    // Encrypt a message

    cout<<"Enter the value to be encrypted : " << endl;
    mpz_class A;
    int a;
    cin >> a;
    A.set_str(std::to_string(a),10);
    Cipher c = dtpkc.enc(A,pkey);
    cout<<"The obtained cipher is : T1= " << c.T1.get_str() << ", T2 = " << c.T2.get_str() << endl;

    // Weak Decrypt a cipher
    mpz_class B;
    B=dtpkc.dec(c,skey);

    cout<<"The decrypted value is : B = " << B.get_str() << endl;

    // Strong decrypt of a cipher

    mpz_class C;
    C=dtpkc.Sdec(c);

    cout<<"The decrypted value is : C = " << C.get_str() << endl;

    // Strong partial decrypt of a cipher

    Cipher D;
    D=dtpkc.PSdec0(c);

    cout<<"The partially decrypted value using SK0 is : D = " << D.T1.get_str() << endl;

    mpz_class E;
    E=dtpkc.PSdec1(c,D);

    cout<<"The decrypted value using SK1 is : E = " << E.get_str() << endl;

}