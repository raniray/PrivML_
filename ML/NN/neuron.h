//
// Created by rania on 12/03/19.
//

#ifndef DAPPLE_NEURON_H
#define DAPPLE_NEURON_H
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include "../IO/Record.h"
#include "../IO/DatasetReader.h"
#include <chrono>

using namespace std;

class neuron{

public :
    double alpha;
    int batchSize;
    vector<float> w;
    float th;
    DatasetReader *dt;
    bool debug;
    int epochs;
    string mainpath;


    vector<vector<float>> previous_input;
    vector<float> previous_output;
    vector<float> new_output;



public :
    neuron  (vector<float> wieghts, double alpha, int epochs, int batchSize, float th, DatasetReader * dt, bool debug);

public :
    float Sigmoid(float x );

public :
    float predict (vector <float> r, bool test);


public :
    vector<float> predict_batch (vector<vector<float>> XB, bool test);


public :
    vector<float> miniBatchGrad( vector<float> ypred,   vector<float> ytrue );

public :
    void train (vector<vector<float>> XB, vector<float> ytrue);

public:
    float dotprod ( vector <float> a , vector < float> b );

};
#endif //DAPPLE_NEURON_H
