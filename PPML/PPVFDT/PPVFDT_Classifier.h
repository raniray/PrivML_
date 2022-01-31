//
// Created by rania on 13/11/18.
//

#ifndef DAPPLE_PPVFDT_CLASSIFIER_H
#define DAPPLE_PPVFDT_CLASSIFIER_H

#include <netinet/in.h>
#include "../IO/PPDatasetReader.h"
#include "PPNode.h"
#include "../../UTIL/math_util.hh"
#include "math.h"
#include "../../CRYPTO/DTPKC.h"
#include "../../CRYPTO/EvalEntropy.h"
#include "../../CRYPTO/EvalHBound.h"
#include "../../CRYPTO/EvalMult.h"
#include "../../CRYPTO/EvalAddPow2.h"
#include "../../CRYPTO/EvalStandardDiv.h"
#include "../../CRYPTO/EvalDiv.h"
#include "../../CRYPTO/EvalDiv100.h"
#include "../../CRYPTO/EvalAdd.h"
#include "../../CRYPTO/EvalSub.h"
#include "../../CRYPTO/EvalThreashold.h"

class PPVFDT_Classifier {

public :

    PPNode *Tree;
    static PPDatasetReader * datasetReader;
    static float delta;
    static mpz_class tau;
    static int grace, max_depth;
    static int thresholdNumber;
    static string logfile;
    static DTPKC dtpkc;
    static Cipher zero, one;
    static double trainTime;
    static double testTime;
    static mpz_class  extTrainBd;
    static mpz_class extTestBd;
    static double trainErr;
    static double clientTestTime;
    static int treeDepth;
    static double timeSU;
    static double timeMU;
    static double bdwSU;
    static double bdwMU;
    int numberSplits;
    int numberCriticalZone;
    int sockfd;
    sockaddr_in cliaddr;
    bool debug;
    int CounterZero;
    int recordCounter;
    int maxNodes;
    int numberOfNodes;



    static double trainTimeSU;
    static double trainTimeMU;
    static double trainBdw;

    static double testTimeSU;
    static double testTimeMU;
    static double testBdw;

    static int addCtr, cmpCtr, divCtr, addPowCtr, stdCtr, evalThCtr, subCtr, multCtr, entropyCtr, hboundCtr ;
    static double addTime, cmpTime, divTime, addPowTime, stdTime, evalThTime, subTime, multTime, entropyTime, hboundTime ;

public:
    ~PPVFDT_Classifier()
    {
        delete Tree;
    }

public:
    PPVFDT_Classifier(PPNode* Tree, PPDatasetReader * datasetReader, float delta, mpz_class tau, int grace, int max_depth, int thresholdNumber, DTPKC dtpkc, string logfile, sockaddr_in cliaddr, int sockfd, bool debug, int maxNodes );

public :
    void train ();

public :
    std::vector<int> splitNode(PPNode *n, int attribute, Cipher threshold );

public :
    vector<Cipher> SplittingThresholds(PPNode *n, int attribute );

public :
    Cipher informationGain(PPNode *n, int attribute, Cipher thresholds);

public :
    Cipher nodeEntropy (PPNode * node);

public :
    static Cipher HBound(Cipher A, Cipher B);

public :
    void Test();

//public :
//    void printStats();

public :
    void RemoteTest();


public :
    int sendMsg(string msg, int sockfd, sockaddr_in cliaddr);

public :
    string recvMsg(int sockfd,  sockaddr_in cliaddr);

public :
    PPRecord *recvPPRecord(int sockfd, sockaddr_in cliaddr, int dim, DTPKC dtpkc, int  id);

public:
    void sendResponse(int sockfd, sockaddr_in cliaddr, Cipher res);

};
#endif //DAPPLE_PPVFDT_CLASSIFIER_H
