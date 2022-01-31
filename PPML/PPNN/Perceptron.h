#ifndef DAPPLE_PERCEPTRON_H
#define DAPPLE_PERCEPTRON_H

#include <iostream>
#include <map>
#include <iterator>
#include "../IO/PPRecord.h"
#include <gmpxx.h>
#include <gmp.h>
#include <omp.h>
#include "../../UTIL/util_gmp_rand.h"
#include "../../UTIL/num_th_alg.hh"
#include "../../UTIL/mpz_class.hh"
#include "../../CRYPTO/DTPKC.h"
#include "../../CRYPTO/EvalAdd.h"
#include "../../CRYPTO/EvalMult.h"
#include "../../CRYPTO/EvalDiv100.h"
#include "../../CRYPTO/EvalProba.h"
#include "../../CRYPTO/EvalLog.h"
#include "../IO/PPDatasetReader.h"

class Perceptron {


public :

    PPDatasetReader * datasetReader;
    bool debug;
    DTPKC dtpkc;
    string mainpath;
    int alpha;
    int batchSize;
    vector<Cipher> w;
    Cipher th;
    PPDatasetReader *dt;
    int epochs;
    Cipher zero;
    Cipher one;

    /******* Functions **********/

public :
    Cipher Sigmoid(Cipher x);

public :
    Cipher predict(PPRecord *r, bool test );

public :
    Perceptron(vector<Cipher> wieghts, int alpha, int epochs, int batchSize, int th, PPDatasetReader *datasetReader, bool debug);

public :
    vector<Cipher> miniBatchGrad(vector<PPRecord *> XB);

public:
    Cipher dotProduct(vector<Cipher> w, PPRecord * r, int size );

public:
    ~Perceptron()
    {

    }

public :
    void train ();


};

#endif //DAPPLE_PERCEPTRON_H
