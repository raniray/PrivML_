#include "TESTBuildingBlocks.h"
#include "../ML/VFDT/VFDT_Classifier.h"
#include "../CRYPTO/EvalThreashold.h"
#include "../CRYPTO/EvalLrUpdate.h"
#include "../CRYPTO/EvalLog.h"
#include "../CRYPTO/EvalSqrt.h"
#include "../CRYPTO/EvalExpo.h"

using namespace std;

int testBdwBGV()
{

}

int TESTBuildingBlocks::validate(DTPKC dtpkc)
{

    int cpt=0;
    mpz_class pkey, skey, newPkey, newSkey;
    mpz_class A, B,C, T;
    DTPKC::Cipher C_A, C_B, C_C;
    int cptF = 0;
    pkey = dtpkc.pkw;
    dtpkc.getKey(newPkey,newSkey);
    dtpkc.updatePkw(newPkey);

    // Read operands

    B = mpz_class_get_z_bits(15);
    //cout << A << endl;
    A = B + mpz_class_get_z_bits(15);


    // Encrypt operands
    C_A = dtpkc.enc(A,pkey);
    C_B = dtpkc.enc(B,pkey);

    Cipher C_T;

    T.set_str("1000",10);
    C_T = dtpkc.enc(T,pkey);

    int size = 5;
    int t = 1000;
    vector<mpz_class> INTER (size);
    vector<Cipher> classses (size);
    vector<int> inter (size);


    for(int g=0;g<5;g++)
    {
        inter[g]=200;
        INTER[g].set_str("200",10);
        classses[g] = dtpkc.enc(INTER[g],pkey);
    }



/**
* Test SDec
 *
*/

    cpt++;

    mpz_class c_a = dtpkc.Sdec(C_A);
    const char* state = (c_a == A) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") : <<SDec> gave : " << c_a << " state :" << state << endl;
    if (c_a != A) cptF++;



/**
* Test Comparion
*/
    cpt ++;

    EvalCmp evalCmp(C_A,C_B,dtpkc);
    bool r = evalCmp.EvalCmp_U2_step2(evalCmp.EvalCmp_U1_step1());


    state = (A<B == r) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") : <<comparison>> gave : " << r << " state :" << state << endl;
    if (A<B != r) cptF++;

/**
* Test EvalHBound
*/

    cpt ++;
    mpz_class i = 10000;
    i = i * pow(log2(3) * 100,2) * log(1/0.00001);
    EvalHBound evalHBound(dtpkc.enc(i,pkey),dtpkc.enc(1000,pkey),dtpkc);
    C_C = evalHBound.EvalHbound_U1_step3(evalHBound.EvalHBound_U2_step2(evalHBound.EvalHBound_U1_step1()));
    C= dtpkc.Sdec(C_C);
    int hbound = (VFDT_Classifier::HBound(log2(3),log(1/0.00001),1000) * 10000 ) ;

    state = (abs(int(hbound - C.get_ui())) < 300) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<HBound>> gave : " << C.get_str() << " expected result : " << hbound << " state :" << state <<   endl;
    if (abs(int(hbound - C.get_ui())) > 300) cptF++;


/**
* Test EvalSqrt
*/



    ///state = (abs(int(hbound - C.get_ui())) < 300) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<HBound>> gave : " << C.get_str() << " expected result : " << hbound << " state :" << state <<   endl;
    //if (abs(int(hbound - C.get_ui())) > 300) cptF++;


/**
* Test EvalAdd
*/
    cpt++;

    EvalAdd add(C_A,C_B);
    C_C=add.EvalAdd_U1();
    C=dtpkc.Sdec(C_C);

    state = (A+B == C) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<addition>> gave : " << C.get_str() << " state :" << state << endl;
    if (A+B != C) cptF++;

/**
* Test EvalSub
*/
    cpt ++;
    // Run EvalSub
    EvalSub sub(C_A,C_B,dtpkc);
    C_C=sub.EvalSub_U1();
    C=dtpkc.Sdec(C_C);

    state = (A-B == C) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<subtraction>> gave : " << C.get_str() << " state :" << state << endl;
    if (A-B != C) cptF++;

/**
* Test EvalDiv
*/
    cpt ++;
    // Run EvalDiv
    EvalDiv div(C_A,C_B,dtpkc);
    std::vector <DTPKC::Cipher> res = div.EvalDiv_U1_step1();
    C_C=div.EvalDiv_U1_step3(div.EvalDiv_U2_step2(res[0],res[1],res[2],res[3]));
    C=dtpkc.Sdec(C_C);

    state = (abs (A/B - C.get_ui()) < 300) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<division>> gave : " << C.get_str() << " expected rusult " << A/B << " state :" << state << endl;
    if (abs (A/B - C.get_ui()) > 300) cptF++;

/**
* Test logarithm on big numbers
*/

    cpt++;
    mpz_class ress = mpz_class_log2_10(A);
    state = (abs(trunc(log2(A.get_ui())*100) - ress) < 100) ? " passed":" failed";

    //cout<<"Test(" << cpt << ") <<Logarithme>> gave : " <<  ress << " expected result : " << log2(A.get_ui())*100 << " state : " << state << endl;
    if (abs(trunc(log2(A.get_ui())*100) - ress) > 100) cptF++;
/**
* Test Secure entropy
*/

    // Read operands

    cpt ++;
    EvalEntropy evalEntropy(C_T,classses,dtpkc);
    C_C= evalEntropy.EvalEntropy_U1_step3(evalEntropy.EvalEntropy_U2_step2(evalEntropy.EvalEntropy_U1_step1()));
    C=dtpkc.Sdec(C_C);

    double result = VFDT_Classifier::Entropy(t,inter)*10000;
    state = ( result - C < 300 ) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<Entropy>> gave : " << C.get_str()  << " exepcted result = "<<result << " state : " << state << endl;
    if (result - C > 300) cptF++;

/**
* Test SkeySwitch
*/
    cpt++;


    SkeySwitch skeySwitch (C_A,newPkey, dtpkc);
    C_C = skeySwitch.SkeySwitch_U1_step3(skeySwitch.SkeySwitch_U2_step2(skeySwitch.SkeySwitch_U1_step1()));
    C = dtpkc.Sdec(C_C);

    state = ( C == A ) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<Key Switch>> gave : " << C.get_str() << " state : " << state << endl;
    if ( C != A ) cptF++;

/**
* Test EvalMult
*/
    cpt++;
    EvalMult e1(C_A, C_B, dtpkc);
    C_C = e1.EvalMult_U1_step3(e1.EvalMult_U2_step2(e1.EvalMult_U1_step1()));

    C = dtpkc.Sdec(C_C);

    state = ( C == A*B) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<Multplication>> gave : " << C.get_str() << " state : " << state << endl;
    if ( C != A*B) cptF++;


/**
* Test EvalAddPow2
*/
    cpt++;

    EvalAddPow2 evalAddPow2(C_A,C_B,dtpkc);
    C_C = evalAddPow2.EvalAddPow2_U1_step3(evalAddPow2.EvalAddPow2_U2_step2(evalAddPow2.EvalAddPow2_U1_step1()));

    C = dtpkc.Sdec(C_C);
    mpz_class inter2 =  A+mpz_class_powm(B,2,1000000000000000);
    state = ( C == inter2 ) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<AddPowerOf2>> gave : " << C.get_str()  <<" expected result : " <<  inter2.get_str( 10 )<< " state : " << state <<  endl;
    if (  C != inter2 ) cptF++;

/**
* Test EvalTestStandardDiv
*/
    cpt++;
    mpz_class i1, i2, i3;
    i1.set_str("100000000",10);
    i2.set_str("100",10);
    i3.set_str("200",10);
    EvalStandardDiv evalStadardDiv(dtpkc.enc(i1,pkey),dtpkc.enc(i2,pkey),dtpkc.enc(i3,pkey), dtpkc);
    C_C = evalStadardDiv.EvalStandardDiv_U1_step3(evalStadardDiv.EvalStandardDiv_U2_step2(evalStadardDiv.EvalStandardDiv_U1_step1()));

    C = dtpkc.Sdec(C_C);
    int l = sqrt(1000000-pow(200,2));
    state = ( C == l) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<standard div>> gave : " << C.get_str() <<" expected result = " << l << " state : " << state <<  endl;
    if (C != l) cptF++;

/**
* Test EvalThreashold
**/


    cpt++;

    i1.set_str("1000",10);
    i2.set_str("280",10);
    double x = 0.7;
    EvalThreashold evalThreashold(dtpkc.enc(i1,pkey),dtpkc.enc(i2,pkey),x, dtpkc);
    C_C= evalThreashold.eval();

    C = dtpkc.Sdec(C_C);
    double m = (1000+280*NormalCDFInverse(x))*100;
    state = ( C-m<300 )  ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<EvalTh>> gave : " << C.get_str() << " expected result =" << m << " state : " << state << endl;
    if ( C-m>300 ) cptF++;


    /**
* Test EvalDiv100
*/
    cpt ++;
    EvalDiv100 div2(C_A,C_B,dtpkc);
    std::vector <DTPKC::Cipher> res3 = div2.EvalDiv_U1_step1();
    C_C=div2.EvalDiv_U1_step3(div2.EvalDiv_U2_step2(res3[0],res3[1],res3[2],res3[3]));
    C=dtpkc.Sdec(C_C);

    state = (100*A/B == C) ? " passed":" failed";
    //cout<<"Test(" << cpt << ") <<EvalDiv100> gave : " << C.get_str() << "expected result " << 100*A/B << " state :" << state << endl;
    if ( 100*A/B != C ) cptF++;


/**
 * Summary
 */

    //cout <<"***************************" << endl;
    //cout << "Total tests run : " << cpt << " Failed : " << cptF << endl;
    //cout <<"***************************" << endl;


    return cptF;
}


