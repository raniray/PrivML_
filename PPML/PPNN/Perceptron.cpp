#include <chrono>
#include "Perceptron.h"
#include "../../CRYPTO/EvalDotProd.h"
#include "../../CRYPTO/EvalSub2.h"


#define MAX_DATE 120
using namespace std;

Cipher Perceptron::Sigmoid(Cipher x) {

    Cipher C_C;
    Cipher fifty = dtpkc.enc(50, x.Pub);
    C_C = fifty;
    C_C.T1 = C_C.T1 * x.T1;
    C_C.T2 = C_C.T2 * x.T2;
    C_C.Pub = x.Pub;
    C_C.sign = x.sign;
    return C_C;
}



Perceptron::Perceptron(vector<Cipher> weights, int alpha, int epochs, int batchSize, int th,
                                 PPDatasetReader *datasetReader,
                                 bool debug)
{
    this->datasetReader= datasetReader;

    this->debug=debug;
    this->dtpkc=datasetReader->dtpkc;
    this->w = weights;
    this->alpha = alpha;
    this->batchSize=batchSize;
    this->th = dtpkc.enc(th, dtpkc.pkw);
    this->dt = dt;
    this->epochs=epochs;


    zero =  dtpkc.enc(0,dtpkc.pkw);
    one =  dtpkc.enc(1,dtpkc.pkw);



    if (debug )
    {
        for (int i=0; i< w.size(); i++)
            cout << "w["<<i<<"]= " << w[i] <<endl;
            cout << "alpha " << alpha << endl;
            cout << "batchSize " << batchSize<< endl;
            cout << "th " << th << endl;
            cout << ""<< endl;
    }



}


Cipher Perceptron::dotProduct(vector<Cipher> w, PPRecord * r, int size )
{
    EvalDotProd evalDotProd(w, r->values,size, dtpkc);
    Cipher C_C = evalDotProd.EvalDotProd_U1_step3(evalDotProd.EvalDotProd_U2_step2(evalDotProd.EvalDotProd_U1_step1()));
    return C_C;
}

Cipher Perceptron::predict(PPRecord *r, bool test ) {

    Cipher wx = zero;
    Cipher proba = zero;
    Cipher res =  zero;
    int size = r->values.size()-1;

    if (test) size = size + 1;
    wx = dotProduct(w,r, size);
    proba = Sigmoid(wx);

    if ((proba < th) or (proba.sign == 0))
        res = zero;
    else
        res = one;


    if (debug)
    {
        cout << "WX " << wx << endl;
        cout << "Proba " << proba << endl;
        cout << "Res " << res << endl;
        cout << "Th " << th << endl;
    }


    return res;
}


vector<Cipher> Perceptron::miniBatchGrad(vector<PPRecord *> XB) {

    vector<Cipher> ypred, ytrue;
    PPRecord *r;
    int dim = XB[0]->values.size() - 1;
    Cipher inter;

    Cipher transpose [dim] [XB.size()];

    // Predict labels, compute error and transpose
    for (int j=0; j< XB.size(); j++ )
    {
        r = XB[j];
        if (debug) r->print();

        ypred.push_back(predict(r,false));
        ytrue.push_back(r->values[r->values.size()-1]);

        for (int i=0; i < dim ; i++ )
        {
            transpose [i][j] = r->values[i];

        }

    }

    vector< Cipher> prod(dim);


#pragma omp parallel for shared (prod, ypred, ytrue, transpose) schedule(static)
    for (int i=0; i < dim ; i++ )
    {

        vector<Cipher> line;

        for (int j=0; j< XB.size(); j++ )
        {
            Cipher tmp = transpose[i][j];
            line.push_back(tmp);
        }


        //#pragma omp critical
        {

            prod[i] = inter;

        };

    }


    if (debug ) {

        for (int i = 0; i < w.size(); i++)
            cout << "prod[" << i << "]= " << prod[i] << endl;

    }
    return  prod;

}



void Perceptron::train() {

    int counter = 0;
    int recordCounter = 0;
    int sizeBatch = batchSize;
    int size = dt->train_size;
    PPRecord *record;
    vector<PPRecord *> XB;


    auto begin = chrono::high_resolution_clock::now();

    for (int epochCpt = 0; epochCpt < epochs; epochCpt++) {

        //shuffle(dt);
        while (counter < size) {
            if (size - counter < batchSize)
                sizeBatch = size - counter;


            for (recordCounter = 0; recordCounter < sizeBatch; recordCounter++) {
                try {

                    record = dt->getTrainRecord();
                    XB.push_back(record);
                    counter++;
                }
                catch (std::exception const &e) {
                    cout << e.what() << endl;
                }

            }

            vector<Cipher> grad = miniBatchGrad(XB);

#pragma omp parallel for
            for (int i = 0; i < w.size(); i++) {
                EvalSub2 sub(w[i], grad[i], dtpkc);
                w[i] = sub.EvalSub_U1();
            }


            if (debug) {
                for (int i = 0; i < w.size(); i++) {
                    cout << "W after update " << w[i] << endl;
                    cout << grad[i] << endl;
                }
            }

            for (int i = 0; i < XB.size(); i++) {
                delete XB[i];
            }

            XB.clear();


        }

        counter = 0;

    }

    }
