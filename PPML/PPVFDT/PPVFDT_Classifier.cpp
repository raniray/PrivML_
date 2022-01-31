#define _BSD_SOURCE
#include "PPVFDT_Classifier.h"
#include "../../UTIL/mpz_class.hh"
#include <time.h>
#include <string>
#include <set>
#include <chrono>
#include "omp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../CRYPTO/EvalCmp100.h"



#define MAX_DATE 120



PPDatasetReader * PPVFDT_Classifier::datasetReader;
float PPVFDT_Classifier::delta;
mpz_class PPVFDT_Classifier::tau;
int PPVFDT_Classifier::grace, PPVFDT_Classifier::max_depth;
int PPVFDT_Classifier::thresholdNumber;
string PPVFDT_Classifier::logfile;
DTPKC PPVFDT_Classifier::dtpkc;
Cipher PPVFDT_Classifier::zero, PPVFDT_Classifier::one;
mpz_class  PPVFDT_Classifier::extTrainBd;
mpz_class PPVFDT_Classifier::extTestBd;
double PPVFDT_Classifier::trainErr;
double PPVFDT_Classifier::trainTime;
double PPVFDT_Classifier::testTime;
int PPVFDT_Classifier::treeDepth;
double PPVFDT_Classifier::clientTestTime;
int PPVFDT_Classifier::addCtr, PPVFDT_Classifier::cmpCtr, PPVFDT_Classifier::divCtr, PPVFDT_Classifier::addPowCtr, PPVFDT_Classifier::stdCtr, PPVFDT_Classifier::evalThCtr, PPVFDT_Classifier::subCtr, PPVFDT_Classifier::multCtr, PPVFDT_Classifier::entropyCtr, PPVFDT_Classifier::hboundCtr ;

double PPVFDT_Classifier::addTime, PPVFDT_Classifier::cmpTime, PPVFDT_Classifier::divTime, PPVFDT_Classifier::addPowTime, PPVFDT_Classifier::stdTime, PPVFDT_Classifier::evalThTime, PPVFDT_Classifier::subTime, PPVFDT_Classifier::multTime, PPVFDT_Classifier::entropyTime, PPVFDT_Classifier::hboundTime ;

double PPVFDT_Classifier::trainTimeSU;
double PPVFDT_Classifier::trainTimeMU;
double PPVFDT_Classifier::trainBdw;

double PPVFDT_Classifier::testTimeSU;
double PPVFDT_Classifier::testTimeMU;
double PPVFDT_Classifier::testBdw;

PPRecord * PPVFDT_Classifier::recvPPRecord(int sockfd, sockaddr_in cliaddr, int dim, DTPKC dtpkc, int  id)
{

    if (debug) cout << "recieving record " << id << endl;
    socklen_t addrlen = sizeof(cliaddr);

    int size = dim*2000+dim+2000;
    string recievedRec="";
    int n=0;
    while (size>0)
    {
        if ( size < 65000 )
        {
            char buf[size];
            memset(buf, 0, sizeof(buf));
            n = recvfrom(sockfd, buf, size, 0, (struct sockaddr *)&cliaddr, &addrlen);
            //buf[n] = '\0';
            recievedRec+=buf;

        } else {

            char buf[65000];
            n = recvfrom(sockfd, buf, 65000, 0, (struct sockaddr *)&cliaddr, &addrlen);
            //buf[n] = '\0';
            recievedRec+=buf;

        }
        if (n!=-1)
        size -= n;

    }


    std::vector<Cipher> values;
    PPRecord * r;


    char * dup = strdup(recievedRec.c_str());
    char * v=strtok (dup,",");
    mpz_class key;
    key.set_str(v,10);

    while (v != NULL) {
        v = strtok(NULL, ",");
        if(v!= NULL) {
            Cipher c;
            c.Pub = key;
            c.T1.set_str(v, 10);

            if (v != NULL) {
                v = strtok(NULL, ",");
                c.T2.set_str(v, 10);
            }

            values.push_back(c);
        } else
            break;

    }
    try {
        free(dup);
        free(v);
    }catch (exception e )
    {
        cout << "trying to remove dup value  in PPrecord receive " << endl;
        cout << e.what() << endl;
    }
    r = new PPRecord (id,values,dtpkc);
    return r;

}