void TESTBuildingBlocks::run (bool deserialize, int keysize, int prec, int error, string filename, bool optim, string outputfile) {

    DTPKC dtpkc;
    int cpt=0;
    mpz_class pkey, skey, newPkey, newSkey;
    mpz_class A, B,C, T;
    DTPKC::Cipher C_A, C_B, C_C;
    vector<std::chrono::high_resolution_clock::time_point> saveTimeStart;
    vector<std::chrono::high_resolution_clock::time_point> saveTimeEnd;
    gmp_randstate_t randstate;


    double bdwMU_SKS, bdwSU_SKS;
    double bdwMU_SE, bdwSU_SE;
    double bdwMU_STS, bdwSU_STS;
    double bdwMU_SHBC, bdwSU_SHBC;
    double bdwMU_SSig, bdwSU_SSig;
    double bdwMU_SDP, bdwSU_SDP;
    double bdwMU_SD, bdwSU_SD;
    double bdwMU_SM, bdwSU_SM;
    double bdwMU_SC, bdwSU_SC;
    double bdwMU_SLog, bdwSU_SLog;
    double bdwMU_SSqrt, bdwSU_SSqrt;
    double bdwMU_SExpo, bdwSU_SExpo;
    double bdwMU_SPU, bdwSU_SPU;

    /*double totalRuntiume [13];
    double sumSquares [13];

    for (int k=0; k <13 ; k++ )
    {
        totalRuntiume[k] = 0.0;
        sumSquares [k] = 0.0;
    }
    */



    std::ofstream stats;
    stats.open(outputfile);

    stats <<"SKS,SE,STS,SHBC,SSig,SDP,SD,SM,SC,SLog,SSqrt,SExpo,SPU"<<endl;

    int N=1;


    if (deserialize) {
        dtpkc.deserializeDtpkc(filename);
        dtpkc.blindVal = prec;
        dtpkc.optim = false;
        pkey = dtpkc.pkw;

    } else {

        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate, time(NULL));
        std::chrono::milliseconds delay(1000);
        dtpkc.keygen(prec, randstate, delay, keysize, error, optim,200 );

        // Generate public key
        dtpkc.getKey(pkey, skey);
        dtpkc.updatePkw(pkey);
        //cout << "Generated dtpkc parameters " << endl;
    }
    std::chrono::milliseconds delay_(10);
    dtpkc.delay=delay_;

    dtpkc.getKey(newPkey, newSkey);
    dtpkc.updatePkw(newPkey);

    // Generate operands

    B = mpz_class_get_z_bits(48);
    A = B + mpz_class_get_z_bits(48);

    // Encrypt operands
    C_A = dtpkc.enc(A, pkey);
    C_B = dtpkc.enc(B, pkey);

    Cipher C_T;

    T.set_str("1000", 10);
    C_T = dtpkc.enc(T, pkey);

    int size = 5;
    int t = 1000;
    vector<mpz_class> INTER(size);
    vector<Cipher> classses(size);
    vector<int> inter(size);

    for(int g=0;g<5;g++)
    {
        inter[g]=200;
        INTER[g].set_str("200",10);
        classses[g] = dtpkc.enc(INTER[g],pkey);
    }


    string line ="";

    for (int j=0; j<N; j++) {


/**
* Test SkeySwitch
*/

        auto begin  = chrono::high_resolution_clock::now();
        SkeySwitch skeySwitch(C_A, newPkey, dtpkc);
        C_C = skeySwitch.SkeySwitch_U1_step3(skeySwitch.SkeySwitch_U2_step2(skeySwitch.SkeySwitch_U1_step1()));
        auto en = chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);




