//
// Created by rania on 13/11/18.
//

#include "VFDT_Classifier.h"
#include "../../UTIL/mpz_class.hh"
#include <time.h>
#include <string>
#include "omp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>


#define MAX_DATE 120
using namespace std;




Record * VFDT_Classifier::recvRecord(int sockfd, sockaddr_in cliaddr, int dim, int  id)
{

    if (debug) cout << "recieving record " << id << endl;
    socklen_t addrlen = sizeof(cliaddr);

    int size = dim*200+dim+200;
    string recievedRec="";
    int n=0;
    while (size>0)
    {
        if ( size < 65000 )
        {
            char buf[size];
            n = recvfrom(sockfd, buf, size, 0, (struct sockaddr *)&cliaddr, &addrlen);
            //buf[n] = '\0';
            recievedRec+=buf;

        } else {

            char buf[65000];
            n = recvfrom(sockfd, buf, 65000, 0, (struct sockaddr *)&cliaddr, &addrlen);
            //buf[n] = '\0';
            recievedRec+=buf;

        }

        size -= n;

    }

    if (debug) cout << "I have recieved this message : " << recievedRec << endl;
    if (debug) cout << "The message size is : " << recievedRec.size() << endl;

    std::vector<int> values;
    Record * r;


    char * dup = strdup(recievedRec.c_str());
    char * v=strtok (dup,",");

    while (v != NULL) {

            values.push_back(atoi(v));
            v=strtok (NULL,",");
    }
    try {
        free(dup);
        free(v);
    }catch (exception e )
    {
        cout << "trying to remove dup value  in PPrecord receive " << endl;
        cout << e.what() << endl;
    }
    r = new Record (id,values);
    r->print();
    return r;

}

void VFDT_Classifier::sendResponse(int sockfd, sockaddr_in cliaddr, int res)
{
    char buffer[200];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%s", std::to_string(res).c_str());
    sendto(sockfd, buffer, 200, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent response " << endl;
}

int
VFDT_Classifier ::sendMsg(string msg, int sockfd, sockaddr_in cliaddr)
{
    int n = sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    return  n;
}

string
VFDT_Classifier :: recvMsg(int sockfd,  sockaddr_in cliaddr)
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
        if (debug) cout << "received nothing " << res << endl;

    return res;
}

VFDT_Classifier :: VFDT_Classifier(Node* Tree, DatasetReader * datasetReader, float delta, float tau, int grace, int max_depth, int thresholdNumber, string logfile, sockaddr_in cliaddr, int sockfd, bool debug, int maxNodes){

    this->Tree=Tree;
    this->datasetReader= datasetReader;
    this->delta=delta;
    this->tau=tau;
    this->grace=grace;
    this->max_depth=max_depth;
    this->thresholdNumber = thresholdNumber;
    this->logfile=logfile;
    this->debug = debug;
    this->sockfd=sockfd;
    this->cliaddr=cliaddr;
    this->trainTime=0;
    this->extTrainBd=0;
    this->testTime=0;
    this->clientTestTime=0;
    this->extTestBd=0;
    this->treeDepth=0;
    this->numberSplits=0;
    this->numberCriticalZone=0;
    this->recordCounter=0;
    this->maxNodes = maxNodes;
    this->numberOfNodes = 0;


}

