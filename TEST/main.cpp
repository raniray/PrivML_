/**
** Entry point
 * Authors : Talbi Rania
**
**/

#include "../PPML/PPNB/PPNB_Classifier.h"
#include "../PPML/PPLR/PPLR_Classifier.h"
#include "../PPML/PPVFDT/PPVFDT_Classifier.h"
#include "../ML/NB/NB_Classifier.h"
#include "../ML/VFDT/VFDT_Classifier.h"
#include "../ML/NN/NN.h"
#include "../LIB/config4cpp/include/config4cpp/Configuration.h"
#include "TESTBuildingBlocks.h"
#include "TESTElementaryOps.h"
#include "TESTpacking.h"
#include "../ML/LR/LR.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_DATE 120
#include <mcheck.h>



config4cpp::Configuration *  cfg = config4cpp::Configuration::create();
const char *     scope = "perf";
const char *     configFile = "../CONFIG/DAPPLE.config";

bool PrivacyPreservation = false;
string MLAlgorithm="NB";
int runs_number = 1;
int thread_number = 1;
bool remoteClient = false;
bool Incremental = false;
bool microBench= false;
string OutputPath="/home/rania/CLionProjects/DAPPLE/EXP/";
string scenarioName="test1";
string mainPath="";
float alpha,LRTH;
int batchSize;
string path="/home/rania/CLionProjects/DAPPLE/DATA/Datasets/Adult/";
string chunk_prefix="DO";
int class_number = 2;
int dimension = 15;
int chunk_number = 8;

float delta = 0.00001;
float tau = 0.1;
int grace = 50;
int max_depth = 10;
int threshold_number= 3 ;
int sgdWorkers =1;
bool sendParaClient=false;
bool optim = false;
bool precompute = false;
int epochs=1;
int callsTrain[14];
int callsPred[14];
double timeTrain[14];
double timePred[14];

int  LaplaceConstant = 1;

bool deserialize= false;
int keySize = 1024;
int DTPKC_Err = 600;
int ComputationalPrecision = 1000;
float useORE= false ;
int blindVal =100;
sockaddr_in cliaddr ={0};
bool encryptData=false;
bool testBuildingBlock = false;
int maxNodes=0;

int portS = 5003;
string serverIP = "127.0.0.1";

double P=0.0, R=0.0, F1=0.0;


mpz_class pkey, skey;


bool debug= false ;
int maxTrainSize= 10 ;
int maxTestSize= 10;
string clearPath="";
string pathSer ="";

int delay;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int validateCrypto(DTPKC dtpkc)
{
    TESTBuildingBlocks testBuildingBlocks;
    int cpt = testBuildingBlocks.validate(dtpkc);
    return cpt;
}


void connect (string serverIP, int portS, int & sockfd,sockaddr_in &cliaddr)
{
    // Create socket
    sockaddr_in  servaddr;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));


    cliaddr.sin_family    = AF_INET; // IPv4
    cliaddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    cliaddr.sin_port = htons(portS);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (debug) cout << "Server is binded to socket correctly" << endl;

    socklen_t addrlen = sizeof(cliaddr);
    char buf[200];

    // Recieving connect message from thge client
    int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);


}


void evaluateAccuracy(string datasetPath, string mainPath, string outputPath, string classOutput, int test_size, double &P, double &R, double &F1)
{
    std::string filename = mainPath+ "PPML/EVAL/EvaluateClassification.py ";
    std::string command = "python3 ";
    std::string arguments =datasetPath+"result.data "+outputPath+ " "+ classOutput+" "+to_string(test_size);
    command += filename+ arguments;
    system(command.c_str());
    std::string output = outputPath+"Report-"+classOutput;
    ifstream infile(output);

    string t;
    getline(infile,t);
    F1 = atof(t.c_str());
    getline(infile,t);
    P= atof(t.c_str());
    getline(infile,t);
    R = atof(t.c_str());
    infile.close();
}



