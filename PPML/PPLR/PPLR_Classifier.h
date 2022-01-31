#ifndef DAPPLE_PPLR_CLASSIFIER_H
#define DAPPLE_PPLR_CLASSIFIER_H

#include "../IO/PPDatasetReader.h"
#include "../../UTIL/math_util.hh"
#include "../../CRYPTO/EvalSigmoid.h"
#include "../../CRYPTO/EvalDotProd.h"
#include "../../CRYPTO/EvalLrUpdate.h"
#include "../../CRYPTO/EvalSub2.h"
#include "math.h"


class PPLR_Classifier {

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
    int sockfd;
    sockaddr_in cliaddr;
    string mainpath;
    int alpha;
    int batchSize;
    vector<Cipher> w;
    Cipher th;
    PPDatasetReader *dt;
    int epochs;
    Cipher zero;
    Cipher one;


    static double trainTimeSU;
    static double trainTimeMU;
    static double trainBdw;


    static double testTimeSU;
    static double testTimeMU;
    static double testBdw;

    static int cmpCtr, updateLrCtr, subCtr, dotProdCtr, sigmoidCtr ;

    static double cmpTime, updateLrTime, subTime, dotProdTime, sigmoidTime ;

public :
    void shuffle(PPDatasetReader * dt);

public :
    Cipher Sigmoid(Cipher x);

public :
    Cipher predict(PPRecord *r, bool test );

public :
    PPLR_Classifier(vector<Cipher> wieghts, int alpha, int epochs, int batchSize, int th, PPDatasetReader *datasetReader, string log, int sockfd, sockaddr_in cliaddr, bool debug, string mainpath);

public :
    vector<Cipher> miniBatchGrad(vector<PPRecord *> XB);

public:
    Cipher dotProduct(vector<Cipher> w, PPRecord * r, int size );

public:
~PPLR_Classifier()
    {

    }

public :
    void train ();

public :
    void Test();

public :
    void RemoteTest();


public :
PPRecord * recvPPRecord(int sockfd, sockaddr_in cliaddr, int dim, DTPKC dtpkc, int  id);

public:
void sendResponse(int sockfd, sockaddr_in cliaddr, Cipher res);

public:
int sendMsg(string msg, int sockfd, sockaddr_in cliaddr);


public :
string recvMsg(int sockfd,  sockaddr_in cliaddr);


};
#endif //DAPPLE_VFDT_CLASSIFIER_H