/**
* Test Secure entropy
*/

        begin  = chrono::high_resolution_clock::now();
        EvalEntropy evalEntropy(C_T, classses, dtpkc);
        C_C = evalEntropy.EvalEntropy_U1_step3(evalEntropy.EvalEntropy_U2_step2(evalEntropy.EvalEntropy_U1_step1()));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);



/**
* Test EvalThreashold
**/

        Cipher i1, i2,i3;

        i1 = dtpkc.enc(10000, pkey);
        i2 = dtpkc.enc(1000000, pkey);
        i3 = dtpkc.enc(5, pkey);

        begin  = chrono::high_resolution_clock::now();
        EvalDiv e_(i1,i3,dtpkc);
        vector<Cipher> res = e_.EvalDiv_U1_step1();
        Cipher mean= e_.EvalDiv_U1_step3(e_.EvalDiv_U2_step2(res[0], res[1],res[2],res[3]));

        EvalSqrt evalsqrt_(i3, dtpkc);
        C_C = evalsqrt_.EvalHbound_U1_step3(evalsqrt_.EvalHBound_U2_step2(evalsqrt_.EvalHBound_U1_step1()));

        double x = 0.7;

        EvalThreashold evalThreashold(mean,C_C, x, dtpkc);
        C_C = evalThreashold.eval();
        en = chrono::high_resolution_clock::now();

        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";