double VFDT_Classifier ::train ()
{
    int counter =0;
    int size=datasetReader->train_size;
    double TRainErr=0.0;
    int tree_depth=Tree->treeDepth();
    Node * leaf;
    double infoGain, firstBestGain =0.0, secondBestGain = 0.0;
    int splitAttribute=-1;
    double  threshold=0.0,  splittingThreshold=0.0;
    map < int, map<double,double>> infoGains;
    int attribute=0;
    double diffGain;
    double hBound;
    Record * record;

    numberCriticalZone=0;
    numberSplits=0;
    extTrainBd = 0;


    auto begin = chrono::high_resolution_clock::now();
    while (counter < size)
    {


        try
        {
            record = datasetReader->getTrainRecord();
            counter++;
            extTrainBd += sizeof(int)*record->values.size();
        }
        catch(std::exception const& e)
        {
            //std::cout << "Exception: " << e.what() << "\n";
            return -1;
        }
        recordCounter++;
        leaf = Tree->Classify(record);

        infoGain = 0.0;
        firstBestGain =0.0;
        secondBestGain = 0.0;

        splitAttribute=-1;
        threshold=0.0;
        splittingThreshold=0.0;
        attribute=0;


        if(leaf->nodeStats->mostFrequentClass != record->values[datasetReader->dimension-1])
            TRainErr += 1;

        if (debug) cout << "Train step : " << counter << " over " << size << endl;


        leaf->nodeStats->UpdateNodeStats(record);
        leaf->nodeStats->computeMostFrequentClass();


        if (recordCounter >= grace && leaf->depth()<max_depth && Tree->numberOfNodes() < maxNodes )
        {

            if(leaf->nodeStats->diffClass){

                numberCriticalZone++;
                //#pragma omp parallel shared (infoGains, leaf ) private ( attribute, threshold, infoGain, i )
                {
                    Node * l = new Node(leaf);
                    //#pragma omp for
                    for (attribute = 0; attribute < datasetReader->dimension -1; attribute ++)
                    {
                        if ((leaf->father == NULL) || (attribute != leaf->father->nodeTest->attribute)) {

                            vector <double> thresholdVect;
                            thresholdVect = SplittingThresholds(l, attribute);

                            map<double, double> innerMapInfoGain;

                            for (int i=0; i < this->thresholdNumber; i++) {
                                threshold = thresholdVect[i]; // PB
                                if (threshold >= 0) {
                                    infoGain = informationGain(l, attribute, threshold);

                                    //if (debug) cout << " infogain " << infoGain << ", " << attribute << "," << threshold << endl;

                                    innerMapInfoGain.insert(std::make_pair(threshold,infoGain));

                                }
                            }
                            infoGains.insert(std::make_pair(attribute, innerMapInfoGain));

                        }


                    }
                    delete l;
                }

                // determine the best infoGain in infoGains and get the attribute and the threshold
                map<int,map<double,double>> ::iterator it1;
                for (it1 = infoGains.begin(); it1!=infoGains.end(); ++it1)
                {
                    map <double,double>::iterator it2;
                    for (it2 = it1->second.begin(); it2!=it1->second.end(); ++it2)
                    {
                        //cout << "( " << it1->first << ", " << it2->first << ", " << it2->second << " )" << endl;
                        if (it2->second>firstBestGain)
                        {
                            firstBestGain = it2->second;
                            splitAttribute = it1->first;
                            splittingThreshold = it2->first;

                        }else
                        {
                            if (it2->second>secondBestGain && firstBestGain != secondBestGain)
                            {
                                secondBestGain = it2->second;

                            }
                        }
                    }
                }
                infoGains.clear();

                //cout << " firstBestGain " <<firstBestGain << endl;
                //cout << " secondBestGain " << secondBestGain << endl;

                if (splitAttribute != -1 )
                {
                    diffGain = firstBestGain - secondBestGain;

                    hBound = HBound(log2(datasetReader->classNumber),log(1/delta), 2* leaf->nodeStats->totSampleNum);

                    //cout << " leaf->nodeStats->totSampleNum " << leaf->nodeStats->totSampleNum << endl;


                    if (debug) cout << " HBound " << hBound << " diffGain " << diffGain << endl;


                    if (diffGain > hBound || diffGain < tau)
                    {
                        //cout << "**Att** " << splitAttribute << " **TH** " << splittingThreshold << endl;
                        splitNode(leaf,  splitAttribute, splittingThreshold);
                        leaf->isLeaf=false;
                        leaf->children->leftChild->resetNodeStats();
                        leaf->children->rightChild->resetNodeStats(); // must create this function first
                        recordCounter = 0;
                        numberSplits++;


                        /*leaf->dump();
                        leaf->children->leftChild->dump();
                        leaf->children->rightChild->dump();*/
                    }
                }
            }


        }



        try
        {
            delete record;

        }
        catch(std::exception const& e)
        {
            //std::cout << "Exception: " << e.what() << "\n";
            return -1;
        }

        //tree_depth=Tree->treeDepth();

    }


    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - begin;

    trainTime = duration.count();

    treeDepth = Tree->treeDepth() ;

    numberOfNodes = Tree->numberOfNodes();

    trainErr = TRainErr/size;

    //modelSize = Tree->NodeSize();

    return TRainErr/size;

}