void PPVFDT_Classifier::sendResponse(int sockfd, sockaddr_in cliaddr, Cipher res)
{
    char buffer[4001];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%s,%s", res.T1.get_str().c_str(), res.T2.get_str().c_str());
    sendto(sockfd, buffer, 4001, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent response " << endl;
}
//
int
PPVFDT_Classifier ::sendMsg(string msg, int sockfd, sockaddr_in cliaddr)
{

    int n = sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent msg " << msg << endl;
    return  n;
}

string
PPVFDT_Classifier :: recvMsg(int sockfd,  sockaddr_in cliaddr)
{
    string res ="";
    socklen_t addrlen = sizeof(cliaddr);
    char buf[200];
    int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);
    if (n!=-1) {
        buf[n] = '\0';
        res = buf;
        if (debug) cout << "received msg " << res << endl;
    }
    return res;
}


PPVFDT_Classifier :: PPVFDT_Classifier(PPNode* Tree, PPDatasetReader * datasetReader, float delta, mpz_class tau, int grace, int max_depth, int thresholdNumber, DTPKC dtpkc, string logfile, sockaddr_in cliaddr, int sockfd, bool debug, int maxNodes){

    this->Tree=Tree;
    this->datasetReader= datasetReader;
    this->delta=delta;
    this->tau=tau;
    this->grace=grace;
    this->max_depth=max_depth;
    this->thresholdNumber = thresholdNumber;
    this->dtpkc=dtpkc;
    this->zero =dtpkc.enc(0,dtpkc.pkw);
    this->one =dtpkc.enc(1,dtpkc.pkw);
    this->logfile = logfile;
    this->cliaddr = cliaddr;
    this->sockfd = sockfd;
    this->numberSplits=0;
    this->numberCriticalZone=0;
    this->debug=debug;
    this->recordCounter = 0;
    this->trainTime=0;
    this->extTrainBd=0;
    this->testTime=0;
    this->clientTestTime=0;
    this->extTestBd=0;
    this->treeDepth=0;
    this->maxNodes=maxNodes;
    this->numberOfNodes=0;

    this->addTime = 0.0, this->cmpTime =0.0 , this->divTime = 0.0 , this->addPowTime = 0.0;
    this->stdTime = 0.0, this->evalThTime =0.0 , this->subTime = 0.0 , this->multTime = 0.0;
    this->entropyTime = 0.0 , this->hboundTime = 0.0;


    this->addCtr = 0, this->cmpCtr =0 , this->divCtr = 0 , this->addPowCtr = 0;
    this->stdCtr = 0, this->evalThCtr =0 , this->subCtr = 0 , this->multCtr = 0;
    this->entropyCtr = 0 , this->hboundCtr = 0;

    this->trainTimeSU=0.0;
    this->trainTimeMU=0.0;
    this->trainBdw=0.0;

    this->testTimeSU=0.0;
    this->testTimeMU=0.0;
    this->testBdw=0.0;

}