/**
* Test EvalHBound
*/

        begin = chrono::high_resolution_clock::now();

        mpz_class i = 10000;
        i = i * pow(log2(3) * 100, 2) * log(1 / 0.00001);
        begin  = chrono::high_resolution_clock::now();
        EvalHBound evalHBound(dtpkc.enc(i, pkey), dtpkc.enc(1000, pkey), dtpkc);
        C_C = evalHBound.EvalHbound_U1_step3(evalHBound.EvalHBound_U2_step2(evalHBound.EvalHBound_U1_step1()));

        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";





/**
* Test Sigmoid
*/

        x = 2;
        Cipher C_x = dtpkc.enc(x, dtpkc.pkw);
        C_x.sign = 0;
        begin  = chrono::high_resolution_clock::now();
        EvalSigmoid evalSigmoid(C_x, 100, dtpkc);
        vector<Cipher> in = evalSigmoid.EvalSig_U1_step1();
        C_C = evalSigmoid.EvalSig_U1_step3(evalSigmoid.EvalSig_U2_step2(in[0], in[1]));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);



        /**
         * Testing the dot product protocol
         */


        vector<mpz_class> av, bv, cv;
        int alpha = 20;
        mpz_class alpha2 = 20;

        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(1);
        av.push_back(3000);
        av.push_back(20);
        av.push_back(15);
        av.push_back(17);
        av.push_back(200);

        bv.push_back(30);
        bv.push_back(3);
        bv.push_back(1);
        bv.push_back(3000);
        bv.push_back(20);
        bv.push_back(15);
        bv.push_back(17);
        bv.push_back(200);
        bv.push_back(1);
        bv.push_back(3000);
        bv.push_back(20);
        bv.push_back(15);
        bv.push_back(17);
        bv.push_back(200);


        cv.push_back(15);
        cv.push_back(-3);
        cv.push_back(10);
        cv.push_back(30);
        cv.push_back(-10);
        cv.push_back(-105);
        cv.push_back(107);
        cv.push_back(4200);
        cv.push_back(1);
        cv.push_back(3000);
        cv.push_back(20);
        cv.push_back(15);
        cv.push_back(17);
        cv.push_back(200);

        vector<Cipher> C_av, C_bv, C_cv;
        mpz_class output = 0;
        mpz_class output2 = 0;
        for (int i = 0; i < 14; i++) {
            C_av.push_back(dtpkc.enc(av[i], dtpkc.pkw));
            C_bv.push_back(dtpkc.enc(bv[i], dtpkc.pkw));
            C_cv.push_back(dtpkc.enc(cv[i], dtpkc.pkw));
            output = output + alpha2 * (av[i] - bv[i]) * cv[i];
            output2 = output2 + cv[i] * bv[i];
        }

        begin  = chrono::high_resolution_clock::now();
        EvalDotProd evalDotProd(C_cv, C_bv, C_bv.size(), dtpkc);
        C_C = evalDotProd.EvalDotProd_U1_step3(evalDotProd.EvalDotProd_U2_step2(evalDotProd.EvalDotProd_U1_step1()));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);



