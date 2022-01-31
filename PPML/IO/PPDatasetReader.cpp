/**
** Class :  PPDatasetReader
 * Authors : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
** Purpose : Read a dataset
 *
**/
#include "PPDatasetReader.h"
#include <iostream>

using namespace std;

std::string PPDatasetReader::path, PPDatasetReader::chunk_prefix;
int PPDatasetReader::test_size, PPDatasetReader::classNumber, PPDatasetReader::train_size, PPDatasetReader::current_chunk, PPDatasetReader::current_train_record, PPDatasetReader::current_test_record, PPDatasetReader::dimension;
bool PPDatasetReader::full;
Cipher PPDatasetReader::one;
std::ifstream *PPDatasetReader::train, *PPDatasetReader::test;
DTPKC PPDatasetReader::dtpkc;
mpz_class PPDatasetReader::trainKey, PPDatasetReader::testKey;
bool PPDatasetReader::lr;
bool PPDatasetReader::debug;
int PPDatasetReader::maxTrainSize;
int PPDatasetReader::maxTestSize;


PPDatasetReader::PPDatasetReader(DTPKC dtpkc)
{
    this->dtpkc=dtpkc;
}

PPDatasetReader:: PPDatasetReader(string path,bool incremental,string chunk_prefix, int classNumber, int dimension, DTPKC dtpkc, bool debug, int maxTrainSize, int maxTestSize, bool lr)
{
    std::string line;
    this->classNumber=classNumber;
    this->path=path;
    this->full=incremental;
    this->dimension=dimension;
    this->chunk_prefix=chunk_prefix;
    this->test= new std::ifstream(path+"testing.enc");
    this->debug = debug;
    this->maxTestSize=maxTestSize;
    this->maxTrainSize = maxTrainSize;
    this->lr = lr;

    current_chunk=1;
    current_train_record=current_test_record=0;
    if (full)
        this->train=new ifstream(path+chunk_prefix+"_1.enc");
    else
        this->train=new ifstream(path+"training.enc");

    //Compute the size of the training (chunk) dataset

    getline(*this->train, line);
    this->trainKey.set_str(line, 10);

    if (!debug) {
        while (getline(*this->train, line)) {
            ++this->train_size;
        }

        this->train->clear();
        this->train->seekg(0);
        getline(*this->train, line);

    }


    else
        this->train_size=maxTrainSize;


        // Compute the size of the test set
        getline(*this->test, line);
        this->testKey.set_str(line, 10);

        if (!debug) {
            while (getline(*this->test, line))
                ++this->test_size;


            this->test->clear();
            this->test->seekg(0);
            getline(*this->test, line);

         }
         else {
                this->test_size=maxTestSize;

          }

    this->dtpkc=dtpkc;

    if (debug )
    {
        cout << "size of the training set is " << this->maxTrainSize << endl;
        cout << "size of the testing set is " << this->maxTestSize << endl;

    }
    this->one = dtpkc.enc(1,trainKey);

}



PPRecord *
PPDatasetReader::getTrainRecord()
{
    // line : read the next line in the train file
    // v: the next attribute value in this line
    // i : iterator through the added values of the record
    string line;
    char *v;
    std::vector<Cipher> values;
    PPRecord * r;

    // if training dataset is open
    if (train->is_open()) {

        // get the next line
        std::getline(*train, line);
        if (train->eof())
            train->close();

        // get the values of the record in the form of an integer
        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");
        if (lr)
            values.push_back(one);
        while (v != NULL)
        {
            Cipher c;
            c.Pub=this->trainKey;
            c.T1.set_str(v,10);

            v=strtok (NULL,",");

            if (v!=NULL)
            {
                c.T2.set_str(v,10);
                v=strtok (NULL,",");
            }

            values.push_back(c);
        }
        delete [] dup ;
        free(v);

        this->current_train_record++;
        r = new  PPRecord (this->current_train_record,values,dtpkc);

        return r;

    }
    else {

        throw std::runtime_error("There is no next record, train");

    }
}

PPRecord *
PPDatasetReader::getTestRecord()
{
    // line : read the next line in the test file
    // v: the next attribute value in this line
    // values : vector tha holds  the record values

    string line;
    char *v;
    std::vector<Cipher> values;
    PPRecord * r;

    // if training dataset is open
    if (test->is_open()) {

        // get the next line
        std::getline(*test, line);

        /*if (test->eof())
        {

            test->clear();
            test->seekg(0);
            getline(*this->train, line);
            current_test_record=0;
        }*/

        // get the values of the record in the form of an integer
        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());

        v=strtok (dup,",");
        if (lr)
            values.push_back(one);
        while (v != NULL)
        {
            Cipher c;
            c.Pub=this->testKey;
            c.T1.set_str(v,10);

            v=strtok (NULL,",");

            if (v!=NULL)
            {
                c.T2.set_str(v,10);
                v=strtok (NULL,",");
            }

            values.push_back(c);
        }
        delete [] dup;
        free(v);
        this->current_test_record++;
        r = new PPRecord (this->current_test_record,values,dtpkc);

        return r;

    }
    else {

        throw std::runtime_error("There is no next record, test");

    }
}