void  PPVFDT_Classifier ::train ()
{
    numberCriticalZone=0;
    numberSplits=0;
    int counter =0, rcdCtr=0; int tree_depth=Tree->treeDepth();
    PPNode * leaf;
    Cipher infoGain, firstBestGain = zero, secondBestGain = zero;
    double TRainErr=0.0;
    int size=datasetReader->train_size;
    Cipher threshold= zero;
    Cipher splittingThreshold = zero;
    int attribute=0;
    Cipher diffGain;
    Cipher hBound;
    PPRecord * record;

    CounterZero=0;
    bool in = false;
    extTrainBd = 0;

    dtpkc.timeSU = 0.0;
    dtpkc.timeMU = 0.0;
    dtpkc.bdwMU = 0.0;
    dtpkc.bdwSU = 0.0;
    dtpkc.cmpCtr = 0;

    double t1, t2;

    auto begin = chrono::high_resolution_clock::now();

    while (counter < size)
    {
        try
        {
            record = datasetReader->getTrainRecord();
            counter ++ ;
        }
        catch(std::exception const& e) {
            std::cout << "Exception: " << e.what() << "\n";
        }

        extTrainBd += record->getSize();
        recordCounter++;
        leaf = Tree->Classify(record,dtpkc);
        firstBestGain = zero;
        secondBestGain = zero;
        int splitAttribute = -1 ;
        splittingThreshold=  zero;
        attribute=0;

        if(leaf->nodeStats->mostFrequentClass != record->values[datasetReader->dimension-1])
            TRainErr += 1;

        if (debug) cout << "Train step : " << counter << " over " << size << endl;

        leaf->nodeStats->UpdateNodeStats(record);
        leaf->nodeStats->computeMostFrequentClass();


        if (recordCounter >= grace && leaf->depth()<max_depth && Tree->numberOfNodes() < maxNodes ) {

            if (leaf->nodeStats->diffClass) {

                numberCriticalZone++;

                map<int, map<Cipher, Cipher>> infoGains;



                double maxTimeEntr =0;
                double maxTimeSts = 0;
                #pragma omp parallel for private(attribute, infoGain, threshold) shared (infoGains, leaf) schedule(static)
                for (attribute = 0; attribute < datasetReader->dimension - 1; attribute++) {
                    if ((leaf->father == NULL) || (attribute != leaf->father->nodeTest->attribute)) {


                        
                        PPNode *l = new PPNode(leaf);
                        vector<Cipher> thresholdVect;
                        auto begin_para0 = chrono::high_resolution_clock::now();
                        thresholdVect = SplittingThresholds(l, attribute);
                        auto end_para0 = chrono::high_resolution_clock::now();
                        std::chrono::duration<double, std::milli> duration = end_para0  - begin_para0;
                        double sts_l = duration.count();
                        map<Cipher, Cipher> innerMapInfoGain;

                        auto begin_para = chrono::high_resolution_clock::now();
                        for (int i = 0; i < this->thresholdNumber; i++) {

                            threshold = thresholdVect[i];
                            infoGain = informationGain(l, attribute, threshold);
                            //if (debug) cout << " infogain " << infoGain << ", " << attribute << "," << threshold << endl;
                            innerMapInfoGain.insert(std::make_pair(threshold, infoGain));

                        }

                        auto end_para = chrono::high_resolution_clock::now();
                        duration = end_para  - begin_para;
                        /**
                         * this->multTime  = duration.count()* 0.3;
                         * this->divTime  = 100 * duration.count() * 0.3 ;
                         * this->entropyTime_l  = 100 * duration.count() * 0.4;
                         */
                        double entropyTime_l = duration.count();


                        /**
                         * To do :
                         * double sts_duration
                         * if (sts_durationd  < stsl) sts_duration = stsl;
                         * this->evalThCtr++;
                         *
                         */
                        #pragma omp critical
                        {
                            infoGains.insert(std::make_pair(attribute, innerMapInfoGain));
                            if (sts_l>maxTimeSts) maxTimeSts = sts_l;
                            if (entropyTime_l>maxTimeEntr) maxTimeEntr = entropyTime_l;


                            delete l;
                        }
                    }
                }

                this->multTime += 0.3 * maxTimeEntr;
                this->divTime += 0.3 * maxTimeEntr;;
                this->entropyTime += 0.4 * maxTimeEntr;
                this->evalThTime +=maxTimeSts;
                this->multCtr += 3*this->thresholdNumber;
                this->divCtr += 3*this->thresholdNumber;
                this->entropyCtr += 3*this->thresholdNumber;
                this->evalThCtr++;


                auto be_l = chrono::high_resolution_clock::now();
                map<int, map<Cipher, Cipher>>::iterator it1;
                for (it1 = infoGains.begin(); it1 != infoGains.end(); ++it1) {
                    map<Cipher, Cipher>::iterator it2;
                    for (it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
                        //cout << "( " << it1->first << ", " << it2->first << ", " << it2->second << " )" << endl;
                        if (it2->second > firstBestGain) {
                            firstBestGain = it2->second;
                            splitAttribute = it1->first;
                            splittingThreshold = it2->first;

                        } else {
                            if (it2->second > secondBestGain && firstBestGain != secondBestGain) {
                                secondBestGain = it2->second;

                            }
                        }
                    }
                }

                auto en_l = chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = en_l  - be_l;
                PPVFDT_Classifier::cmpTime+= duration.count();
                PPVFDT_Classifier::cmpCtr += infoGains.size();;

                infoGains.clear();

                //cout << " firstBestGain " <<firstBestGain << endl;
                //cout << " secondBestGain " << secondBestGain << endl;

                if (splitAttribute != -1) {

                    int tmp1 = 0, tmp2 = 0;
                    //#pragma omp sections
                    {
                        //#pragma omp section

                        {

                            EvalSub evalSub(firstBestGain, secondBestGain, dtpkc);
                            diffGain = evalSub.EvalSub_U1();



                        }
                        //#pragma omp section
                        {

                            mpz_class i = DTPKC::precision* DTPKC::precision* 100 ;
                            mpz_class k = log(1 / delta);
                            i = i * mpz_class_powm(mpz_class_log2_10(datasetReader->classNumber), 2, dtpkc.n2) * k;
                            Cipher tmp;

                            tmp.T1 = leaf->nodeStats->totSampleNum.T1 * leaf->nodeStats->totSampleNum.T1;
                            tmp.T2 = leaf->nodeStats->totSampleNum.T2 * leaf->nodeStats->totSampleNum.T2;
                            tmp.Pub = leaf->nodeStats->totSampleNum.Pub;


                            be_l = chrono::high_resolution_clock::now();
                            hBound = HBound(dtpkc.enc(i, dtpkc.pkw), tmp);
                            en_l = chrono::high_resolution_clock::now();
                            duration = en_l  - be_l;
                            PPVFDT_Classifier::hboundTime+= duration.count();
                            this->hboundCtr++;

                        }
                    }

                    if (debug) cout << " HBound " << hBound << " diffGain " << diffGain << endl;


                    if (diffGain > hBound || diffGain < dtpkc.enc(tau * 1000000, dtpkc.pkw)) {


                        //cout << "**Att** " << splitAttribute << " **TH** " << splittingThreshold << endl;

                        be_l = chrono::high_resolution_clock::now();
                        std::vector<int> res = splitNode(leaf, splitAttribute, splittingThreshold);
                        auto en_l = chrono::high_resolution_clock::now();
                        std::chrono::duration<double, std::milli> duration = en_l  - be_l ;
                        PPVFDT_Classifier::cmpTime+= duration.count();
                        this->cmpCtr += res[0] + 1 ;
                        leaf->isLeaf = false;
                        leaf->children->leftChild->resetNodeStats();
                        leaf->children->rightChild->resetNodeStats();
                        recordCounter = 0;
                        numberSplits++;

                        if (debug) cout << "I have done a splitting " << endl;

                        //leaf->dump();
                        //leaf->children->leftChild->dump();
                        //leaf->children->rightChild->dump();


                    }
                }


            }


            tree_depth = Tree->treeDepth();
            if (debug) cout << "Current tree depth is " << tree_depth << endl;

        }
    }

    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - begin;

    trainTime = duration.count();

    t1 = (this->cmpCtr + dtpkc.cmpCtr)* 6.39  + this->addCtr*0.00092 + this->addPowCtr* 0.44092+ this->divCtr*9.5 + this->stdCtr*1022.97 + this->evalThCtr*8.98 + this->subCtr*0.44092 + this->multCtr*12.22 + this->entropyCtr*12.13 + this->hboundCtr*2048.77;
    t2 = (this->cmpCtr + dtpkc.cmpCtr) * 6.56  + this->divCtr*9 + this->stdCtr*6 + this->evalThCtr*8.73 + this->multCtr* 9.03+ this->entropyCtr* 15.87+ this->hboundCtr*6;


    this->trainBdw = (this->cmpCtr + dtpkc.cmpCtr)*769 + this->divCtr*1632 + this->stdCtr*1184 + this->evalThCtr*1632 +  this->multCtr*1920 + this->entropyCtr*1376 + this->hboundCtr*1184;


    this->trainTimeMU = t1/(t1+t2);
    this->trainTimeSU = t2/(t1+t2);


    treeDepth = Tree->treeDepth() ;

    numberOfNodes = Tree->numberOfNodes();

    trainErr = TRainErr/size;

}


