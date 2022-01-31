#include "EvalThreashold.h"
#include "EvalDiv.h"


EvalThreashold::EvalThreashold(Cipher A, Cipher B , double x, DTPKC dtpkc)
{
    this->A=A;
    this->B=B;
    this->x=x;
    this->dtpkc=dtpkc;
}


Cipher
EvalThreashold::eval()
{

        mpz_class m;
        Cipher S1, res ;

    auto begin = chrono::high_resolution_clock::now();

    A.T1 = mpz_class_powm(A.T1, 100, dtpkc.n2);
    A.T2 = mpz_class_powm(A.T2, 100, dtpkc.n2);
    A.Pub = A.Pub;

    B.T1 = mpz_class_powm(B.T1, 100, dtpkc.n2);
    B.T2 = mpz_class_powm(B.T2, 100, dtpkc.n2);
    B.Pub = B.Pub;

    m= NormalCDFInverse(x)*100.0;

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    timeMU = duration.count();

        if (m > 0 ) {
            //cout << "m =" << m << endl;
            // -B*m*100
            begin = chrono::high_resolution_clock::now();

            S1.T1 = mpz_class_powm(B.T1, m, dtpkc.n2);
            S1.T2 = mpz_class_powm(B.T2, m, dtpkc.n2);
            S1.Pub = B.Pub;

            en = chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = en  - begin ;
            timeMU += duration.count();

            //cout << "B =" << B << endl;

            EvalDiv div(S1, dtpkc.enc(100, B.Pub), dtpkc);


            std::vector<DTPKC::Cipher> re = div.EvalDiv_U1_step1();
            S1 = div.EvalDiv_U1_step3(div.EvalDiv_U2_step2(re[0], re[1], re[2], re[3]));

            timeMU += div.timeMU;
            timeSU = div.timeSU;

            bdwMU = div.bdwMU;
            bdwSU = div.bdwSU;


                    //cout << "S1 =" << S1 << endl;

            // Add mean to B*m
            begin = chrono::high_resolution_clock::now();

            res.T1 = S1.T1 * A.T1;
            res.T2 = S1.T2 * A.T2;
            res.Pub = A.Pub;

            en = chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration2 = en  - begin ;
            timeMU += duration2.count();

            //cout << "A =" << A << endl;



        }else
        {

            //cout << "m =" << m << endl;
            // -B*m*100
            S1.T1 = mpz_class_powm(B.T1, -m, dtpkc.n2);
            S1.T2 = mpz_class_powm(B.T2, -m, dtpkc.n2);
            S1.Pub = B.Pub;

            std::chrono::duration<double, std::milli> duration = en  - begin ;
            timeMU += duration.count();
            //cout << "B =" << B << endl;

            EvalDiv div(S1, dtpkc.enc(100, B.Pub), dtpkc);
            std::vector<DTPKC::Cipher> re = div.EvalDiv_U1_step1();
            S1 = div.EvalDiv_U1_step3(div.EvalDiv_U2_step2(re[0], re[1], re[2], re[3]));
            timeMU += div.timeMU;
            timeSU = div.timeSU;

            bdwMU = div.bdwMU;
            bdwSU = div.bdwSU;

            //cout << "A =" << A << endl;
            begin = chrono::high_resolution_clock::now();



            if (A > S1 || A == S1) {

                S1.T1 = mpz_class_powm(S1.T1, dtpkc.n - 1, dtpkc.n2);
                S1.T2 = mpz_class_powm(S1.T2, dtpkc.n - 1, dtpkc.n2);
                S1.Pub = B.Pub;


                // Add mean to B*m
                res.T1 = S1.T1 * A.T1;
                res.T2 = S1.T2 * A.T2;
                res.Pub = A.Pub;




            }else
            {
                res.T1 = -1;
                res.T2 = -1;
                res.Pub = A.Pub;
            }


            std::chrono::duration<double, std::milli> duration2 = en  - begin ;
            timeMU += duration2.count();
        }


    //cout << "res =" << res << endl;

    return res;

}

