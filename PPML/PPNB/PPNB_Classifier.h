#ifndef DAPPLE_PPNB_CLASSIFIER_H
#define DAPPLE_PPNB_CLASSIFIER_H

#include "../IO/PPDatasetReader.h"
#include "../../UTIL/math_util.hh"
#include "PPStats.h"
#include "math.h"


class PPNB_Classifier {

public :

    PPDatasetReader * datasetReader;
    PPStats* currentModel;
    PPStats* oldModel;
    string logfile;
    bool debug;
    mpz_class  extTrainBd;
    mpz_class extTestBd;
    double trainTime;
    double testTime;
    double clientTestTime;
    DTPKC dtpkc;
    Cipher laplaceConstant;
    int sockfd;
    sockaddr_in cliaddr;

    static double trainTimeSU;
    static double trainTimeMU;
    static double trainBdw;

    static double testTimeSU;
    static double testTimeMU;
    static double testBdw;


    static int addCtr, cmpCtr, divCtr, logCtr;
    static double addTime, cmpTime, divTime, logTime;





public:
    PPNB_Classifier(PPDatasetReader *datasetReader, PPStats *stats, int laplaceConstant, string log, int sockfd, sockaddr_in cliaddr, bool debug);


public:
~PPNB_Classifier()
    {
        delete currentModel;
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