std::vector<int> PPVFDT_Classifier::splitNode(PPNode *node, int attribute, Cipher threshold ) {

    std::vector<int> res;

    res.push_back(0);
    res.push_back(0);


    node->addChildren();
    node->nodeTest = new PPNodeTest(attribute, threshold);

    map<int, map<Cipher, map<Cipher, Cipher >>>::iterator it = node->nodeStats->dataAVC.find(attribute);

    if (it != node->nodeStats->dataAVC.end()) {

        PPNodeStats::Stats &innerMap = it->second;
        PPNodeStats::Stats::iterator innerit;

            for (innerit = innerMap.begin(); innerit != innerMap.end(); innerit++) {

                EvalCmp100 evalCmp(innerit->first , threshold, dtpkc);
                bool test = evalCmp.EvalCmp_U2_step2(evalCmp.EvalCmp_U1_step1());
                res[0]++;

                if (test)
                {
                    map<Cipher, Cipher> &innerMap2 = innerit->second;
                    map<Cipher, Cipher>::iterator it3;
                    for (it3 = innerMap2.begin(); it3 != innerMap2.end(); it3++) {

                        Cipher occ = it3->second;
                        EvalAdd a(occ, node->children->rightChild->nodeStats->totSampleNum);
                        node->children->rightChild->nodeStats->totSampleNum = a.EvalAdd_U1();
                        res[1]++;


                        map<Cipher, Cipher>::iterator it4 = node->children->rightChild->nodeStats->classOccurrence.find(
                                it3->first);

                        if (it4 != node->children->rightChild->nodeStats->classOccurrence.end()) {
                            EvalAdd b(it4->second, occ);
                            it4->second = b.EvalAdd_U1();
                            res[1]++;

                        } else
                            node->children->rightChild->nodeStats->classOccurrence.insert(
                                    std::make_pair(it3->first, occ));

                        if (node->children->rightChild->nodeStats->mostFrequentClassOccurrence != (it3->first))
                            node->children->rightChild->nodeStats->diffClass = true;


                        if (node->children->rightChild->nodeStats->mostFrequentClassOccurrence <
                            node->children->rightChild->nodeStats->classOccurrence.find(it3->first)->second) {
                            node->children->rightChild->nodeStats->mostFrequentClassOccurrence = node->children->rightChild->nodeStats->classOccurrence.find(
                                    it3->first)->second;
                            node->children->rightChild->nodeStats->mostFrequentClass = it3->first;
                        }


                    }

                }
                else {

                    map<Cipher, Cipher> &innerMap2 = innerit->second;
                    map<Cipher, Cipher>::iterator it3;

                    for (it3 = innerMap2.begin(); it3 != innerMap2.end(); it3++) {


                        Cipher occ = it3->second;
                        EvalAdd a(node->children->leftChild->nodeStats->totSampleNum, occ);
                        node->children->leftChild->nodeStats->totSampleNum = a.EvalAdd_U1();
                        res[1]++;


                        map<Cipher, Cipher>::iterator it4 = node->children->leftChild->nodeStats->classOccurrence.find(
                                it3->first);
                        if (it4 != node->children->leftChild->nodeStats->classOccurrence.end()) {
                            EvalAdd b(it4->second, occ);
                            it4->second = b.EvalAdd_U1();
                            res[1]++;

                        } else
                            node->children->leftChild->nodeStats->classOccurrence.insert(
                                    std::make_pair(it3->first, occ));

                        if (node->children->leftChild->nodeStats->mostFrequentClassOccurrence != (it3->first))
                            node->children->leftChild->nodeStats->diffClass = true;


                        if (node->children->leftChild->nodeStats->mostFrequentClassOccurrence <
                            node->children->leftChild->nodeStats->classOccurrence.find(it3->first)->second) {
                            node->children->leftChild->nodeStats->mostFrequentClassOccurrence = node->children->leftChild->nodeStats->classOccurrence.find(
                                    it3->first)->second;
                            node->children->leftChild->nodeStats->mostFrequentClass = it3->first;
                        }


                    }


                }
            }
        }


    return res;


    }



