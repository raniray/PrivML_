#include "EvalSub2.h"
#include "DTPKC.h"



EvalSub2 :: EvalSub2(Cipher a, Cipher b, DTPKC dtpkc)
{
    this->a=a;
    this->b=b;
    this->dtpkc=dtpkc;
}



Cipher
EvalSub2 :: EvalSub_U1()
{

        if ( (a.sign == 1 && b.sign ==  1 ) || (a.sign == 0 && b.sign ==  0 )){

            if (a > b){

                c.T1=a.T1*mpz_class_powm(b.T1,dtpkc.n-1,dtpkc.n2);
                c.T2=a.T2*mpz_class_powm(b.T2,dtpkc.n-1,dtpkc.n2);
                c.Pub=a.Pub;
                c.sign = 1;

            }else{

                c.T1=b.T1*mpz_class_powm(a.T1,dtpkc.n-1,dtpkc.n2);
                c.T2=b.T2*mpz_class_powm(a.T2,dtpkc.n-1,dtpkc.n2);
                c.Pub=a.Pub;
                c.sign = 0;

            }

            if (a.sign == 0){
                c.sign = 1 - c.sign;
            }


        }else{
            c.T1=b.T1*a.T1;
            c.T2=b.T2*a.T2;
            c.Pub=a.Pub;

            if (b.sign == 0){

                c.sign = 1;

            } else {

                c.sign = 0;
            }


        }

    return c;
}