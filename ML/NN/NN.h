//
// Created by rania on 12/03/19.
//

#ifndef DAPPLE_NN_H
#define DAPPLE_NN_H
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
#include "neuron.h"

using namespace std;

class NN{

public :
    double alpha;
    int batchSize;
    vector<vector<neuron*>> network;

    vector<int> network_dimensions = {14,12,6,2};

    float th;




    DatasetReader *dt;
    double trainTime;
    double testTime;
    mpz_class  extTrainBd;
    mpz_class extTestBd;
    double clientTestTime;
    bool debug;
    string logfile;
    int epochs;
    string mainpath;



public :
    NN  (double alpha, int epochs, int batchSize, float th, DatasetReader * dt, string logfile, bool debug, string mainpath);

public :
    vector<int> predict(vector<Record *>R, bool test );


public :
    vector<vector<float>> backpropagate_layer(vector<neuron*> layer, vector<vector<float>> ytrue);


public :
    void backpropagate(vector<Record *> XB);

public :
    vector<vector<float>> forward_layer(vector<neuron*> layer, vector<vector<float>> x, bool test, bool first = false);

public :
    void train ();

public :
    void Test();


};
#endif //DAPPLE_NN_H