/**
* Test EvalDiv
*/
        begin  = chrono::high_resolution_clock::now();
        EvalDiv div(C_A, C_B, dtpkc);
        res = div.EvalDiv_U1_step1();
        C_C = div.EvalDiv_U1_step3(div.EvalDiv_U2_step2(res[0], res[1], res[2], res[3]));




        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);

/**
* Test EvalMult
*/
        begin  = chrono::high_resolution_clock::now();
        EvalMult e1(C_A, C_B, dtpkc);
        C_C = e1.EvalMult_U1_step3(e1.EvalMult_U2_step2(e1.EvalMult_U1_step1()));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);





/**
* Test Comparion
*/
        begin = chrono::high_resolution_clock::now();


        EvalCmp evalCmp(C_A, C_B, dtpkc);
        bool r = evalCmp.EvalCmp_U2_step2(evalCmp.EvalCmp_U1_step1());

        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";



        x = 194;
        C_x = dtpkc.enc(x, dtpkc.pkw);
        C_x.sign = 1;
        cpt++;
        begin  = chrono::high_resolution_clock::now();
        EvalLog evalLog(C_x, 10000, dtpkc);
        in = evalLog.EvalLog_U1_step1();
        C_C = evalLog.EvalLog_U1_step3(evalLog.EvalLog_U2_step2(in[0], in[1]));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);


/**
* Test EvalSqrt
*/
        begin  = chrono::high_resolution_clock::now();
        EvalSqrt evalsqrt(C_A, dtpkc);
        C_C = evalsqrt.EvalHbound_U1_step3(evalsqrt.EvalHBound_U2_step2(evalsqrt.EvalHBound_U1_step1()));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);


/**
* Test Expo
*/


        begin  = chrono::high_resolution_clock::now();
        EvalExpo evalExpo(C_A, 100, dtpkc);
        vector<Cipher> in_ = evalExpo.EvalSig_U1_step1();
        C_C = evalExpo.EvalSig_U1_step3(evalExpo.EvalSig_U2_step2(in_[0], in_[1]));
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";



/**
* Test EvalProba
*/
        begin  = chrono::high_resolution_clock::now();

        EvalDiv div_(C_A,C_B,dtpkc);
        res = div_.EvalDiv_U1_step1();
        C_C=div_.EvalDiv_U1_step3(div_.EvalDiv_U2_step2(res[0],res[1],res[2],res[3]));

        EvalLog evalLog_(C_C, 10000, dtpkc);
        in = evalLog_.EvalLog_U1_step1();
        C_C = evalLog_.EvalLog_U1_step3(evalLog_.EvalLog_U2_step2(in[0], in[1]));

        //C_C.T1= mpz_class_powm(C_C.T1,dtpkc.n-1,dtpkc.n2);
        //C_C.T2= mpz_class_powm(C_C.T2,dtpkc.n-1,dtpkc.n2);

        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()) +",";
        C = dtpkc.Sdec(C_C);

        stats << line << endl;
        cout << line << endl;
        line ="";