int main (int argc, char **argv) {

    for (int cp=0; cp<14; cp++)
    {
        callsTrain[cp]=0;
        callsPred[cp]=0;
    }



    try {

        /**Experiments parameters**/

        if (argc > 2) {
            thread_number = atoi(argv[1]);
            configFile  =  argv[2];

        }

        cfg->parse(configFile);
        testBuildingBlock = cfg->lookupBoolean(scope, "testBuildingBlocks");
        PrivacyPreservation = cfg->lookupBoolean(scope, "PrivacyPreservation");
        microBench = cfg->lookupBoolean(scope, "microBench");
        MLAlgorithm = cfg->lookupString(scope, "MLAlgorithm");
        runs_number = cfg->lookupInt(scope, "runs_number");
        //thread_number = cfg->lookupInt(scope, "thread_number");
        remoteClient = cfg->lookupBoolean(scope, "remoteClient");
        Incremental = cfg->lookupBoolean(scope, "Incremental");
        OutputPath = cfg->lookupString(scope, "OutputPath");
        scenarioName = cfg->lookupString(scope, "scenarioName");

        debug = cfg->lookupBoolean(scope, "debug");
        maxTrainSize = cfg->lookupInt(scope, "maxTrainSize");
        maxTestSize = cfg->lookupInt(scope, "maxTestSize");

        /** dataset**/

        scope = "dataset";
        path = cfg->lookupString(scope, "path");
        mainPath = cfg->lookupString(scope, "mainPath");
        chunk_prefix = cfg->lookupString(scope, "chunk_prefix");
        dimension = cfg->lookupInt(scope, "dimension");
        chunk_number = cfg->lookupInt(scope, "chunk_number");
        class_number = cfg->lookupInt(scope, "class_number");
        clearPath = cfg->lookupString(scope, "clearPath");



        /**vfdt**/

        scope = "vfdt";
        delta = cfg->lookupFloat(scope, "delta");
        tau = cfg->lookupFloat(scope, "tau");
        grace = cfg->lookupInt(scope, "grace");
        max_depth = cfg->lookupInt(scope, "max_depth");
        threshold_number = cfg->lookupInt(scope, "threshold_number");
        maxNodes = cfg->lookupInt(scope, "maxNodes");

        /**NB**/
        scope = "nb";
        LaplaceConstant = cfg->lookupInt(scope, "LaplaceConstant");

        /**LR**/
        scope="lr";
        alpha = cfg->lookupFloat(scope, "alpha");
        LRTH = cfg->lookupFloat(scope, "LRTH");
        batchSize = cfg->lookupInt(scope, "batchSize");
        epochs = cfg->lookupInt(scope,"epochs");
        sgdWorkers = cfg->lookupInt(scope, "sgdWorkers");

        /**Crypto**/

        scope = "crypto";
        deserialize = cfg->lookupBoolean(scope, "deserialize");
        keySize = cfg->lookupInt(scope, "keySize");
        DTPKC_Err = cfg->lookupInt(scope, "DTPKC_Err");
        ComputationalPrecision = cfg->lookupInt(scope, "ComputationalPrecision");
        useORE = cfg->lookupBoolean(scope, "useORE");
        pathSer = cfg->lookupString(scope, "pathSer");
        sendParaClient = cfg->lookupBoolean(scope, "sendParaClient");
        blindVal = cfg->lookupInt(scope, "blindingSize");
        encryptData = cfg->lookupBoolean(scope, "encryptData");
        delay = cfg->lookupInt(scope, "delay");
        precompute = cfg->lookupBoolean(scope, "precompute");
        optim = cfg->lookupBoolean(scope, "optim");

        /**Network**/
        scope = "network";

        serverIP = cfg->lookupString(scope, "serverIP");
        portS = cfg->lookupInt(scope, "portS");


    } catch (const config4cpp::ConfigurationException &ex) {
        cerr << ex.c_str() << endl;
        cfg->destroy();
    }


    cfg->destroy();


    //cout << thread_number << endl;

    int sockfd=0;


    if (remoteClient) {

        connect(serverIP, portS, sockfd, cliaddr);

    }


    if (debug) cout << "Config file read " << endl;

    string logfile = "";
    logfile = logfile + OutputPath.c_str() + scenarioName.c_str() + "-";

    time_t now;
    char the_date[MAX_DATE];
    the_date[0] = '\0';
    now = time(NULL);

    if (now != -1) {
        strftime(the_date, MAX_DATE, "%Y-%m-%d-%X", gmtime(&now));
    }

    logfile = logfile + the_date;
    string classOutput = scenarioName + "-" + the_date + "-ClassOutput.txt";

    std::ofstream stats;
    stats.open(logfile + "-Stats.csv");
    std::ofstream config;
    config.open(logfile + "-Config.txt");
    ifstream infile(configFile);

    string content = "";
    for (int i = 0; infile.eof() != true; i++) // get content of infile
        content += infile.get();

    content.erase(content.end() - 1);     // erase last character
    infile.close();
    config << content;
    config.close();
    config.close();

    omp_set_dynamic(0);
    omp_set_num_threads(thread_number);

    if (!testBuildingBlock) {
        if (PrivacyPreservation) {
            gmp_randstate_t randstate;
            DTPKC dtpkc;

            if (deserialize) {
                dtpkc.deserializeDtpkc(pathSer);
                std::chrono::milliseconds delay_(delay);
                dtpkc.delay=delay_;
                dtpkc.blindVal = blindVal;
                if (debug) cout << "DTPKC parameters deserialized " << endl;


            } else {
                // Generate public parameters of DTPKC
                gmp_randinit_default(randstate);
                gmp_randseed_ui(randstate, time(NULL));
                std::chrono::milliseconds delay_(delay);
                int cpt = 0;
                do {
                    dtpkc.keygen(ComputationalPrecision, randstate, delay_, keySize, DTPKC_Err, optim, blindVal);

                    dtpkc.getKey(pkey, skey);
                    dtpkc.updatePkw(pkey);
                    cpt = validateCrypto(dtpkc);

                } while (cpt > 1);

                dtpkc.serializeDtpkc(pathSer);

                if (debug) cout << "DTPKC parameters generated " << endl;

            }

            if (sendParaClient && remoteClient) {
                dtpkc.sendPara(sockfd, cliaddr);
            }

            PPDatasetReader *datasetReader = new PPDatasetReader(dtpkc);
            if (debug) cout << "Dataset read " << endl;

            if (encryptData) {
                // encrypt data
                if (Incremental) {
                    for (int ch = 0; ch < chunk_number; ch++) {
                        datasetReader->encryptData(clearPath + "DO_" + std::to_string(ch + 1) + ".data",
                                                   path + "DO_" + std::to_string(ch + 1) + ".enc", pkey);
                        if (debug)
                            cout << "File " << clearPath + "DO_" + std::to_string(ch + 1) + ".enc" << " generated"
                                 << endl;
                    }


                } else {

                    datasetReader->encryptData(clearPath + "training.data", path + "training.enc", pkey);
                    if (debug) cout << "File " << clearPath + "training.enc" << " generated" << endl;

                }

                datasetReader->encryptData(clearPath + "testing.data", path + "testing.enc", pkey);
                if (debug) cout << "File" << path + "testing.enc" << " generated" << endl;
            }


            try {

                delete datasetReader;

            } catch (exception e) {
                cout << "Trying to delete dataset reader  " << endl;
                cout << e.what() << endl;
            }
            datasetReader = new PPDatasetReader(path, Incremental, chunk_prefix, class_number, dimension, dtpkc, debug,
                                                maxTrainSize, maxTestSize);

            if (MLAlgorithm == "vfdt") {

                for (int i = 0; i < runs_number; i++) {
                    if (debug) cout << "Run : " << i << "Over " << runs_number << endl;
                    stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;
                    PPNode *Tree = new PPNode(datasetReader->dimension - 1, datasetReader->classNumber);
                    PPVFDT_Classifier *classifier = new PPVFDT_Classifier(Tree, datasetReader, delta, tau, grace,
                                                                          max_depth, threshold_number, dtpkc,
                                                                          OutputPath + classOutput, cliaddr, sockfd,
                                                                          debug, maxNodes);

                    if (Incremental && !microBench) {
                        stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1,NumberOfNodes,TreeDepth, numberSplits, TrainTimeMU, TrainTimeSU, TrainBdw, testTimeMU, testTimeSU, testBdw"
                                << endl;
                        for (int ch = 0; ch < chunk_number; ch++) {

                            if (debug)
                                cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;
                            classifier->train();
                            datasetReader->nextChunk();
                            //cout << "Ended training on chunk n°: " << ch+1 << endl;
                            if (remoteClient) {

                                classifier->sendMsg("start", sockfd, cliaddr);
                                string msg = classifier->recvMsg(sockfd, cliaddr);

                                if (msg == "ok") {
                                    if (debug)
                                        cout << "Started remote testing after update n°: " << ch + 1 << " from: "
                                             << chunk_number << endl;
                                    classifier->RemoteTest();
                                    if (debug) cout << " Remote test " << ch + 1 << "done" << endl;
                                }

                            } else {
                                if (debug)
                                    cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                         << endl;
                                classifier->Test();
                                if (debug) cout << "Test " << ch + 1 << " done" << endl;
                            }
                            //cout << "Client Test Time " << c << endl;
                            evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                             R, F1);

                            stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                     "," + std::to_string(classifier->testTime) + "," +
                                     std::to_string(classifier->clientTestTime) + "," +
                                     classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                     std::to_string(R) + "," + std::to_string(F1) + "," +
                                     std::to_string(classifier->numberOfNodes) + "," +
                                     std::to_string(classifier->treeDepth) + "," +
                                     std::to_string(classifier->numberSplits) + "," +
                                     std::to_string(classifier->trainTimeMU) + "," +
                                    std::to_string(classifier->trainTimeSU) + "," +
                                    std::to_string(classifier->trainBdw) + "," +
                                    std::to_string(classifier->testTimeMU) + "," +
                                    std::to_string(classifier->testTimeSU) + "," +
                                    std::to_string(classifier->testBdw)

                                     << endl;

                            datasetReader->reloadTest();
                        }
                    } else {


                        if (microBench)
                        {

                            stats << "0_SE, 1_STS, 2_SHBC, 3_SSig, 4_SDP, 5_SD, 6_SM, 7_SC, 8_Slog, 9_SExpo, 10_SSqrt, 11_SPU, 12_SAdd, 13_Ssub, 0_SE_Time, 1_STS_Time, 2_SHBC_Time, 3_SSig_Time, 4_SDP_Time, 5_SD_Time, 6_SM_Time, 7_SC_Time, 8_Slog_Time, 9_SExpo_Time, 10_SSqrt_Time, 11_SPU_Time, 12_SAdd_Time, 13_Ssub_Time" << endl;

                            classifier->train();

                            callsTrain[12] =classifier->addCtr;
                            callsTrain[7] =classifier->cmpCtr;
                            callsTrain[5] =classifier->divCtr;
                            callsTrain[6] += classifier->addPowCtr;
                            callsTrain[10] =classifier->stdCtr;
                            callsTrain[1] = classifier->evalThCtr;
                            callsTrain[13] =classifier->subCtr;
                            callsTrain[6] +=classifier->multCtr;
                            callsTrain[0] =classifier->entropyCtr;
                            callsTrain[2] =classifier->hboundCtr ;


                            timeTrain[12] =classifier->addTime;
                            timeTrain[7] =classifier->cmpTime;
                            timeTrain[5] =classifier->divTime;
                            timeTrain[6] += classifier->addPowTime;
                            timeTrain[10] =classifier->stdTime;
                            timeTrain[1] = classifier->evalThTime;
                            timeTrain[13] =classifier->subTime;
                            timeTrain[6] +=classifier->multTime;
                            timeTrain[0] =classifier->entropyTime;
                            timeTrain[2] =classifier->hboundTime ;

                            classifier->Test();


                            callsPred[7] =classifier->cmpCtr;

                            timePred[7] =classifier->cmpTime;

                            string trainStats = "";
                            string testStats= "";
                            for (int cp=0; cp <14; cp++ )
                            {
                                trainStats += std::to_string(callsTrain[cp]) +",";
                                testStats += std::to_string(callsPred[cp]) +",";

                            }

                            for (int cp=0; cp <14; cp++ )
                            {
                                trainStats += std::to_string(timeTrain[cp]) +",";
                                testStats += std::to_string(timePred[cp]) +",";

                            }

                            stats << trainStats << endl;
                            stats << testStats << endl;


                        }else{
                            if (debug) cout << "Training started" << endl;
                            classifier->train();
                            if (debug) cout << "Training done" << endl;

                            if (remoteClient) {
                                classifier->sendMsg("start", sockfd, cliaddr);
                                if (debug) cout << " Remote testing started " << endl;
                                classifier->RemoteTest();
                                if (debug) cout << " Remote testing done " << endl;

                            } else {
                                if (debug) cout << " Testing started " << endl;
                                classifier->Test();
                                if (debug) cout << " Testing done " << endl;
                            }
                        }


                    }
                    evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                    R, F1);

                    stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                             "," + std::to_string(classifier->testTime) + "," +
                             std::to_string(classifier->clientTestTime) + "," +
                             classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                             std::to_string(R) + "," + std::to_string(F1) + "," +
                             std::to_string(classifier->numberOfNodes) + "," +
                             std::to_string(classifier->treeDepth) + "," +
                             std::to_string(classifier->numberSplits) + "," +
                            std::to_string(classifier->trainTimeMU) + "," +
                            std::to_string(classifier->trainTimeSU) + "," +
                            std::to_string(classifier->trainBdw) + "," +
                            std::to_string(classifier->testTimeMU) + "," +
                            std::to_string(classifier->testTimeSU) + "," +
                            std::to_string(classifier->testBdw)

                          << endl;
                    datasetReader->closeTraining();
                    datasetReader->closeTesting();
                    datasetReader->reinit();

                    if (i == (runs_number - 1) && remoteClient) {
                        classifier->sendMsg("stop", sockfd, cliaddr);
                        close(sockfd);

                    }
                    try {

                        delete classifier;

                    } catch (exception e) {
                        cout << "Trying to delete classifier " << endl;
                        cout << e.what() << endl;
                    }

                }

                datasetReader->closeTraining();
                datasetReader->closeTesting();
                delete datasetReader;

            } else if (MLAlgorithm == "nb") {




                for (int i = 0; i < runs_number; i++) {
                    if (debug) cout << "Run : " << i << "Over " << runs_number << endl;
                    stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;

                    PPStats *S = new PPStats(class_number, dimension - 1);
                    PPNB_Classifier *classifier = new PPNB_Classifier(datasetReader, S, LaplaceConstant,
                                                                      OutputPath + classOutput, sockfd, cliaddr, debug);
                    stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1,TrainMU,TrainSU,TrainBdw,TestMU,TestSU,TestBdw" << endl;
                    if (Incremental) {

                        for (int ch = 0; ch < chunk_number; ch++) {
                            if (debug)
                                cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;

                                classifier->train();

                                double t1=0.0, t2=0.0;
                            t1 = PPNB_Classifier::addCtr * 0.00092 + (PPNB_Classifier::cmpCtr+DTPKC::cmpCtr)*6.39 + PPNB_Classifier::divCtr*9.15 + PPNB_Classifier::logCtr*9.62;
                            t2 =   PPNB_Classifier::divCtr*9 + PPNB_Classifier::logCtr*6.01 + (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr)*6.56;
                            PPNB_Classifier::trainTimeMU = t1/(t1+t2);
                            PPNB_Classifier::trainTimeSU = t2/(t1+t2);

                            datasetReader->nextChunk();
                            if (remoteClient) {

                                classifier->sendMsg("start", sockfd, cliaddr);
                                string msg = classifier->recvMsg(sockfd, cliaddr);

                                if (msg == "ok") {
                                    if (debug)
                                        cout << "Started remote testing after update n°: " << ch + 1 << " from: "
                                             << chunk_number << endl;
                                    classifier->RemoteTest();
                                    if (debug) cout << " Remote test " << ch + 1 << "done" << endl;
                                }

                            } else {
                                if (debug)
                                    cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                         << endl;
                                classifier->Test();
                                if (debug) cout << "Test " << ch + 1 << " done" << endl;
                            }
                            evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                             R, F1);


                           t1 = PPNB_Classifier::addCtr * 0.00092 + (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr )*6.39;
                           t2 = (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr)*6.56;

                            PPNB_Classifier::testTimeMU = t1/(t1+t2);
                            PPNB_Classifier::testTimeSU = t2/(t1+t2);



                            stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                     "," + std::to_string(classifier->testTime) + "," +
                                     std::to_string(classifier->clientTestTime) + "," +
                                     classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                     std::to_string(R) + "," + std::to_string(F1) + "," + std::to_string(PPNB_Classifier::trainTimeMU) + "," + std::to_string(PPNB_Classifier::trainTimeSU)  +","+ std::to_string(PPNB_Classifier::trainBdw) + "," +
                                    std::to_string(PPNB_Classifier::testTimeMU) +"," + std::to_string(PPNB_Classifier::testTimeSU) +"," + std::to_string(PPNB_Classifier::testBdw)<< endl;
                            classifier->trainTime = 0;
                            classifier->testTime = 0 ;
                            datasetReader->reloadTest();
                        }
                    } else {


                        if (microBench)
                        {

                            stats << "0_SE, 1_STS, 2_SHBC, 3_SSig, 4_SDP, 5_SD, 6_SM, 7_SC, 8_Slog, 9_SExpo, 10_SSqrt, 11_SPU, 12_SAdd, 13_Ssub, 0_SE_Time, 1_STS_Time, 2_SHBC_Time, 3_SSig_Time, 4_SDP_Time, 5_SD_Time, 6_SM_Time, 7_SC_Time, 8_Slog_Time, 9_SExpo_Time, 10_SSqrt_Time, 11_SPU_Time, 12_SAdd_Time, 13_Ssub_Time" << endl;

                            classifier->train();

                            callsTrain[12] =classifier->addCtr;
                            callsTrain[7] =classifier->cmpCtr;
                            callsTrain[5] =classifier->divCtr;
                            callsTrain[8] = classifier->logCtr;

                            timeTrain[12] =classifier->addTime;
                            timeTrain[7] =classifier->cmpTime;
                            timeTrain[5] =classifier->divTime;
                            timeTrain[8] = classifier->logTime;


                            classifier->Test();

                            callsPred[12] =classifier->addCtr;
                            callsPred[7] =classifier->cmpCtr;

                            timePred[12] =classifier->addTime;
                            timePred[7] =classifier->cmpTime;

                            string trainStats = "";
                            string testStats= "";
                            for (int cp=0; cp <14; cp++ )
                            {
                                trainStats += std::to_string(callsTrain[cp]) +",";
                                testStats += std::to_string(callsPred[cp]) +",";

                            }

                            for (int cp=0; cp <14; cp++ )
                            {
                                trainStats += std::to_string(timeTrain[cp]) +",";
                                testStats += std::to_string(timePred[cp]) +",";

                            }

                            stats << trainStats << endl;
                            stats << testStats << endl;


                        }else {

                            stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1,TrainMU,TrainSU,TrainBdw,TestMU,TestSU,TestBdw" << endl;
                            double t1, t2;
                            if (debug) cout << "Training started" << endl;
                            classifier->train();
                            t1 = PPNB_Classifier::addCtr * 0.00092 + (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr) * 6.39 +
                                 PPNB_Classifier::divCtr * 9.15 + PPNB_Classifier::logCtr * 9.62;
                            t2 = PPNB_Classifier::divCtr * 9 + PPNB_Classifier::logCtr * 6.01 +
                                 (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr) * 6.56;
                            PPNB_Classifier::trainTimeMU = t1 / (t1 + t2);
                            PPNB_Classifier::trainTimeSU = t2 / (t1 + t2);
                            if (debug) cout << "Training done" << endl;

                            if (remoteClient) {
                                classifier->sendMsg("start", sockfd, cliaddr);
                                if (debug) cout << " Remote testing started " << endl;
                                classifier->RemoteTest();
                                if (debug) cout << " Remote testing done " << endl;

                            } else {
                                if (debug) cout << " Testing started " << endl;
                                classifier->Test();
                                if (debug) cout << " Testing done " << endl;
                            }


                        }

                        double t1, t2;
                        evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                         R, F1);

                        t1 = PPNB_Classifier::addCtr * 0.00092 + (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr )*6.39;
                        t2 = (PPNB_Classifier::cmpCtr + DTPKC::cmpCtr)*6.56;

                        PPNB_Classifier::testTimeMU = t1/(t1+t2);
                        PPNB_Classifier::testTimeSU = t2/(t1+t2);
                        stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                 "," + std::to_string(classifier->testTime) + "," +
                                 std::to_string(classifier->clientTestTime) + "," +
                                 classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                 std::to_string(R) + "," + std::to_string(F1) + "," + std::to_string(PPNB_Classifier::trainTimeMU) + "," + std::to_string(PPNB_Classifier::trainTimeSU)  +","+ std::to_string(PPNB_Classifier::trainBdw) + "," +
                                                           std::to_string(PPNB_Classifier::testTimeMU) +"," + std::to_string(PPNB_Classifier::testTimeSU) +"," + std::to_string(PPNB_Classifier::testBdw)<< endl;

                    }



                    datasetReader->closeTraining();
                    datasetReader->closeTesting();
                    datasetReader->reinit();
                    try {

                        //delete S;

                    } catch (exception e) {
                        cout << "Trying to delete Tree " << endl;
                        cout << e.what() << endl;
                    }

                    try {

                        delete classifier;

                    } catch (exception e) {
                        cout << "Trying to delete classifier " << endl;
                        cout << e.what() << endl;
                    }

                    if (i == (runs_number - 1) && remoteClient) {
                        classifier->sendMsg("stop", sockfd, cliaddr);
                        close(sockfd);

                    }

                }

                datasetReader->closeTraining();
                datasetReader->closeTesting();
                delete datasetReader;

            }else{

                datasetReader = new PPDatasetReader(path, Incremental, chunk_prefix, class_number, dimension, dtpkc, debug,
                                                    maxTrainSize, maxTestSize, true);
                for (int i = 0; i < runs_number; i++) {
                    if (debug) cout << "Run : " << i << "Over " << runs_number << endl;
                    stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;

                    vector<Cipher> w;
                    Cipher zero = dtpkc.enc(0, dtpkc.pkw);
                    for (int k=0; k < datasetReader->dimension; k ++ )
                        w.push_back(zero);

                    PPLR_Classifier *classifier = new PPLR_Classifier (w,(int)(alpha/batchSize),epochs, batchSize, (int)(LRTH*100), datasetReader, OutputPath + classOutput,
                                             sockfd, cliaddr, debug, mainPath);

                    stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1,trainMU,trainSU,trainBdw,testMU,testSU,testBdw"<< endl;

                    if (Incremental) {


                        for (int ch = 0; ch < chunk_number; ch++) {

                            if (debug)
                                cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;
                            classifier->train();
                            datasetReader->nextChunk();
                            //cout << "Ended training on chunk n°: " << ch+1 << endl;
                            if (remoteClient) {

                                classifier->sendMsg("start", sockfd, cliaddr);
                                string msg = classifier->recvMsg(sockfd, cliaddr);

                                if (msg == "ok") {
                                    if (debug)
                                        cout << "Started remote testing after update n°: " << ch + 1 << " from: "
                                             << chunk_number << endl;
                                    classifier->RemoteTest();
                                    if (debug) cout << " Remote test " << ch + 1 << "done" << endl;
                                }

                            } else {
                                if (debug)
                                    cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                         << endl;
                                classifier->Test();
                                if (debug) cout << "Test " << ch + 1 << " done" << endl;
                            }
                            //cout << "Client Test Time " << c << endl;
                            evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                             R, F1);

                            stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                     "," + std::to_string(classifier->testTime) + "," +
                                     std::to_string(classifier->clientTestTime) + "," +
                                     classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                     std::to_string(R) + "," + std::to_string(F1) +","+
                                     std::to_string(classifier->trainTimeMU) +","+
                                     std::to_string(classifier->trainTimeSU) +","+
                                     std::to_string(classifier->trainBdw) +","+
                                     std::to_string(classifier->testTimeMU) +","+
                                    std::to_string(classifier->testTimeSU) +","+
                                    std::to_string(classifier->testBdw)

                                    << endl;

                            datasetReader->reloadTest();
                        }
                    } else {

                        if(microBench)
                        {


                            stats << "0_SE, 1_STS, 2_SHBC, 3_SSig, 4_SDP, 5_SD, 6_SM, 7_SC, 8_Slog, 9_SExpo, 10_SSqrt, 11_SPU, 12_SAdd, 13_Ssub, 0_SE_Time, 1_STS_Time, 2_SHBC_Time, 3_SSig_Time, 4_SDP_Time, 5_SD_Time, 6_SM_Time, 7_SC_Time, 8_Slog_Time, 9_SExpo_Time, 10_SSqrt_Time, 11_SPU_Time, 12_SAdd_Time, 13_Ssub_Time" << endl;


                            classifier->train();

                            callsTrain[7] =classifier->cmpCtr;
                            callsTrain[4] +=classifier->updateLrCtr + classifier->dotProdCtr;
                            callsTrain[13] +=classifier->updateLrCtr + classifier->subCtr;
                            callsTrain[3] = classifier->sigmoidCtr;

                            timeTrain[7] =classifier->cmpTime;
                            timeTrain[4] +=classifier->updateLrTime + classifier->dotProdTime;
                            timeTrain[13] +=classifier->updateLrTime + classifier->subTime;
                            timeTrain[3] = classifier->sigmoidTime;


                            classifier->cmpTime = 0.0 ;
                            classifier->dotProdTime = 0.0;
                            classifier->sigmoidTime = 0.0;


                            classifier->cmpCtr = 0 ;
                            classifier->dotProdCtr = 0;
                            classifier->sigmoidCtr = 0;

                            classifier->Test();

                            callsPred[7] = classifier->cmpCtr;
                            callsPred[4] = classifier->dotProdCtr;
                            callsPred[3] = classifier->sigmoidCtr;


                            timePred[7] = classifier->cmpTime;
                            timePred[4] = classifier->dotProdTime;
                            timePred[3] = classifier->sigmoidTime;

                            string trainStats = "";
                            string testStats= "";

                            for (int cp=0; cp <14; cp++ )
                            {
                                trainStats += std::to_string(callsTrain[cp]) +",";
                                testStats += std::to_string(callsPred[cp]) +",";

                            }


                            for (int cp=0; cp <14; cp++ )
                            {
                                trainStats += std::to_string(timeTrain[cp]) +",";
                                testStats += std::to_string(timePred[cp]) +",";

                            }


                            stats << trainStats << endl;
                            stats << testStats << endl;




                        } else {


                            if (debug) cout << "Training started" << endl;
                            classifier->train();
                            if (debug) cout << "Training done" << endl;

                            if (remoteClient) {
                                classifier->sendMsg("start", sockfd, cliaddr);
                                if (debug) cout << " Remote testing started " << endl;
                                classifier->RemoteTest();
                                if (debug) cout << " Remote testing done " << endl;

                            } else {
                                if (debug) cout << " Testing started " << endl;
                                classifier->Test();
                                if (debug) cout << " Testing done " << endl;
                            }
                        }

                        evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                         R, F1);
                        cout << "F1 " << F1 << endl;

                        stats << std::to_string(classifier->trainTime).c_str() << ","
                              << classifier->extTrainBd.get_str() <<
                              "," << std::to_string(classifier->testTime).c_str() << "," <<
                              std::to_string(classifier->clientTestTime).c_str() << "," <<
                              classifier->extTestBd.get_str() << "," << std::to_string(P).c_str() << "," <<
                              std::to_string(R).c_str() << "," << std::to_string(F1).c_str()  << "," <<
                                                                  std::to_string(classifier->trainTimeMU) << "," <<
                                                                  std::to_string(classifier->trainTimeSU) << "," <<
                                                                  std::to_string(classifier->trainBdw) << "," <<
                                                                  std::to_string(classifier->testTimeMU) << "," <<
                                                                  std::to_string(classifier->testTimeSU) << "," <<
                                                                  std::to_string(classifier->testBdw)

                                                               << endl;
                    }
                        datasetReader->closeTraining();
                        datasetReader->closeTesting();
                        datasetReader->reinit();

                    if (i == (runs_number - 1) && remoteClient) {
                        classifier->sendMsg("stop", sockfd, cliaddr);
                        close(sockfd);

                    }
                    try {

                        delete classifier;

                    } catch (exception e) {
                        cout << "Trying to delete classifier " << endl;
                        cout << e.what() << endl;
                    }

                }

                datasetReader->closeTraining();
                datasetReader->closeTesting();
                delete datasetReader;




            }


           if(!deserialize) gmp_randclear(randstate);

        } else {

            if (MLAlgorithm == "vfdt") {

                DatasetReader *datasetReader = new DatasetReader(path, Incremental, chunk_prefix, class_number,
                                                                 dimension);

                for (int i = 0; i < runs_number; i++) {
                    if (debug) cout << "Run : " << i << "Over " << runs_number << endl;
                    stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;
                    Node *Tree = new Node(datasetReader->dimension - 1, datasetReader->classNumber);
                    VFDT_Classifier *classifier = new VFDT_Classifier(Tree, datasetReader, delta, tau, grace, max_depth,
                                                                      threshold_number, OutputPath + classOutput,
                                                                      cliaddr, sockfd, debug, maxNodes);

                    if (Incremental) {
                        stats
                                << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1,NumberOfNodes,TreeDepth, numberSplits, numberCriticalZone, CounterZero"
                                << endl;
                        for (int ch = 0; ch < chunk_number; ch++) {

                            if (debug)
                                cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;
                            classifier->train();
                            datasetReader->nextChunk();
                            //cout << "Ended training on chunk n°: " << ch+1 << endl;
                            if (remoteClient) {

                                classifier->sendMsg("start", sockfd, cliaddr);
                                string msg = classifier->recvMsg(sockfd, cliaddr);

                                if (msg == "ok") {
                                    if (debug)
                                        cout << "Started remote testing after update n°: " << ch + 1 << " from: "
                                             << chunk_number << endl;
                                    classifier->RemoteTest();
                                    if (debug) cout << " Remote test " << ch + 1 << "done" << endl;
                                }

                            } else {
                                if (debug)
                                    cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                         << endl;
                                classifier->Test();
                                if (debug) cout << "Test " << ch + 1 << " done" << endl;
                            }
                            //cout << "Client Test Time " << c << endl;
                            evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                             R, F1);

                            stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                     "," + std::to_string(classifier->testTime) + "," +
                                     std::to_string(classifier->clientTestTime) + "," +
                                     classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                     std::to_string(R) + "," + std::to_string(F1) + "," +
                                     std::to_string(classifier->numberOfNodes) + "," +
                                     std::to_string(classifier->treeDepth) + "," +
                                     std::to_string(classifier->numberSplits) + "," +
                                     std::to_string(classifier->numberCriticalZone) << endl;

                            datasetReader->reloadTest();
                        }
                    } else {
                        if (debug) cout << "Training started" << endl;
                        classifier->train();
                        if (debug) cout << "Training done" << endl;

                        if (remoteClient) {
                            classifier->sendMsg("start", sockfd, cliaddr);
                            if (debug) cout << " Remote testing started " << endl;
                            classifier->RemoteTest();
                            if (debug) cout << " Remote testing done " << endl;

                        } else {
                            if (debug) cout << " Testing started " << endl;
                            classifier->Test();
                            if (debug) cout << " Testing done " << endl;
                        }

                        evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                         R, F1);

                        stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                 "," + std::to_string(classifier->testTime) + "," +
                                 std::to_string(classifier->clientTestTime) + "," +
                                 classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                 std::to_string(R) + "," + std::to_string(F1) + "," +
                                 std::to_string(classifier->numberOfNodes) + "," +
                                 std::to_string(classifier->treeDepth) + "," +
                                 std::to_string(classifier->numberSplits) + "," +
                                 std::to_string(classifier->numberCriticalZone) << endl;
                        datasetReader->closeTraining();
                        datasetReader->closeTesting();
                        datasetReader->reinit();

                    }

                    try {

                        delete Tree;

                    } catch (exception e) {
                        cout << "Trying to delete Tree " << endl;
                        cout << e.what() << endl;
                    }

                    if (i == (runs_number - 1) && remoteClient) {
                        classifier->sendMsg("stop", sockfd, cliaddr);
                        close(sockfd);

                    }

                    try {

                        delete classifier;

                    } catch (exception e) {
                        cout << "Trying to delete classifier " << endl;
                        cout << e.what() << endl;
                    }

                }


                datasetReader->closeTraining();
                datasetReader->closeTesting();
                delete datasetReader;


            } else if (MLAlgorithm == "nb") {
                stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1,TrainMU,TrainSU,TrainBdw,TestMU,TestSU,TestBdw" << endl;

                DatasetReader *datasetReader = new DatasetReader(path, Incremental, chunk_prefix, class_number,
                                                                 dimension);

                for (int i = 0; i < runs_number; i++) {
                    if (debug) cout << "Run : " << i << "Over " << runs_number << endl;
                    stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;

                    Stats *S = new Stats(class_number, dimension - 1);
                    Stats::initialised = false;
                    NB_Classifier *classifier = new NB_Classifier(datasetReader, S, LaplaceConstant,
                                                                  OutputPath + classOutput, cliaddr, sockfd, debug);

                    if (Incremental) {
                        //stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1" << endl;
                        for (int ch = 0; ch < chunk_number; ch++) {

                            classifier->train();

                            if (debug)
                                cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;

                            datasetReader->nextChunk();

                            if (remoteClient) {

                                classifier->sendMsg("start", sockfd, cliaddr);
                                string msg = classifier->recvMsg(sockfd, cliaddr);

                                if (msg == "ok") {
                                    if (debug)
                                        cout << "Started remote testing after update n°: " << ch + 1 << " from: "
                                             << chunk_number << endl;
                                    classifier->RemoteTest();
                                    if (debug) cout << " Remote test " << ch + 1 << "done" << endl;
                                }

                            } else {
                                if (debug)
                                    cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                         << endl;
                                classifier->Test();
                                if (debug) cout << "Test " << ch + 1 << " done" << endl;
                            }

                            evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                             R, F1);
                            stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                     "," + std::to_string(classifier->testTime) + "," +
                                     std::to_string(classifier->clientTestTime) + "," +
                                     classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                     std::to_string(R) + "," + std::to_string(F1) << endl;
                            datasetReader->reloadTest();
                        }
                    } else {
                        if (debug) cout << "Training started" << endl;
                        classifier->train();
                        if (debug) cout << "Training done" << endl;

                        if (remoteClient) {
                            classifier->sendMsg("start", sockfd, cliaddr);
                            if (debug) cout << " Remote testing started " << endl;
                            classifier->RemoteTest();
                            if (debug) cout << " Remote testing done " << endl;

                        } else {
                            if (debug) cout << " Testing started " << endl;
                            classifier->Test();
                            if (debug) cout << " Testing done " << endl;
                        }

                        evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                         R, F1);
                        stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                 "," + std::to_string(classifier->testTime) + "," +
                                 std::to_string(classifier->clientTestTime) + "," +
                                 classifier->extTestBd.get_str() + "," + std::to_string(P) + "," +
                                 std::to_string(R) + "," + std::to_string(F1) << endl;

                    }
                    datasetReader->closeTraining();
                    datasetReader->closeTesting();
                    datasetReader->reinit();

                    if (i == (runs_number - 1) && remoteClient) {
                        classifier->sendMsg("stop", sockfd, cliaddr);
                        close(sockfd);

                    }


                    try {

                        delete classifier;

                    } catch (exception e) {
                        cout << "Trying to delete classifier " << endl;
                        cout << e.what() << endl;
                    }


                }

                datasetReader->closeTraining();
                datasetReader->closeTesting();
                delete datasetReader;


            }else {


                if (MLAlgorithm=="lr")
                {


                    DatasetReader *datasetReader = new DatasetReader(path, Incremental, chunk_prefix, class_number,
                                                                     dimension);
                    for (int i = 0; i < runs_number; i++) {
                        if (debug) cout << "Run : " << i << " Over " << runs_number << endl;
                        stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;
                        vector<float> w;
                        for (int k=0; k < datasetReader->dimension-1; k ++ )
                            w.push_back(0);
                        LR *classifier = new LR (w,alpha, epochs, batchSize ,LRTH, datasetReader, OutputPath + classOutput,
                                                                          cliaddr, sockfd, debug, mainPath, sgdWorkers);

                        if (Incremental) {
                            stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1" << endl;
                            for (int ch = 0; ch < chunk_number; ch++) {

                                if (debug)
                                    cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;
                                classifier->train();
                                datasetReader->nextChunk();
                                //cout << "Ended training on chunk n°: " << ch+1 << endl;
                                if (remoteClient) {

                                    classifier->sendMsg("start", sockfd, cliaddr);
                                    string msg = classifier->recvMsg(sockfd, cliaddr);

                                    if (msg == "ok") {
                                        if (debug)
                                            cout << "Started remote testing after update n°: " << ch + 1 << " from: "
                                                 << chunk_number << endl;
                                        classifier->RemoteTest();
                                        if (debug) cout << " Remote test " << ch + 1 << "done" << endl;
                                    }

                                } else {
                                    if (debug)
                                        cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                             << endl;
                                    classifier->Test();
                                    if (debug) cout << "Test " << ch + 1 << " done" << endl;
                                }
                                //cout << "Client Test Time " << c << endl;
                                evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                                 R, F1);

                                stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                         "," + std::to_string(classifier->testTime) + "," +
                                         std::to_string(classifier->clientTestTime) + "," +
                                         classifier->extTestBd.get_str() + "," + std::to_string(P).c_str() + "," +
                                         std::to_string(R).c_str() + "," + std::to_string(F1).c_str() << endl;

                                datasetReader->reloadTest();
                            }
                        } else {
                            cout << datasetReader->train_size << endl;
                            cout << datasetReader->test_size << endl;
                            if (debug) cout << "Training started" << endl;
                            classifier->train();
                            if (debug) cout << "Training done" << endl;

                            if (remoteClient) {
                                classifier->sendMsg("start", sockfd, cliaddr);
                                if (debug) cout << " Remote testing started " << endl;
                                classifier->RemoteTest();
                                if (debug) cout << " Remote testing done " << endl;

                            } else {
                                if (debug) cout << " Testing started " << endl;
                                classifier->Test();
                                if (debug) cout << " Testing done " << endl;
                            }


                            evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                             R, F1);
                            cout << "F1 " << F1 << endl;

                            stats << std::to_string(classifier->trainTime).c_str() << "," << classifier->extTrainBd.get_str() <<
                                     "," << std::to_string(classifier->testTime).c_str() << "," <<
                                     std::to_string(classifier->clientTestTime).c_str() << "," <<
                                     classifier->extTestBd.get_str() << "," << std::to_string(P).c_str() << "," <<
                                     std::to_string(R).c_str() << "," << std::to_string(F1).c_str() << endl;

                        }
                        datasetReader->closeTraining();
                        datasetReader->closeTesting();
                        datasetReader->reinit();


                        if (i == (runs_number - 1) && remoteClient) {
                            classifier->sendMsg("stop", sockfd, cliaddr);
                            close(sockfd);

                        }

                        try {

                            delete classifier;

                        } catch (exception e) {
                            cout << "Trying to delete classifier " << endl;
                            cout << e.what() << endl;
                        }

                    }


                    datasetReader->closeTraining();
                    datasetReader->closeTesting();
                    delete datasetReader;



                }
                else{


                    if (MLAlgorithm=="nn")
                    {


                        DatasetReader *datasetReader = new DatasetReader(path, Incremental, chunk_prefix, class_number,
                                                                         dimension, true);
                        for (int i = 0; i < runs_number; i++) {
                            if (debug) cout << "Run : " << i << " Over " << runs_number << endl;
                            stats << "--------Run@" << i + 1 << " over : " << runs_number << "---------" << endl;


                            NN *classifier = new NN (alpha, epochs, batchSize ,LRTH, datasetReader, OutputPath + classOutput,
                                                    debug, mainPath);


                            if (Incremental) {
                                stats << "TrainTime,TrainBd,TestServerTime,TestClientTime,TestBd,P,R,F1" << endl;
                                for (int ch = 0; ch < chunk_number; ch++) {

                                    if (debug)
                                        cout << "Started training on chunk n°: " << ch + 1 << " from: " << chunk_number << endl;
                                    classifier->train();
                                    datasetReader->nextChunk();

                                        if (debug)
                                            cout << "Started testing after update n°: " << ch + 1 << " from: " << chunk_number
                                                 << endl;
                                        classifier->Test();
                                        if (debug) cout << "Test " << ch + 1 << " done" << endl;

                                    //cout << "Client Test Time " << c << endl;
                                    evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                                     R, F1);

                                    stats << std::to_string(classifier->trainTime) + "," + classifier->extTrainBd.get_str() +
                                             "," + std::to_string(classifier->testTime) + "," +
                                             std::to_string(classifier->clientTestTime) + "," +
                                             classifier->extTestBd.get_str() + "," + std::to_string(P).c_str() + "," +
                                             std::to_string(R).c_str() + "," + std::to_string(F1).c_str() << endl;

                                    datasetReader->reloadTest();
                                }
                            } else {
                                cout << datasetReader->train_size << endl;
                                cout << datasetReader->test_size << endl;
                                if (debug) cout << "Training started" << endl;
                                classifier->train();
                                if (debug) cout << "Training done" << endl;


                                    if (debug) cout << " Testing started " << endl;
                                    classifier->Test();
                                    if (debug) cout << " Testing done " << endl;



                                evaluateAccuracy(clearPath, mainPath, OutputPath, classOutput, datasetReader->test_size, P,
                                                 R, F1);
                                cout << "F1 " << F1 << endl;

                                stats << std::to_string(classifier->trainTime).c_str() << "," << classifier->extTrainBd.get_str() <<
                                      "," << std::to_string(classifier->testTime).c_str() << "," <<
                                      std::to_string(classifier->clientTestTime).c_str() << "," <<
                                      classifier->extTestBd.get_str() << "," << std::to_string(P).c_str() << "," <<
                                      std::to_string(R).c_str() << "," << std::to_string(F1).c_str() << endl;

                            }
                            datasetReader->closeTraining();
                            datasetReader->closeTesting();
                            datasetReader->reinit();




                            try {

                                delete classifier;

                            } catch (exception e) {
                                cout << "Trying to delete classifier " << endl;
                                cout << e.what() << endl;
                            }

                        }


                        datasetReader->closeTraining();
                        datasetReader->closeTesting();
                        delete datasetReader;



                    }


                }

            }



        }


    stats.close();
}
else {


        for (int i = 0; i < runs_number ; i++) {

            TESTBuildingBlocks testBuildingBlocks;
            testBuildingBlocks.run(deserialize, keySize, blindVal, DTPKC_Err, pathSer, false, logfile+"NON");
            testBuildingBlocks.run(deserialize, keySize, blindVal, DTPKC_Err, pathSer, optim, logfile+"opt");

            //TESTBuildingBlocks testBuildingBlocks;
            //testBuildingBlocks.TrunGlobal(deserialize, keySize, blindVal, DTPKC_Err, pathSer);


            //TESTpacking tesTpacking;
            //tesTpacking.run(deserialize, keySize, blindVal, DTPKC_Err, pathSer);


            //TESTElementaryOps testOps;
            //testOps.run(deserialize, keySize, blindVal, DTPKC_Err, pathSer, false);
            //testOps.run(deserialize, keySize, blindVal, DTPKC_Err, pathSer, true );

        }

}


    return  0;
}
