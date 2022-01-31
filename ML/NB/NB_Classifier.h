#ifndef DAPPLE_NB_CLASSIFIER_H
#define DAPPLE_NB_CLASSIFIER_H

#include <netinet/in.h>
#include "../IO/DatasetReader.h"
#include "../../UTIL/math_util.hh"
#include "Stats.h"
#include "math.h"


class NB_Classifier {

public :

    DatasetReader * datasetReader;
    Stats* currentModel;
    Stats* newModel;
    string logfile;
    double trainTime;
    double testTime;
    mpz_class  extTrainBd;
    mpz_class extTestBd;
    double trainErr;
    double clientTestTime;
    sockaddr_in cliaddr;
    int sockfd;
    int laplaceConstant;
    bool debug;

public:
    NB_Classifier(DatasetReader * datasetReader, Stats* stats,  int laplaceConstant, string logfile, sockaddr_in cliaddr, int sockfd, bool debug );

public :
    void train ();

public :
    void Test();

public :
    void RemoteTest();
public:
    Record*recvRecord(int sockfd, sockaddr_in cliaddr, int dim, int  id);

public:
    void sendResponse(int sockfd, sockaddr_in cliaddr, int res);

public:
    int sendMsg(string msg, int sockfd, sockaddr_in cliaddr);

public:
    string recvMsg(int sockfd,  sockaddr_in cliaddr);



};
#endif //DAPPLE_VFDT_CLASSIFIER_H