/**
 * Summary
 */



        bdwMU_SKS +=skeySwitch.bdwMU; bdwSU_SKS +=skeySwitch.bdwSU;
        bdwMU_SE +=evalEntropy.bdwMU; bdwSU_SE +=evalEntropy.bdwSU;
        bdwMU_STS +=evalThreashold.bdwMU; bdwSU_STS +=evalThreashold.bdwSU;
        bdwMU_SHBC +=evalHBound.bdwMU; bdwSU_SHBC +=evalHBound.bdwSU;
        bdwMU_SSig +=evalSigmoid.bdwMU; bdwSU_SSig +=evalSigmoid.bdwSU;
        bdwMU_SDP +=evalDotProd.bdwMU; bdwSU_SDP +=evalDotProd.bdwSU;
        bdwMU_SD +=div.bdwMU; bdwSU_SD +=div.bdwSU;
        bdwMU_SM +=e1.bdwMU; bdwSU_SM +=e1.bdwSU;
        bdwMU_SC +=evalCmp.bdwMU; bdwSU_SC +=evalCmp.bdwSU;
        bdwMU_SLog +=evalLog.bdwMU; bdwSU_SLog +=evalLog.bdwSU;
        bdwMU_SSqrt +=evalsqrt.bdwMU; bdwSU_SSqrt +=evalsqrt.bdwSU;
        bdwMU_SExpo +=evalExpo.bdwMU; bdwSU_SExpo +=evalExpo.bdwSU;
        bdwMU_SPU +=e_.bdwMU; bdwSU_SPU +=e_.bdwSU + evalLog_.bdwMU; bdwSU_SLog +=evalLog_.bdwSU;


    }


    bdwMU_SKS /=N; bdwSU_SKS /=N;
    bdwMU_SE /=N; bdwSU_SE /=N;
    bdwMU_STS /=N; bdwSU_STS /=N;
    bdwMU_SHBC /=N; bdwSU_SHBC /=N;
    bdwMU_SSig /=N; bdwSU_SSig /=N;
    bdwMU_SDP /=N; bdwSU_SDP /=N;
    bdwMU_SD /=N; bdwSU_SD /=N;
    bdwMU_SM /=N; bdwSU_SM /=N;
    bdwMU_SC /=N; bdwSU_SC /=N;
    bdwMU_SLog /=N; bdwSU_SLog/=N;
    bdwMU_SSqrt /=N; bdwSU_SSqrt /=N;
    bdwMU_SExpo /=N; bdwSU_SExpo /=N;
    bdwMU_SPU /=N; bdwSU_SPU /=N;

    //double bdw = mpz_size(C_A.T1.get_mpz_t())+ mpz_size(C_A.T2.get_mpz_t())* sizeof(mp_limb_t);
    //cout << "bdw " << bdw;

    stats << "****************************************" <<endl;
    stats << " bdw_SKS= " << bdwMU_SKS  + bdwSU_SKS << endl;
    stats << " bdw_SE" << bdwMU_SE  + bdwSU_SE << endl;
    stats << " bdw_STS= " << bdwMU_STS  + bdwSU_STS << endl;
    stats <<" bdw_SHBC= " << bdwMU_SHBC  +  bdwSU_SHBC << endl;
    stats << " bdw_SSig= " << bdwMU_SSig  + bdwSU_SSig << endl;
    stats << " bdw_SDP= " << bdwMU_SDP  + bdwSU_SDP << endl;
    stats << " bdw_SD= " << bdwMU_SD  +  bdwSU_SD << endl;
    stats << " bdw_SM= " << bdwMU_SM  + bdwSU_SM << endl;
    stats <<" bdw_SC= " << bdwMU_SC  + bdwSU_SC << endl;
    stats <<" bdw_SLog= " << bdwMU_SC  + bdwSU_SLog << endl;
    stats << " bdw_SExpo= " << bdwMU_SExpo  + bdwSU_SExpo << endl;
    stats <<" bdw_SSqrt= " << bdwMU_SSqrt + bdwSU_SSqrt << endl;
    stats <<" bdw_SPU= " << bdwMU_SPU  + bdwSU_SPU << endl;



    stats.close();

    if(!deserialize) gmp_randclear(randstate);
}




void TESTBuildingBlocks::TrunGlobal(bool deserialize, int keysize, int prec, int error, string filename) {

    DTPKC dtpkc;
    int cpt=0;
    mpz_class pkey, skey, newPkey, newSkey;
    mpz_class A, B,C, T;
    DTPKC::Cipher C_A, C_B, C_C;
    vector<std::chrono::high_resolution_clock::time_point> saveTimeStart;
    vector<std::chrono::high_resolution_clock::time_point> saveTimeEnd;
    gmp_randstate_t randstate;


    double bdwMU_SKS, bdwSU_SKS;
    double bdwMU_SE, bdwSU_SE;
    double bdwMU_STS, bdwSU_STS;
    double bdwMU_SHBC, bdwSU_SHBC;
    double bdwMU_SSig, bdwSU_SSig;
    double bdwMU_SDP, bdwSU_SDP;
    double bdwMU_SD, bdwSU_SD;
    double bdwMU_SM, bdwSU_SM;
    double bdwMU_SC, bdwSU_SC;
    double bdwMU_SLog, bdwSU_SLog;
    double bdwMU_SSqrt, bdwSU_SSqrt;
    double bdwMU_SExpo, bdwSU_SExpo;
    double bdwMU_SPU, bdwSU_SPU;



    std::ofstream stats;
    stats.open( "buildingblocks-Stats.csv");

    stats <<"SKS,SE,STS,SHBC,SSig,SDP,SD,SM,SC,SLog,SSqrt,SExpo,SPU"<<endl;

    int N=1;


    if (deserialize) {
        dtpkc.deserializeDtpkc(filename);
        dtpkc.blindVal = prec;
        pkey = dtpkc.pkw;

    } else {

        gmp_randinit_default(randstate);
        gmp_randseed_ui(randstate, time(NULL));
        std::chrono::milliseconds delay(1000);
        dtpkc.keygen(prec, randstate, delay, keysize, error);

        // Generate public key
        dtpkc.getKey(pkey, skey);
        dtpkc.updatePkw(pkey);
        //cout << "Generated dtpkc parameters " << endl;
    }
    std::chrono::milliseconds delay_(10);
    dtpkc.delay=delay_;

    dtpkc.getKey(newPkey, newSkey);
    dtpkc.updatePkw(newPkey);

    // Generate operands

    B = mpz_class_get_z_bits(48);
    A = B + mpz_class_get_z_bits(48);

    // Encrypt operands
    C_A = dtpkc.enc(A, pkey);
    C_B = dtpkc.enc(B, pkey);

    Cipher C_T;

    T.set_str("1000", 10);
    C_T = dtpkc.enc(T, pkey);

    int size = 5;
    int t = 1000;
    vector<mpz_class> INTER(size);
    vector<Cipher> classses(size);
    vector<int> inter(size);

    for(int g=0;g<5;g++)
    {
        inter[g]=200;
        INTER[g].set_str("200",10);
        classses[g] = dtpkc.enc(INTER[g],pkey);
    }


    string line ="";
    auto begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        SkeySwitch skeySwitch(C_A, newPkey, dtpkc);
        C_C = skeySwitch.SkeySwitch_U1_step3(skeySwitch.SkeySwitch_U2_step2(skeySwitch.SkeySwitch_U1_step1()));

    }
    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en - begin;
    line += std::to_string(duration.count()/N) + ",";




