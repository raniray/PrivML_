#ifndef DAPPLE_VFDT_CLASSIFIER_H
#define DAPPLE_VFDT_CLASSIFIER_H

#include <netinet/in.h>
#include "../IO/DatasetReader.h"
#include "Node.h"
#include "../../UTIL/math_util.hh"
#include "math.h"


class VFDT_Classifier {

public :

    Node *Tree;
    DatasetReader * datasetReader;
    float delta, tau;
    int grace, max_depth;
    int thresholdNumber;
    string logfile;
    double trainTime;
    double testTime;
    mpz_class  extTrainBd;
    mpz_class extTestBd;
    double trainErr;
    double clientTestTime;
    int treeDepth;
    int numberSplits;
    int numberCriticalZone;
    int modelSize;
    bool debug;
    sockaddr_in cliaddr;
    int sockfd;
    int recordCounter;
    int numberOfNodes;
    int maxNodes;

public:
    VFDT_Classifier(Node* Tree, DatasetReader * datasetReader, float delta, float tau, int grace, int max_depth, int thresholdNUmber, string logfile, sockaddr_in cliaddr, int sockfd, bool debug, int maxNodes);

public :
    double train ();

public :
    void splitNode(Node *n, int attribute, double threshold );

public :
    vector<double> SplittingThresholds(Node *n, int attribute);

public :
    double informationGain(Node *n, int attribute, double thresholds);

public :
    static double Entropy (int S, vector<int> Ck);

public :
    double nodeEntropy (Node * node);

public :
    static double HBound(double A, double B, double  C);

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



};
#endif //DAPPLE_VFDT_CLASSIFIER_H
