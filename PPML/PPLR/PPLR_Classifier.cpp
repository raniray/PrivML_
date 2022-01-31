#include <chrono>
#include "PPLR_Classifier.h"


#define MAX_DATE 120
using namespace std;





double PPLR_Classifier::trainTimeSU;
double PPLR_Classifier::trainTimeMU;
double PPLR_Classifier::trainBdw;


double PPLR_Classifier::testTimeSU;
double PPLR_Classifier::testTimeMU;
double PPLR_Classifier::testBdw;

int PPLR_Classifier::updateLrCtr, PPLR_Classifier::subCtr, PPLR_Classifier::dotProdCtr, PPLR_Classifier::sigmoidCtr, PPLR_Classifier::cmpCtr;

double PPLR_Classifier::updateLrTime, PPLR_Classifier::subTime, PPLR_Classifier::dotProdTime, PPLR_Classifier::sigmoidTime, PPLR_Classifier::cmpTime;


void PPLR_Classifier::shuffle(PPDatasetReader * dt)
{
    if (dt->full) {
        dt->closeTraining();
        string datasetPath = dt->path + dt->chunk_prefix + "_" + std::to_string(dt->current_chunk) + ".enc";
        string filename = mainpath + "DATA/Scripts/Shuffle_enc.py ";
        std::string command = "python3 ";
        std::string arguments = datasetPath;
        command += filename + arguments;
        system(command.c_str());
        dt->current_chunk--;
        dt->nextChunk();

    } else {

        dt->closeTraining();
        string datasetPath = dt->path + "training.enc";
        string filename = mainpath + "DATA/Scripts/Shuffle_enc.py ";
        std::string command = "python3 ";
        std::string arguments = datasetPath;
        command += filename + arguments;
        system(command.c_str());
        delete dt->train;
        dt->train=new ifstream(dt->path+"training.enc");

    }
}

Cipher PPLR_Classifier::Sigmoid(Cipher x) {

    /*EvalSigmoid evalSigmoid(x,100,dtpkc);
    vector<Cipher> in = evalSigmoid.EvalSig_U1_step1();
    Cipher C_C = evalSigmoid.EvalSig_U1_step3(evalSigmoid.EvalSig_U2_step2(in[0],in[1]));
    return C_C;*/
    Cipher C_C;
    Cipher fifty = dtpkc.enc(50, x.Pub);

    C_C = fifty;
    C_C.T1 = C_C.T1 * x.T1;
    C_C.T2 = C_C.T2 * x.T2;
    C_C.Pub = x.Pub;
    C_C.sign = x.sign;


    return C_C;
}

PPRecord * PPLR_Classifier::recvPPRecord(int sockfd, sockaddr_in cliaddr, int dim, DTPKC dtpkc, int  id)
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

    //if (debug) cout << "I have recieved this message : " << recievedRec << endl;
    //if (debug) cout << "The message size is : " << recievedRec.size() << endl;

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

void PPLR_Classifier::sendResponse(int sockfd, sockaddr_in cliaddr, Cipher res)
{
    char buffer[4001];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer,"%s,%s", res.T1.get_str().c_str(), res.T2.get_str().c_str());
    sendto(sockfd, buffer, 4001, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent response " << endl;
}

int
PPLR_Classifier::sendMsg(string msg, int sockfd, sockaddr_in cliaddr)
{

    int n = sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    if (debug) cout << "sent msg " << msg << endl;
    return  n;
}

string
PPLR_Classifier:: recvMsg(int sockfd,  sockaddr_in cliaddr)
{
    string res ="";
    socklen_t addrlen = sizeof(cliaddr);
    char buf[200];
    int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);
    if (n!=-1) {
        buf[n] = '\0';
        res = buf;
        res = buf;
        if (debug) cout << "received msg " << res << endl;
    }
    return res;
}