/**
* Test Secure entropy
*/

    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalEntropy evalEntropy(C_T, classses, dtpkc);
        C_C = evalEntropy.EvalEntropy_U1_step3(evalEntropy.EvalEntropy_U2_step2(evalEntropy.EvalEntropy_U1_step1()));
    }
    en = chrono::high_resolution_clock::now();
    duration = en - begin;
    line += std::to_string(duration.count()/N) + ",";


/**
* Test EvalThreashold
**/

        Cipher i1, i2,i3;

        i1 = dtpkc.enc(10000, pkey);
        i2 = dtpkc.enc(1000000, pkey);
        i3 = dtpkc.enc(5, pkey);



    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {


        EvalDiv e_(i1, i3, dtpkc);
        vector<Cipher> res = e_.EvalDiv_U1_step1();
        Cipher mean = e_.EvalDiv_U1_step3(e_.EvalDiv_U2_step2(res[0], res[1], res[2], res[3]));

        EvalSqrt evalsqrt_(i3, dtpkc);
        C_C = evalsqrt_.EvalHbound_U1_step3(evalsqrt_.EvalHBound_U2_step2(evalsqrt_.EvalHBound_U1_step1()));

        double x = 0.7;

        EvalThreashold evalThreashold(mean, C_C, x, dtpkc);
        C_C = evalThreashold.eval();
    }

        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";



