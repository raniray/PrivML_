//
// Created by rania on 13/11/18.
//

#include <chrono>
#include "NB_Classifier.h"


#define MAX_DATE 120
using namespace std;




NB_Classifier::NB_Classifier(DatasetReader * datasetReader, Stats* stats,  int laplaceConstant, string logfile, sockaddr_in cliaddr, int sockfd, bool debug ) {

    this->datasetReader= datasetReader;
    this->currentModel=stats;
    this->newModel=stats;
    this->laplaceConstant=laplaceConstant;
    this->logfile=logfile;
    this->cliaddr=cliaddr;
    this->sockfd=sockfd;
    this->debug=debug;
    this->trainTime=0;
    this->extTrainBd=0;
    this->testTime=0;
    this->clientTestTime=0;
    this->extTestBd=0;

}


void NB_Classifier::Test() {

    int counter =0;
    int size=datasetReader->test_size;
    Record * record;
    int classAtt;
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
            std::cout << "Exception: " << e.what() << "\n";
        }

        counter++;
        classAtt = currentModel->Classify(record);
        if(classOutput.is_open())
        {
            classOutput<<classAtt<< endl;
        }

    }

    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    this->testTime = duration.count();
    classOutput.close();
    currentModel->print();

}


void NB_Classifier::train() {

    int counter =0;
    int size=datasetReader->train_size;
    Record * record;

    extTrainBd = 0;
    auto begin = chrono::high_resolution_clock::now();
    while (counter < size) {
        try {
            record = datasetReader->getTrainRecord();
            currentModel->UpdateStatistics(record);
            extTrainBd += sizeof(int)*record->values.size();

        }
        catch (std::exception const &e) {
            std::cout << "Exception: " << e.what() << "\n";

        }
        counter++;

    }

        currentModel->InitStats(laplaceConstant);
        Stats::initialised= true;
        auto end = chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end  - begin ;
        this->trainTime = duration.count();

}

Record * NB_Classifier::recvRecord(int sockfd, sockaddr_in cliaddr, int dim, int  id)
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

void NB_Classifier::sendResponse(int sockfd, sockaddr_in cliaddr, int res)
{
    char buffer[200];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%s", std::to_string(res).c_str());
    sendto(sockfd, buffer, 200, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent response " << endl;
}

int
NB_Classifier::sendMsg(string msg, int sockfd, sockaddr_in cliaddr)
{
    int n = sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    return  n;
}

string
NB_Classifier::recvMsg(int sockfd,  sockaddr_in cliaddr)
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


void NB_Classifier::RemoteTest(){

    int counter =0;
    int size=datasetReader->test_size;
    Record * record;
    int classAtt;
    std::ofstream classOutput;
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
        classAtt = currentModel->Classify(record);
        sendResponse(sockfd, cliaddr, classAtt);
        classOutput<< std::to_string(classAtt).c_str()<< endl;
        extTestBd+=sizeof(classAtt);
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
