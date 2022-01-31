//
// Created by rania on 12/03/19.
//

#ifndef DAPPLE_LR_H
#define DAPPLE_LR_H
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

class LR{

public :
    double alpha;
    int batchSize;
    vector<float> w;
    float th;
    DatasetReader *dt;
    double trainTime;
    double testTime;
    mpz_class  extTrainBd;
    mpz_class extTestBd;
    double clientTestTime;
    bool debug;
    sockaddr_in cliaddr;
    int sockfd;
    string logfile;
    int epochs;
    string mainpath;
    int sgdWorkers;



public :
    LR  (vector<float> wieghts, double alpha, int epochs, int batchSize, float th, DatasetReader * dt, string logfile, sockaddr_in cliaddr, int sockfd, bool debug, string mainpath, int sgdWorkers );

public :
    float Sigmoid(float x );

public :
    int predict (Record *r, bool test);

public :
    vector<float> miniBatchGrad(vector<Record*> XB);

public :
    void train ();

public :
    void Test();

public :
    void RemoteTest();

public :
    int sendMsg(string msg, int sockfd, sockaddr_in cliaddr);

public :
    string recvMsg(int sockfd,  sockaddr_in cliaddr);

public:
    Record *recvRecord(int sockfd, sockaddr_in cliaddr, int dim, int  id);

public:
    void sendResponse(int sockfd, sockaddr_in cliaddr, int res);

public:
    void shuffle(DatasetReader * dt);

};
#endif //DAPPLE_LR_H
