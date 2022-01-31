#ifndef DAPPLE_PPRECORD_H
#define DAPPLE_PPRECORD_H
#include<vector>
#include <iostream>
#include "../../CRYPTO/DTPKC.h"

using namespace std;

class PPRecord {


public :
    int id; // represents the id of the record
    std::vector <Cipher> values; // the values of the record
    DTPKC dtpkc;


    /******* Functions **********/


public : PPRecord(int id, std::vector<Cipher> values, DTPKC dtpkc) {
        this->id=id;
        this->values=values;
        this->dtpkc=dtpkc;
    }


public :

    void print() {

        int i=0;
        cout << "ID = " << id << " record = ( ";
        for (i=0; i<values.size()-1;i++)
            cout << dtpkc.Sdec(values[i]) <<", ";
        cout <<  dtpkc.Sdec(values[i]) <<" ) " << endl;

    }



public :
    int getSize()
    {
        int bd =0;
        for (int i=0; i<values.size()-1;i++)
        {
            bd +=mpz_size(values[i].T1.get_mpz_t())+ mpz_size(values[i].T2.get_mpz_t())* sizeof(mp_limb_t);
        }
        return bd;
    }

};

#endif
