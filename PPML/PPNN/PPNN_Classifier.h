#ifndef DAPPLE_PPNN_CLASSIFIER_H
#define DAPPLE_PPNN_CLASSIFIER_H

#include "../IO/PPDatasetReader.h"
#include "../../UTIL/math_util.hh"
#include "../../CRYPTO/EvalSigmoid.h"
#include "../../CRYPTO/EvalDotProd.h"
#include "../../CRYPTO/EvalLrUpdate.h"
#include "../../CRYPTO/EvalSub2.h"
#include "Perceptron.h"
#include "math.h"


class PPNN_Classifier {

public :

    PPDatasetReader * datasetReader;
    string logfile;
    bool debug;
    mpz_class  extTrainBd;
    mpz_class extTestBd;
    double trainTime;
    double testTime;
    double clientTestTime;
    DTPKC dtpkc;
    string mainpath;
    int alpha;
    int batchSize;


    vector<Perceptron> hidden_layer1;
    vector<Perceptron> hidden_layer2;
    vector<Perceptron> output_layer;

    int input_layer_dim = 14;
    int hidden_layer1_dim = 12;
    int hidden_layer2_dim = 6;
    int output_layer_dim = 2;



    PPDatasetReader *dt;
    int epochs;


    Cipher zero;
    Cipher one;


public :
    PPNN_Classifier(vector<Cipher> wieghts, int alpha, int epochs, int batchSize, int th, PPDatasetReader *datasetReader, string log, bool debug, string mainpath);


public:
~PPNN_Classifier()
    {

    }

public:
    Cipher predict(PPRecord *r, bool test );
public :
    void train ();

public :
    void Test();


};
#endif //DAPPLE_VFDT_CLASSIFIER_H