PPLR_Classifier::PPLR_Classifier(vector<Cipher> weights, int alpha, int epochs, int batchSize, int th,
                                 PPDatasetReader *datasetReader, string log, int sockfd, sockaddr_in cliaddr,
                                 bool debug, string mainpath)
{
    this->datasetReader= datasetReader;
    this->logfile = log;
    this->debug=debug;
    this->dtpkc=datasetReader->dtpkc;
    this->sockfd=sockfd;
    this->cliaddr=cliaddr;
    this->trainTime=0;
    this->extTrainBd=0;
    this->testTime=0;
    this->clientTestTime=0;
    this->extTestBd=0;
    this->w = weights;
    this->alpha = alpha;
    this->batchSize=batchSize;
    this->th = dtpkc.enc(th, dtpkc.pkw);
    this->dt = dt;
    this->epochs=epochs;
    this->mainpath = mainpath;

    zero =  dtpkc.enc(0,dtpkc.pkw);
    one =  dtpkc.enc(1,dtpkc.pkw);




    this->trainTimeSU=0;
    this->trainTimeMU=0;
    this->trainBdw=0;


    this->testTimeSU=0;
    this->testTimeMU=0;
    this->testBdw=0;

    this->updateLrTime=0.0; this->subTime=0.0; this->dotProdTime=0.0; this->sigmoidTime=0.0; this->cmpTime=0.0;

    this->updateLrCtr=0; this->subCtr=0; this->dotProdCtr=0; this->sigmoidCtr=0; this->cmpCtr=0;

    if (debug )
    {
    for (int i=0; i< w.size(); i++)
    cout << "w["<<i<<"]= " << w[i] <<endl;

    cout << "alpha " << alpha << endl;
    cout << "batchSize " << batchSize<< endl;
    cout << "th " << th << endl;
    cout << "logfile " << logfile << endl;

    cout << ""<< endl;
    }



}


Cipher PPLR_Classifier::dotProduct(vector<Cipher> w, PPRecord * r, int size )
{
    EvalDotProd evalDotProd(w, r->values,size, dtpkc);
    Cipher C_C = evalDotProd.EvalDotProd_U1_step3(evalDotProd.EvalDotProd_U2_step2(evalDotProd.EvalDotProd_U1_step1()));
    return C_C;
}

Cipher PPLR_Classifier::predict(PPRecord *r, bool test ) {

    Cipher wx = zero;
    Cipher proba = zero;
    Cipher res =  zero;
    int size = r->values.size()-1;

    if (test) size = size + 1;
    auto begin = chrono::high_resolution_clock::now();
    wx = dotProduct(w,r, size);
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    PPLR_Classifier::dotProdTime += duration.count();

    begin = chrono::high_resolution_clock::now();
    proba = Sigmoid(wx);
    end = chrono::high_resolution_clock::now();
    duration = end  - begin ;
    PPLR_Classifier::sigmoidTime += duration.count();

    this->dotProdCtr++;
    this->sigmoidCtr++;
    this->cmpCtr++;

    begin = chrono::high_resolution_clock::now();
    if ((proba < th) or (proba.sign == 0))
        res = zero;
    else
        res = one;

    end = chrono::high_resolution_clock::now();
    duration = end  - begin ;
    PPLR_Classifier::cmpTime += duration.count();

    if (debug)
    {
        cout << "WX " << wx << endl;
        cout << "Proba " << proba << endl;
        cout << "Res " << res << endl;
        cout << "Th " << th << endl;
    }


    return res;
}


vector<Cipher> PPLR_Classifier::miniBatchGrad(vector<PPRecord *> XB) {

    vector<Cipher> ypred, ytrue;
    PPRecord *r;
    int dim = XB[0]->values.size() - 1;

    // Compute XB transpose
    Cipher transpose [dim] [XB.size()];

    // Predict labels, compute error and transpose
    for (int j=0; j< XB.size(); j++ )
    {
        r = XB[j];
        if (debug) r->print();

        ypred.push_back(predict(r,false));
        ytrue.push_back(r->values[r->values.size()-1]);
        cout << " ypred "<< predict(r,false) << endl;
        cout << " ytrue "<< r->values[r->values.size()-1] << endl;
        for (int i=0; i < dim ; i++ )
        {
            transpose [i][j] = r->values[i];

        }

    }

    vector< Cipher> prod(dim);

    auto begin = chrono::high_resolution_clock::now();
    #pragma omp parallel for shared (prod, ypred, ytrue, transpose) schedule(static)
    for (int i=0; i < dim ; i++ )
    {

        vector<Cipher> line;

        for (int j=0; j< XB.size(); j++ )
        {
            Cipher tmp = transpose[i][j];
            line.push_back(tmp);
        }

        EvalLrUpdate evalLrUpdate(ypred, ytrue, line  ,alpha, dtpkc);
        Cipher inter = evalLrUpdate.EvalLrUpdate_U1_step3(evalLrUpdate.EvalLrUpdate_U2_step2(evalLrUpdate.EvalLrUpdate_U1_step1()));

        //#pragma omp critical
        {

            prod[i] = inter;

        };

    }
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    if (PPLR_Classifier::updateLrTime += duration.count() - 0.3 * dim > 0 ) PPLR_Classifier::updateLrTime += duration.count() - 0.3 * dim;
    else PPLR_Classifier::updateLrTime += duration.count();
    PPLR_Classifier::subTime += 0.3*dim;
    this->updateLrCtr +=dim;

    if (debug ) {

        for (int i = 0; i < w.size(); i++)
            cout << "prod[" << i << "]= " << prod[i] << endl;

    }
    return  prod;

}