vector<Cipher> PPVFDT_Classifier::SplittingThresholds(PPNode *node, int attribute)
{

    EvalDiv evalDiv(node->nodeStats->sumOfValues.find(attribute)->second,node->nodeStats->totSampleNum,dtpkc);
    vector<Cipher> res = evalDiv.EvalDiv_U1_step1();
    Cipher mean= evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res[0], res[1],res[2],res[3]));
    this->divCtr++;


    EvalStandardDiv evalStadardDiv(node->nodeStats->sumOfSquares.find(attribute)->second,node->nodeStats->totSampleNum, mean , dtpkc);
    Cipher standardDiv = evalStadardDiv.EvalStandardDiv_U1_step3(evalStadardDiv.EvalStandardDiv_U2_step2(evalStadardDiv.EvalStandardDiv_U1_step1()));
    this->stdCtr++;

    vector <Cipher> result (this->thresholdNumber);
    double x;
    Cipher tmp;
    int k=0;
    for (int i=1; i< this->thresholdNumber+1; i++)
    {
        x = (1.0*i)/(1.0*this->thresholdNumber + 1);
        EvalThreashold evalThreashold (mean, standardDiv, x, dtpkc);
        tmp = evalThreashold.eval();


        if (tmp.T1 != -1 && tmp.T2 != -1)
        {
            result[k]= tmp;
            k++;
        }
        //cout << "th ("<< attribute << ")= " << result[i-1] << "mean " << mean << " std " << standardDiv<< endl;

    }


    return  result;

}

