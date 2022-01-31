#include "TESTpacking.h"
#include "TESTBuildingBlocks.h"
#include "../ML/VFDT/VFDT_Classifier.h"
#include "../CRYPTO/EvalThreashold.h"
#include "../CRYPTO/EvalLrUpdate.h"
#include "../CRYPTO/EvalLog.h"
using namespace std;

void TESTpacking::run(bool deserialize, int keysize, int prec, int error, string filename) {


    DTPKC dtpkc;
    int cpt=0;
    mpz_class pkey, skey, newPkey, newSkey;
    mpz_class A, B,C, T;
    DTPKC::Cipher C_A, C_B, C_C;
    gmp_randstate_t randstate;

    gmp_randclass rr(gmp_randinit_default);
    rr.seed(time(NULL));


    int cptF = 0;

    if (deserialize)
    {
        dtpkc.deserializeDtpkc(filename);
        dtpkc.blindVal = prec;
        pkey = dtpkc.pkw;
        //cout << "Deserialized dtpkc " << endl;

    }else
    {

        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate,time(NULL));
        std::chrono::milliseconds delay(1000);
        dtpkc.keygen(prec,randstate, delay, keysize, error);

        // Generate public key
        dtpkc.getKey(pkey,skey);
        dtpkc.updatePkw(pkey);
        cout << "Generated dtpkc parameters " << endl;
    }

    dtpkc.getKey(newPkey,newSkey);
    dtpkc.updatePkw(newPkey);

    // Read operands

    A = rr.get_z_bits(10);
    B = rr.get_z_bits(8);
    C = rr.get_z_bits(9);


    cout<<"A= " << A << endl;
    cout<<"B= " << B << endl;
    cout<<"C= " << C << endl;

    vector<string> vect;

    vect.push_back(A.get_str(10));

    vect.push_back(B.get_str(10));

    vect.push_back(C.get_str(10));


    vector<string> ones;

    ones.push_back("1");

    ones.push_back("1");

    ones.push_back("1");


    Cipher res = dtpkc.packEnc(vect,5,5, pkey);

    Cipher C_ones = dtpkc.packEnc(ones,5,5, pkey);

    C =  dtpkc.Sdec(res);

    cout<<"Res= " << res << endl;

    cout<<"ones= " << C_ones << endl;

    res.T1 = res.T1 * C_ones.T1;

    res.T2 = res.T2 * C_ones.T2;


    cout<<"Res add =  " << res << endl;

    mpz_class r = mpz_class_get_z_bits(10);

    Cipher C_R = dtpkc.enc(r,pkey);

    res.T1 = mpz_class_powm(res.T1, r, dtpkc.n2);
    res.T2 = mpz_class_powm(res.T2, r, dtpkc.n2);


    std::vector<mpz_class> C_z = dtpkc.Unpackdec(res,5,5);

    for (mpz_class i : C_z)
        cout << i/r << endl;



    if(!deserialize) gmp_randclear(randstate);

}
