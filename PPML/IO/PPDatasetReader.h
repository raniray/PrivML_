#ifndef DAPPLE_PPDATASETREADER_H
#define DAPPLE_PPDATASETREADER_H
#include "PPRecord.h"
#include <fstream> // to manage file streams
#include <stdio.h> //
#include <string.h>// needed for strtok
#include "../../CRYPTO/SkeySwitch.h"
#include <vector>
#include <cstring>


using  namespace std;

class PPDatasetReader {


        public :
                static std::string path, chunk_prefix;
                static int test_size, train_size, current_chunk, current_train_record, current_test_record, dimension, classNumber;
                static bool full;
                static std::ifstream *train, *test;
                static mpz_class trainKey;
                static mpz_class testKey;
                static DTPKC dtpkc;
                static bool debug;
                static int maxTrainSize;
                static int maxTestSize;
                static Cipher one;
                static bool lr;


    /******* Functions **********/

    public :
    PPDatasetReader(string path,bool full,string chunk_prefix, int classNumber, int dimension, DTPKC dtpkc, bool debug=false, int maxTrainSize=0, int maxTestSize=0, bool lr = false);

    public :
    PPDatasetReader(DTPKC dtpkc);

    public:
    PPRecord * getTrainRecord();


    public :
    PPRecord * getTestRecord();


    public :
    void nextChunk();

    public :
    void closeTraining();


    public:
    void closeTesting();

    public:
    void encryptData(string Inpath, string OutPath, mpz_class pkey);

    public:
    void decryptFile(string Inpath, string outPath);

    public:
    void changeCryptoKey(string Inpath, string outPath, mpz_class pkey);

    public:
    void reinit();

    public :
    void reloadTest();
};


#endif