Cipher PPVFDT_Classifier::informationGain(PPNode *node, int attribute, Cipher thresholds)
{
    std::vector<int> stats;
    Cipher result, leftEntropy, rightEntropy, leftWeight, rightWeight, leftPart, rightPart;
    Cipher totSampleNum;
    totSampleNum = node->nodeStats->totSampleNum;

    if (attribute== -1)
        return result;

    result = nodeEntropy(node);

    if (debug) cout << "nodeEntropy " << result << endl;

    EvalMult e(result, dtpkc.enc(100, dtpkc.pkw), dtpkc);

    result = e.EvalMult_U1_step3(e.EvalMult_U2_step2(e.EvalMult_U1_step1()));


    stats  = splitNode(node, attribute, thresholds);


    //cout << "attribute " << attribute << "thresholds" << thresholds << endl;

            leftEntropy = nodeEntropy(node->children->leftChild);
            EvalDiv100 evalDiv100(node->children->leftChild->nodeStats->totSampleNum, totSampleNum, dtpkc);
            vector<Cipher> res1 = evalDiv100.EvalDiv_U1_step1();
            leftWeight = evalDiv100.EvalDiv_U1_step3(evalDiv100.EvalDiv_U2_step2(res1[0], res1[1], res1[2], res1[3]));


            EvalMult e1(leftEntropy,leftWeight,dtpkc);
            leftPart = e1.EvalMult_U1_step3(e1.EvalMult_U2_step2(e1.EvalMult_U1_step1()));


            rightEntropy = nodeEntropy(node->children->rightChild);
            EvalDiv100 evalDiv(node->children->rightChild->nodeStats->totSampleNum, totSampleNum, dtpkc);
            vector<Cipher> res2 = evalDiv.EvalDiv_U1_step1();
            rightWeight = evalDiv.EvalDiv_U1_step3(evalDiv.EvalDiv_U2_step2(res2[0], res2[1], res2[2], res2[3]));

            EvalMult e2(rightEntropy,rightWeight,dtpkc);
            rightPart = e2.EvalMult_U1_step3(e2.EvalMult_U2_step2(e2.EvalMult_U1_step1()));



             if (debug) cout << "left part " << leftPart << endl;


             if (debug) cout << "right  part " << rightPart << endl;


    node->resetCildren();

    EvalAdd add (leftPart, rightPart);
    Cipher children=add.EvalAdd_U1();



    EvalSub res(result, children, dtpkc);
    result = res.EvalSub_U1();



    return result;
}

