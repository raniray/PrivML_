#include <chrono>
#include "PPNN_Classifier.h"


#define MAX_DATE 120
using namespace std;




PPNN_Classifier::PPNN_Classifier(vector<Cipher> weights, int alpha, int epochs, int batchSize, int th,
                                 PPDatasetReader *datasetReader, string log,
                                 bool debug, string mainpath)
{
    this->datasetReader= datasetReader;
    this->logfile = log;
    this->debug=debug;
    this->dtpkc=datasetReader->dtpkc;
    this->trainTime=0;
    this->extTrainBd=0;
    this->testTime=0;
    this->clientTestTime=0;
    this->extTestBd=0;
    //this->w = weights;
    this->alpha = alpha;
    this->batchSize=batchSize;
    //this->th = dtpkc.enc(th, dtpkc.pkw);
    this->dt = dt;
    this->epochs=epochs;
    this->mainpath = mainpath;

    zero =  dtpkc.enc(0,dtpkc.pkw);
    one =  dtpkc.enc(1,dtpkc.pkw);





}




Cipher PPNN_Classifier::predict(PPRecord *r, bool test ) {


    Cipher res =  zero;

    return res;
}


void PPNN_Classifier::train() {


}


void PPNN_Classifier::Test() {

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