/**
* Test EvalHBound
*/

    begin = chrono::high_resolution_clock::now();
    mpz_class i = 10000;
    i = i * pow(log2(3) * 100, 2) * log(1 / 0.00001);
    for (int j=0; j<N; j++) {

        EvalHBound evalHBound(dtpkc.enc(i, pkey), dtpkc.enc(1000, pkey), dtpkc);
        C_C = evalHBound.EvalHbound_U1_step3(evalHBound.EvalHBound_U2_step2(evalHBound.EvalHBound_U1_step1()));

    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";





/**
* Test Sigmoid
*/

        int x = 2;
        Cipher C_x = dtpkc.enc(x, dtpkc.pkw);
        C_x.sign = 0;

    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalSigmoid evalSigmoid(C_x, 100, dtpkc);
        vector<Cipher> in = evalSigmoid.EvalSig_U1_step1();
        C_C = evalSigmoid.EvalSig_U1_step3(evalSigmoid.EvalSig_U2_step2(in[0], in[1]));
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";




        /**
         * Testing the dot product protocol
         */


        vector<mpz_class> av, bv, cv;
        int alpha = 20;
        mpz_class alpha2 = 20;

        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(0);
        av.push_back(1);
        av.push_back(3000);
        av.push_back(20);
        av.push_back(15);
        av.push_back(17);
        av.push_back(200);

        bv.push_back(30);
        bv.push_back(3);
        bv.push_back(1);
        bv.push_back(3000);
        bv.push_back(20);
        bv.push_back(15);
        bv.push_back(17);
        bv.push_back(200);
        bv.push_back(1);
        bv.push_back(3000);
        bv.push_back(20);
        bv.push_back(15);
        bv.push_back(17);
        bv.push_back(200);


        cv.push_back(15);
        cv.push_back(-3);
        cv.push_back(10);
        cv.push_back(30);
        cv.push_back(-10);
        cv.push_back(-105);
        cv.push_back(107);
        cv.push_back(4200);
        cv.push_back(1);
        cv.push_back(3000);
        cv.push_back(20);
        cv.push_back(15);
        cv.push_back(17);
        cv.push_back(200);

        vector<Cipher> C_av, C_bv, C_cv;
        mpz_class output = 0;
        mpz_class output2 = 0;
        for (int i = 0; i < 14; i++) {
            C_av.push_back(dtpkc.enc(av[i], dtpkc.pkw));
            C_bv.push_back(dtpkc.enc(bv[i], dtpkc.pkw));
            C_cv.push_back(dtpkc.enc(cv[i], dtpkc.pkw));
            output = output + alpha2 * (av[i] - bv[i]) * cv[i];
            output2 = output2 + cv[i] * bv[i];
        }

    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {


        EvalDotProd evalDotProd(C_cv, C_bv, C_bv.size(), dtpkc);
        C_C = evalDotProd.EvalDotProd_U1_step3(evalDotProd.EvalDotProd_U2_step2(evalDotProd.EvalDotProd_U1_step1()));
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";




/**
* Test EvalDiv
*/
    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalDiv div(C_A, C_B, dtpkc);
        vector<Cipher> res = div.EvalDiv_U1_step1();
        C_C = div.EvalDiv_U1_step3(div.EvalDiv_U2_step2(res[0], res[1], res[2], res[3]));

    }


        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";
        C = dtpkc.Sdec(C_C);

/**
* Test EvalMult
*/
    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalMult e1(C_A, C_B, dtpkc);
        C_C = e1.EvalMult_U1_step3(e1.EvalMult_U2_step2(e1.EvalMult_U1_step1()));
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";





/**
* Test Comparion
*/
    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalCmp evalCmp(C_A, C_B, dtpkc);
        bool r = evalCmp.EvalCmp_U2_step2(evalCmp.EvalCmp_U1_step1());
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";



        x = 194;
        C_x = dtpkc.enc(x, dtpkc.pkw);
        C_x.sign = 1;
        cpt++;

    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalLog evalLog(C_x, 10000, dtpkc);
        vector<Cipher>  in = evalLog.EvalLog_U1_step1();
        C_C = evalLog.EvalLog_U1_step3(evalLog.EvalLog_U2_step2(in[0], in[1]));
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";
        C = dtpkc.Sdec(C_C);


/**
* Test EvalSqrt
*/

    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalSqrt evalsqrt(C_A, dtpkc);
        C_C = evalsqrt.EvalHbound_U1_step3(evalsqrt.EvalHBound_U2_step2(evalsqrt.EvalHBound_U1_step1()));
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";



/**
* Test Expo
*/



    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalExpo evalExpo(C_A, 100, dtpkc);
        vector<Cipher> in_ = evalExpo.EvalSig_U1_step1();
        C_C = evalExpo.EvalSig_U1_step3(evalExpo.EvalSig_U2_step2(in_[0], in_[1]));
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";



/**
* Test EvalProba
*/

    begin = chrono::high_resolution_clock::now();
    for (int j=0; j<N; j++) {

        EvalDiv div_(C_A, C_B, dtpkc);
        vector<Cipher> res = div_.EvalDiv_U1_step1();
        C_C = div_.EvalDiv_U1_step3(div_.EvalDiv_U2_step2(res[0], res[1], res[2], res[3]));

        EvalLog evalLog_(C_C, 10000, dtpkc);
        vector<Cipher> in = evalLog_.EvalLog_U1_step1();
        C_C = evalLog_.EvalLog_U1_step3(evalLog_.EvalLog_U2_step2(in[0], in[1]));

        //C_C.T1= mpz_class_powm(C_C.T1,dtpkc.n-1,dtpkc.n2);
        //C_C.T2= mpz_class_powm(C_C.T2,dtpkc.n-1,dtpkc.n2);
    }
        en = chrono::high_resolution_clock::now();
        duration = en  - begin ;
        line += std::to_string(duration.count()/N) +",";


        stats << line << endl;
        stats.close();



    if(!deserialize) gmp_randclear(randstate);
}