Cipher PPVFDT_Classifier::nodeEntropy (PPNode *node)
{
    Cipher res;
    vector<Cipher> v;

    for(map<Cipher,Cipher>::iterator it = node->nodeStats->classOccurrence.begin(); it != node->nodeStats->classOccurrence.end(); ++it) {
        v.push_back(it->second);
    }
    if (node->nodeStats->totSampleNum != zero && v.size()>1)
    {
        EvalEntropy evalEntropy(node->nodeStats->totSampleNum, v, dtpkc);
        //node->dump();
        res = evalEntropy.EvalEntropy_U1_step3(evalEntropy.EvalEntropy_U2_step2(evalEntropy.EvalEntropy_U1_step1()));

    }
    else
    {
        res =zero;
        CounterZero++;
    }

    return res;
}


Cipher PPVFDT_Classifier::HBound(Cipher A, Cipher  B)
{
    EvalHBound evalHBound (A, B, dtpkc);
    Cipher res = evalHBound.EvalHbound_U1_step3(evalHBound.EvalHBound_U2_step2(evalHBound.EvalHBound_U1_step1()));

    return res;
}

void PPVFDT_Classifier::Test( ){

    int counter =0;
    int size=datasetReader->test_size;
    PPRecord *  record;
    PPNode * leaf;
    std::ofstream classOutput;
    std::ofstream inter;
    inter.open(logfile+".enc");
    classOutput.open (logfile);

    this->cmpCtr = 0;

    this->cmpTime = 0.0;

    dtpkc.timeSU = 0.0;
    dtpkc.timeMU = 0.0;
    dtpkc.bdwMU = 0.0;
    dtpkc.bdwSU = 0.0;
    dtpkc.cmpCtr = 0;
    extTestBd = 0;

    double t1,t2;

    auto begin = chrono::high_resolution_clock::now();
    while (counter < size) {

        try {
            record = datasetReader->getTestRecord();
            extTestBd += record->getSize();
        }
        catch (std::exception const &e) {
            std::cout << "Exception: " << e.what() << "\n";

        }

        counter++;



            if (debug)
            {
                record->print();

            }

        leaf = Tree->Classify(record,dtpkc);

         try {

             delete record;

         }catch (exception e )
         {
             cout << "Trying to delete a record " << endl;
             cout << e.what() << endl;
         }

        if(classOutput.is_open())
        {
            inter<<leaf->nodeStats->mostFrequentClass.T1.get_str()+","+leaf->nodeStats->mostFrequentClass.T2.get_str()<< endl;
            extTestBd+=mpz_size(leaf->nodeStats->mostFrequentClass.T1.get_mpz_t())+ mpz_size(leaf->nodeStats->mostFrequentClass.T2.get_mpz_t())* sizeof(mp_limb_t);

        }


    }

    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end  - begin ;
    this->testTime = duration.count();
    inter.close();


    t1 = 6.39 * this->cmpCtr;

    t2 = 6.56 * this->cmpCtr;

    this->testTimeMU = t1/(t1+t2);

    this->testTimeSU = t2/(t1+t2);

    std::ifstream *In;
    string line, res="";
    char *v;


    In=new std::ifstream(logfile+".enc");

    while (getline(*In, line)) {

        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");

        while (v != NULL)
        {
            Cipher c;
            c.Pub = dtpkc.pkw;
            c.T1.set_str(v,10);
            v=strtok (NULL,",");

            if (v!=NULL)
            {
                c.T2.set_str(v,10);
                mpz_class m = dtpkc.Sdec(c);
                classOutput << m.get_str() << endl;

            }

            v=strtok (NULL,",");
        }

        try {
            delete[] dup;
        }catch (exception e )
        {
            cout << "Trying to delete dup " << endl;
            cout << e.what() << endl;
        }

    }

    In->close();
    classOutput.close();

}