void PPLR_Classifier::train() {

    int counter = 0;
    int recordCounter = 0;
    int sizeBatch = batchSize;
    int size = dt->train_size;
    PPRecord *record;
    extTrainBd = 0;
    vector<PPRecord *> XB;
    double t1, t2;

    auto begin = chrono::high_resolution_clock::now();

    for (int epochCpt = 0; epochCpt < epochs; epochCpt++) {

        //shuffle(dt);
        while (counter < size) {
            if (size - counter < batchSize)
                sizeBatch = size - counter;


            for (recordCounter = 0; recordCounter < sizeBatch; recordCounter++) {
                try {

                    record = dt->getTrainRecord();
                    XB.push_back(record);
                    extTrainBd += record->getSize();
                    counter++;
                }
                catch (std::exception const &e) {
                    cout << e.what() << endl;
                }

            }

            vector<Cipher> grad = miniBatchGrad(XB);

            #pragma omp parallel for
            for (int i = 0; i < w.size(); i++) {
                EvalSub2 sub(w[i], grad[i], dtpkc);
                w[i] = sub.EvalSub_U1();
            }


            if (debug) {
                for (int i = 0; i < w.size(); i++) {
                    cout << "W after update " << w[i] << endl;
                    cout << grad[i] << endl;
                }
            }

            for (int i = 0; i < XB.size(); i++) {
                delete XB[i];
            }

            XB.clear();


        }

        counter = 0;

    }

    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - begin;

    trainTime = duration.count();

    t1 = 0.44092*subCtr + 1046.09*dotProdCtr + 3048*sigmoidCtr + 6.39*cmpCtr + 1046.53*updateLrCtr;

    t2 = 25.07*dotProdCtr + 6*sigmoidCtr + 6.56*cmpCtr + 25.07*updateLrCtr;

    this->trainTimeMU = t1/(t1+t2);

    this->trainTimeSU = t2/(t1+t2);

    this->trainBdw = 2144*dotProdCtr + 1504*sigmoidCtr + 769*cmpCtr + 2144*updateLrCtr;
}


void PPLR_Classifier::Test() {

    int counter =0;
    int size=datasetReader->test_size;
    PPRecord * record;
    Cipher classAtt;
    std::ofstream classOutput;
    std::ofstream inter;
    inter.open(logfile+".enc");
    classOutput.open (logfile);
    double t1,t2;

    extTestBd = 0;
    auto begin = chrono::high_resolution_clock::now();
    while (counter < size) {

        try {
            record = datasetReader->getTestRecord();
            if (debug) record->print();
            extTestBd += record->getSize();
        }
        catch (std::exception const &e) {
            std::cout << "Exception: " << e.what() << "\n";
        }

        counter++;
        classAtt = predict(record,true);

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
    inter.close();


    t1= 10456.09*dotProdCtr + 3048*sigmoidCtr + 6.39*cmpCtr;
    t2= 25.07*dotProdCtr + 6*sigmoidCtr + 6.56*cmpCtr;
    this->testBdw= 2144*dotProdCtr + 1504*sigmoidCtr + 769*cmpCtr;
    this->testTimeMU = t1/(t1+t2);
    this->testTimeSU = t2/(t1+t2);


    //classOutput.close();

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

void PPLR_Classifier::RemoteTest(){

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
        classAtt = predict(record,true);

        if (debug)
        {
            record->print();
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