void
PPDatasetReader::nextChunk()
{
    string line;
    this->current_chunk++;
    delete this->train;
    this->train=new std::ifstream(path+chunk_prefix+"_"+ std::to_string(current_chunk)+".enc");
    this->train_size=0;

    // Update the training size for each chunk
    getline(*this->train, line);
    this->trainKey.set_str(line, 10);

    if (!debug) {
        while (getline(*this->train, line)) {
            ++this->train_size;
        }
        this->train->clear();
        this->train->seekg(0);
        getline(*this->train, line);
    } else
        this->train_size=maxTrainSize;
}


void
PPDatasetReader::closeTraining()
{
    this->train->close();
}


void
PPDatasetReader::closeTesting()
{
    this->test->close();
}


void PPDatasetReader::encryptData(string Inpath, string OutPath, mpz_class pkey)
{
    std::ifstream *In;
    std::ofstream *Out;
    string line, res="";
    char *v;

    In=new std::ifstream(Inpath);
    Out=new std::ofstream(OutPath);

    *Out<<pkey.get_str(10)<<endl;

    while (getline(*In, line)) {

        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");
        while (v != NULL)
        {
            Cipher c;
            mpz_class tmp;
            tmp.set_str(v,10);
            c=dtpkc.enc(tmp,pkey);
            res+=c.T1.get_str(10)+","+c.T2.get_str(10);

            v=strtok (NULL,",");
            if (v!=NULL)
                res+=",";
        }
        *Out << res <<endl;
        res="";
        delete []dup;
        free(v);
    }

    In->close();
    Out->close();

}



void PPDatasetReader::decryptFile(string Inpath, string outPath)
{

    std::ifstream *In;
    std::ofstream *Out;
    string line, res="";
    char *v;
    mpz_class key;

    In=new std::ifstream(Inpath);
    Out=new std::ofstream(outPath);

    getline(*In, line);
    key.set_str(line, 10);

    while (getline(*In, line)) {

        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");

        while (v != NULL)
        {
            Cipher c;
            c.Pub = key;
            c.T1.set_str(v,10);
            v=strtok (NULL,",");

             if (v!=NULL)
             {
                 c.T2.set_str(v,10);
                 mpz_class m = dtpkc.Sdec(c);
                 cout << " m = " << m.get_str() << endl;
                 res+=m.get_str(10);
                 v=strtok (NULL,",");

                 if (v!=NULL)
                     res+=",";
             }


        }
        *Out << res <<endl;
        res="";
        delete [] dup;
    }

    In->close();
    Out->close();
}


void PPDatasetReader::changeCryptoKey(string Inpath, string outPath, mpz_class pkey)
{
    std::ifstream *In;
    std::ofstream *Out;
    string line, res="";
    char *v;
    mpz_class key;

    In=new std::ifstream(Inpath);
    Out=new std::ofstream(outPath);

    *Out<< pkey.get_str(10) << endl;

    getline(*In, line);
    key.set_str(line, 10);

    while (getline(*In, line)) {

        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");

        while (v != NULL)
        {
            Cipher c;
            c.Pub = key;
            c.T1.set_str(v,10);
            v=strtok (NULL,",");
            if (v!=NULL)
            {
                c.T2.set_str(v,10);
                SkeySwitch sKeySwitch (c,pkey, dtpkc);

                cout << dtpkc.Sdec(c).get_str()<< endl;

                Cipher c2 = sKeySwitch.SkeySwitch_U1_step3(sKeySwitch.SkeySwitch_U2_step2(sKeySwitch.SkeySwitch_U1_step1()));

                //cout << dtpkc.Sdec(c2).get_str()<< endl;

                res += c2.T1.get_str(10)+","+c2.T2.get_str(10);
                v=strtok (NULL,",");
                if (v!=NULL)
                    res+=",";
            }

        }
        *Out << res <<endl;
        res="";
        delete [] dup;
    }

    In->close();
    Out->close();

}

void PPDatasetReader::reinit() {

    std::string line;
    delete  this->test;
    delete  this->train;
    this->test= new std::ifstream(path+"testing.enc");
    current_chunk=1;
    current_train_record=0;
    current_test_record=0;
    if (full)
        this->train=new ifstream(path+chunk_prefix+"_1.enc");
    else
        this->train=new ifstream(path+"training.enc");

    // Compute the size of the training (chunk) dataset

    this->dtpkc=dtpkc;
    if (!debug) {
        this->train_size = -1;
        while (getline(*this->train, line)) {
            ++this->train_size;
        }
        this->train->clear();
        this->train->seekg(0);

        this->test_size = -1;
        while (getline(*this->test, line)) {
            ++this->test_size;
        }
        this->test->clear();
        this->test->seekg(0);

    } else {
        getline(*this->train, line);
        getline(*this->test, line);
        this->test_size=maxTestSize;
        this->train_size=maxTrainSize;
    }

}

void PPDatasetReader::reloadTest()
{
    string line;
    test->clear();
    test->seekg(0);
    getline(*this->test, line);
    current_test_record=0;

}