void PPVFDT_Classifier::RemoteTest(){

    int counter =0;
    int size=this->datasetReader->test_size;
    PPNode * leaf;
    PPRecord * record;
    std::ofstream classOutput;
    std::ofstream inter;
    inter.open(logfile+".enc");
    classOutput.open (logfile);
    extTestBd = 0;

    auto begin = chrono::high_resolution_clock::now();


    while (counter < size) {

        try {

            record = recvPPRecord(this->sockfd,this->cliaddr,datasetReader->dimension-1, dtpkc, counter);
            if (debug) record->print();


            extTestBd += record->getSize();
        }
        catch (std::exception const &e) {
            std::cout << "Exception: " << e.what() << "\n";

        }

        counter++;
        leaf = Tree->Classify(record,dtpkc);
        if (debug)
        {
            record->print();
            //cout << "Test step : " << counter << " over " << size << endl;
        }
        sendResponse(this->sockfd, this->cliaddr, leaf->nodeStats->mostFrequentClass) ;

        try {

            delete record;

        }catch (exception e )
        {
            cout << "Trying to delete a record " << endl;
            cout << e.what() << endl;
        }

        if(inter.is_open())
        {
            inter<<leaf->nodeStats->mostFrequentClass.T1.get_str()+","+leaf->nodeStats->mostFrequentClass.T2.get_str()<< endl;
            extTestBd+=mpz_size(leaf->nodeStats->mostFrequentClass.T1.get_mpz_t())+ mpz_size(leaf->nodeStats->mostFrequentClass.T2.get_mpz_t());
        }


    }

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    this->testTime = duration.count();//-removeTime;

    char buf[200];
    socklen_t addrlen = sizeof(cliaddr);
    int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);
    buf[n] = '\0';
    if (n!=-1) this->clientTestTime = atof(buf);

    inter.close();

    std::ifstream *In;
    string line, res="";
    char *v;


    In=new std::ifstream(logfile+".enc");

    while (getline(*In, line)) {

        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");

        while (v != NULL)
        {
            Cipher c;
            c.Pub = dtpkc.pkw;
            c.T1.set_str(v,10);
            v=strtok (NULL,",");

            if (v!=NULL)
            {
                c.T2.set_str(v,10);
                mpz_class m = dtpkc.Sdec(c);
                classOutput << m.get_str() << endl;

            }

            v=strtok (NULL,",");
        }

        try {

            delete [] dup;
            free(v);

        }catch (exception e )
        {
            cout << "Trying to delete dup  " << endl;
            cout << e.what() << endl;
        }

    }

    In->close();
    classOutput.close();
}