void VFDT_Classifier::splitNode(Node *node, int attribute, double threshold )
{

    node->addChildren();
    node->nodeTest = new NodeTest(attribute,threshold);

    map <int, map <int, map <int, int >> >:: iterator it = node->nodeStats->dataAVC.find(attribute);

    if( it != node->nodeStats->dataAVC.end() ) {

        NodeStats::st &innerMap = it->second;
        NodeStats::st::iterator innerit;

        for ( innerit = innerMap.begin(); innerit != innerMap.end(); innerit++ )
        {
            if (innerit->first < threshold ) // right child
            {

                //cout << innerit->first << endl;
                //cout << threshold << endl;

                map <int,int> &innerMap2 = innerit->second;
                map <int,int>::iterator it3;

                for ( it3 = innerMap2.begin(); it3 != innerMap2.end(); it3++ ) {

                    int occ = it3->second;
                    node->children->rightChild->nodeStats->totSampleNum+= occ;

                    map <int,int>::iterator it4 = node->children->rightChild->nodeStats->classOccurrence.find(it3->first);

                    if (it4 != node->children->rightChild->nodeStats->classOccurrence.end()) it4->second += occ;
                    else
                        node->children->rightChild->nodeStats->classOccurrence.insert(std::make_pair(it3->first,occ));

                    if ( node->children->rightChild->nodeStats->mostFrequentClassOccurrence != (it3->first))
                        node->children->rightChild->nodeStats->diffClass = true;


                    if ( node->children->rightChild->nodeStats->mostFrequentClassOccurrence <  node->children->rightChild->nodeStats->classOccurrence.find(it3->first)->second)
                    {
                        node->children->rightChild->nodeStats->mostFrequentClassOccurrence = node->children->rightChild->nodeStats->classOccurrence.find(it3->first)->second;
                        node->children->rightChild->nodeStats->mostFrequentClass= it3->first;
                    }

                    /* Stats innerMapL;
                     Stats:: iterator copyOfinnerit(innerit);
                     innerMapL.insert(std::make_pair(copyOfinnerit->first,copyOfinnerit->second));
                     node->children->rightChild->nodeStats->dataAVC.insert(std::make_pair(attribute,innerMapL));*/
                }

            } // Left child
            else
            {

                map <int,int> &innerMap2 = innerit->second;
                map <int,int>::iterator it3;

                for ( it3 = innerMap2.begin(); it3 != innerMap2.end(); it3++ ) {


                    int occ = it3->second;
                    node->children->leftChild->nodeStats->totSampleNum+= occ;

                    map <int,int>::iterator it4 = node->children->leftChild->nodeStats->classOccurrence.find(it3->first);
                    if (it4 != node->children->leftChild->nodeStats->classOccurrence.end())
                        it4->second += occ ;
                    else
                        node->children->leftChild->nodeStats->classOccurrence.insert(std::make_pair(it3->first,occ));

                    if ( node->children->leftChild->nodeStats->mostFrequentClassOccurrence != (it3->first))
                        node->children->leftChild->nodeStats->diffClass = true;


                    if ( node->children->leftChild->nodeStats->mostFrequentClassOccurrence <  node->children->leftChild->nodeStats->classOccurrence.find(it3->first)->second)
                    {
                        node->children->leftChild->nodeStats->mostFrequentClassOccurrence = node->children->leftChild->nodeStats->classOccurrence.find(it3->first)->second;
                        node->children->leftChild->nodeStats->mostFrequentClass= it3->first;
                    }

                    /* Stats innerMapL;
                     Stats:: iterator copyOfinnerit(innerit);
                     innerMapL.insert(std::make_pair(copyOfinnerit->first,copyOfinnerit->second));
                     node->children->leftChild->nodeStats->dataAVC.insert(std::make_pair(attribute,innerMapL));*/

                }




            }
        }

    }
}


vector<double> VFDT_Classifier::SplittingThresholds(Node *node, int attribute)
{
    mpz_class mean =  (node->nodeStats->sumOfValues.find(attribute)->second)/(node->nodeStats->totSampleNum);
    mpz_class standardDiv;
    standardDiv=mpz_class_sqrt(node->nodeStats->sumOfSquares.find(attribute)->second/(node->nodeStats->totSampleNum) - mpz_class_powm (mean,2,100000000000000000));
    vector <double> result (this->thresholdNumber);
    double x;
    for (int i=1; i< this->thresholdNumber+1; i++)
    {
        x = (1.0*i)/(1.0*this->thresholdNumber + 1);
        //cout << x << endl;
        //cout << mean.get_ui() << endl;
        //cout <<standardDiv.get_ui() << endl;
        //cout <<NormalCDFInverse(x) << endl;

        result[i-1]= (double) (mean.get_ui() + standardDiv.get_ui() * NormalCDFInverse(x));
    }
    //mpz_clear(mean.get_mpz_t());
    //mpz_clear(standardDiv.get_mpz_t());


    result.erase( unique( result.begin(), result.end() ), result.end() );

    return  result;

}

