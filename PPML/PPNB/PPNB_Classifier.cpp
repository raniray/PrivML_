#include <chrono>
#include "PPNB_Classifier.h"
#define MAX_DATE 120


using namespace std;

/**
 * Naive Bayes classifier over encrypted data
 *
 * */
double PPNB_Classifier:: trainTimeMU;
double PPNB_Classifier:: trainTimeSU;
double PPNB_Classifier:: testTimeMU;
double PPNB_Classifier:: testTimeSU;
double PPNB_Classifier:: trainBdw;
double PPNB_Classifier:: testBdw;
int    PPNB_Classifier::addCtr, PPNB_Classifier::cmpCtr, PPNB_Classifier::divCtr, PPNB_Classifier::logCtr;
double  PPNB_Classifier::addTime,  PPNB_Classifier::cmpTime,  PPNB_Classifier::divTime,  PPNB_Classifier::logTime;


PPRecord * PPNB_Classifier::recvPPRecord(int sockfd, sockaddr_in cliaddr, int dim, DTPKC dtpkc, int  id)
{

    // Recieving data records from clients
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

    if (debug) cout << "I have recieved this message : " << recievedRec << endl;
    if (debug) cout << "The message size is : " << recievedRec.size() << endl;

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

// Sending out the classification result
void PPNB_Classifier::sendResponse(int sockfd, sockaddr_in cliaddr, Cipher res)
{
    char buffer[4001];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%s,%s", res.T1.get_str().c_str(), res.T2.get_str().c_str());
    sendto(sockfd, buffer, 4001, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent response " << endl;
}

// Sending a message to the client

int
PPNB_Classifier::sendMsg(string msg, int sockfd, sockaddr_in cliaddr)
{

    int n = sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent msg " << msg << endl;
    return  n;
}

string
PPNB_Classifier:: recvMsg(int sockfd,  sockaddr_in cliaddr)
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

// Constructor of the classifier

PPNB_Classifier::PPNB_Classifier(PPDatasetReader *datasetReader, PPStats *stats, int laplaceConstant, string log, int sockfd, sockaddr_in cliaddr, bool debug) {

    this->datasetReader= datasetReader;
    this->currentModel=stats;
    this->logfile = log;
    this->debug=debug;
    this->dtpkc=datasetReader->dtpkc;
    this->laplaceConstant =  dtpkc.enc(laplaceConstant, dtpkc.pkw);
    this->sockfd=sockfd;
    this->cliaddr=cliaddr;
    this->trainTime=0;
    this->extTrainBd=0;
    this->testTime=0;
    this->clientTestTime=0;
    this->extTestBd=0;

    this->trainTimeMU=0.0;
    this->trainTimeSU=0.0;
    this->testTimeMU=0.0;
    this->testTimeSU=0.0;
    this->trainBdw=0.0;
    this->testBdw=0.0;

    this->addCtr = 0, this->cmpCtr =0 , this->divCtr = 0 , this->logCtr = 0;
    this->addTime = 0.0, this->cmpTime =0.0 , this->divTime = 0.0 , this->logTime = 0.0;
}


// Prediction function
void PPNB_Classifier::Test() {

    int counter =0;
    int size=datasetReader->test_size;
    PPRecord * record;
    Cipher classAtt;
    std::ofstream classOutput;
    std::ofstream inter;
    inter.open(logfile+".enc");
    classOutput.open (logfile);
    this->addCtr = 0, this->cmpCtr =0 , this->divCtr = 0 , this->logCtr = 0;
    this->addTime = 0.0, this->cmpTime =0.0 , this->divTime = 0.0 , this->logTime = 0.0;
    extTestBd = 0;
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
        classAtt = currentModel->Classify(record);

        if(classOutput.is_open())
        {
            inter<<classAtt.T1.get_str()+","+classAtt.T2.get_str()<< endl;
            extTestBd+=mpz_size(classAtt.T1.get_mpz_t())+ mpz_size(classAtt.T2.get_mpz_t())* sizeof(mp_limb_t);
        }

        try {

            delete record;

        }catch (exception e )
        {
            cout << "Trying to delete a record " << endl;
            cout << e.what() << endl;
        }

    }

    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    this->testTime = duration.count();

    /*this->testTimeSU = dtpkc.timeSU;
    this->testTimeMU = dtpkc.timeMU;*/
    this->testBdw = dtpkc.bdwSU + dtpkc.bdwMU;


    dtpkc.timeSU = 0.0;
    dtpkc.timeMU = 0.0;
    dtpkc.bdwMU = 0.0;
    dtpkc.bdwSU = 0.0;
    dtpkc.cmpCtr = 0;

    inter.close();
    //currentModel->print();

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

void PPNB_Classifier::train() {

    int counter =0;
    int size=datasetReader->train_size;
    PPRecord * record;


    dtpkc.timeSU = 0.0;
    dtpkc.timeMU = 0.0;
    dtpkc.bdwMU = 0.0;
    dtpkc.bdwSU = 0.0;
    dtpkc.cmpCtr = 0;
    this->addTime = 0.0, this->cmpTime =0.0 , this->divTime = 0.0 , this->logTime = 0.0;
    extTrainBd = 0;

    auto begin = chrono::high_resolution_clock::now();

    while (counter < size) {
        try {
            record = datasetReader->getTrainRecord();
            currentModel->UpdateStatistics(record);
            extTrainBd += record->getSize();
            try {

                delete record;

            }catch (exception e )
            {
                cout << "Trying to delete a record " << endl;
                cout << e.what() << endl;
            }

        }
        catch (std::exception const &e) {
            std::cout << "Exception: " << e.what() << "\n";

        }
        counter++;

    }

    currentModel->InitStats();


    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    this->trainTime = duration.count();

    //cout << "dtpkc.timeSU " << dtpkc.timeSU << " this->trainTimeSU " << this->trainTimeSU << " this->trainTime " << this->trainTime <<  endl;
    //this->trainTimeMU += dtpkc.timeMU;
    // //this->trainTimeSU += dtpkc.timeSU;
    this->trainBdw += dtpkc.bdwSU + dtpkc.bdwMU;


    dtpkc.timeSU = 0.0;
    dtpkc.timeMU = 0.0;
    dtpkc.bdwMU = 0.0;
    dtpkc.bdwSU = 0.0;
    dtpkc.cmpCtr = 0;


}


void PPNB_Classifier::RemoteTest(){

    int counter =0;
    int size=datasetReader->test_size;
    PPRecord * record;
    Cipher classAtt;
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
        classAtt = currentModel->Classify(record);
        if (debug)
        {
            //record->print();
            cout << "Test step : " << counter << " over " << size << endl;
        }
        sendResponse(this->sockfd, this->cliaddr, classAtt) ;

        try {

            delete record;

        }catch (exception e )
        {
            cout << "Trying to delete a record " << endl;
            cout << e.what() << endl;
        }

        if(inter.is_open())
        {
            inter<<classAtt.T1.get_str()+","+classAtt.T2.get_str()<< endl;
            extTestBd+=mpz_size(classAtt.T1.get_mpz_t())+ mpz_size(classAtt.T2.get_mpz_t());
        }


    }

    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = en  - begin ;
    this->testTime = duration.count();

   // this->testTimeMU = dtpkc.timeMU;
    this->testBdw = dtpkc.bdwSU + dtpkc.bdwMU;

    //this->testTimeSU = this->testTime - this->testTimeMU;

    dtpkc.timeSU = 0.0;
    dtpkc.timeMU = 0.0;
    dtpkc.bdwMU = 0.0;
    dtpkc.bdwSU = 0.0;

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