double VFDT_Classifier::informationGain(Node *node, int attribute, double thresholds)
{
    double result, leftEntropy, rightEntropy, leftWeight, rightWeight = 0.0;
    int totSampleNum;

    if (attribute== -1)
        return result;

    result = nodeEntropy(node);

    //cout << "nodeEntropy " << result << endl;

    totSampleNum = node->nodeStats->totSampleNum;

    //logfile << "Node entropy : " << result << " Total sample number : " << totSampleNum << endl;
    //logfile << " node->nodeStats->totSampleNum " << node->nodeStats->totSampleNum << " totSampleNum " << totSampleNum << endl;

    splitNode (node, attribute , thresholds);

    //cout << "attribute " << attribute << "thresholds" << thresholds << endl;

    leftEntropy = nodeEntropy(node->children->leftChild);
    rightEntropy = nodeEntropy(node->children->rightChild);

    leftWeight = (1.0 * node->children->leftChild->nodeStats->totSampleNum)/( 1.0 * totSampleNum);
    rightWeight = (1.0 * node->children->rightChild->nodeStats->totSampleNum)/(1.0 * totSampleNum);

    node->resetCildren();

    //logfile << "leftEntropy : " << leftEntropy << "rightEntropy : " << rightEntropy << " lW : " << leftWeight << "rW : " << rightWeight << endl;

    result -= (leftEntropy*leftWeight + rightEntropy*rightWeight);


    //cout << "left part " << leftEntropy*leftWeight << endl;


    //cout << "right  part " << rightEntropy*rightWeight<< endl;

    return result;

}

double VFDT_Classifier::nodeEntropy (Node *node)
{

    vector<int> v;
    for(map<int,int>::iterator it = node->nodeStats->classOccurrence.begin(); it != node->nodeStats->classOccurrence.end(); ++it) {
        v.push_back(it->second);
    }

    return Entropy(node->nodeStats->totSampleNum, v);
}

double VFDT_Classifier::Entropy (int S, vector<int> CkTable)
{

    double result= 0.0;
    int Ck;
    double Pk;
    for(int k=0; k<CkTable.size(); k++)
    {
        Ck = CkTable [k];
        if (Ck != 0 )
        {

            Pk=(1.0*Ck)/(1.0*S);
            result -= Pk * std::log2(Pk);

        }
    }
    return result;

}


double VFDT_Classifier::HBound(double A, double B, double C)
{
    return sqrt((pow(A,2)*B)/C);
}

void VFDT_Classifier::Test( ){

    int counter =0;
    int size=datasetReader->test_size;
    Node * leaf;
    Record * record;

    std::ofstream classOutput;
    classOutput.open (logfile);

    extTestBd = 0;
    auto begin = chrono::high_resolution_clock::now();
    while (counter < size) {

        try {
            record = datasetReader->getTestRecord();
            extTestBd += sizeof(int)*record->values.size();
        }
        catch (std::exception const &e) {
            //std::cout << "Exception: " << e.what() << "\n";

        }

        counter++;

        leaf = Tree->Classify(record);

        if(classOutput.is_open())
        {
            classOutput<<leaf->nodeStats->mostFrequentClass<< endl;
        }
        delete record;
    }

    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end  - begin ;
    this->testTime = duration.count(); //- removeTime;
    classOutput.close();
}

void VFDT_Classifier::RemoteTest(){

    int counter =0;
    int size=datasetReader->test_size;
    Node * leaf;
    Record * record;
    std::ofstream classOutput;

    std::ofstream inter;

    classOutput.open (logfile);
    extTestBd = 0;

    auto begin = chrono::high_resolution_clock::now();

    while (counter < size) {

        try {

            record = recvRecord(sockfd,cliaddr,datasetReader->dimension-1, counter);
            if (debug) record->print();

            extTestBd += sizeof(int)*record->values.size();
        }
        catch (std::exception const &e) {
            std::cout << "Exception: " << e.what() << "\n";
        }

        counter++;
        leaf = Tree->Classify(record);
        sendResponse(sockfd, cliaddr, leaf->nodeStats->mostFrequentClass);
        classOutput<<leaf->nodeStats->mostFrequentClass<< endl;
        extTestBd+=sizeof(leaf->nodeStats->mostFrequentClass);
        delete record;

    }

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    this->testTime = duration.count();


    char buf[200];
    socklen_t addrlen = sizeof(cliaddr);
    int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);
    buf[n] = '\0';
    this->clientTestTime = atof(buf);

    classOutput.close();
}
