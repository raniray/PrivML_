#include<iostream>
#include "TESTCryptoDatasetReader.h"

using namespace std;
using namespace config4cpp;



void TESTCryptoDatasetReader :: run () {


    Configuration *  cfg = Configuration::create();
    const char *     scope = "dataset";
    const char *     configFile = "../CONFIG/DAPPLE.config";
    string  path;
    bool full;
    string chunk_prefix="";

    try {
        cfg->parse(configFile);
        path        = cfg->lookupString(scope, "path");
        full        = cfg->lookupBoolean(scope, "full");
        chunk_prefix=  cfg->lookupString(scope, "chunk_prefix");

    } catch(const ConfigurationException & ex) {
        cerr << ex.c_str() << endl;
        cfg->destroy();
    }

    cout << "path=" << path << " chunk pref =" << chunk_prefix
         << " full=" << full
         << endl;

    cfg->destroy();

    /**
    * Test  DatasetReader(std::string path,boolean full,std::string chunk_prefix="");
    */



    gmp_randstate_t randstate;
    DTPKC dtpkc;
    mpz_class pkey, skey, newPkey, newSkey;

    // Generate public parameters of DTPKC
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    //dtpkc.keygen(randstate, 1024,600);
    dtpkc.deserializeDtpkc("/home/rania/CLionProjects/DAPPLE/SER");

    // Generate public key
    dtpkc.getKey(pkey,skey);
    dtpkc.updatePkw(pkey);

    /**
     * Test encrypt file, decrypt and change encryption key
     */

    PPDatasetReader datasetReader (dtpkc);

    datasetReader.encryptData(path+"DO_1.data", path+"CRYPTO/DO_1.enc", pkey);
    datasetReader.encryptData(path+"DO_2.data", path+"CRYPTO/DO_2.enc", pkey);
    datasetReader.encryptData(path+"DO_3.data", path+"CRYPTO/DO_3.enc", pkey);
    datasetReader.encryptData(path+"DO_4.data", path+"CRYPTO/DO_4.enc", pkey);
    datasetReader.encryptData(path+"DO_5.data", path+"CRYPTO/DO_5.enc", pkey);
    datasetReader.encryptData(path+"DO_6.data", path+"CRYPTO/DO_6.enc", pkey);
    datasetReader.encryptData(path+"DO_7.data", path+"CRYPTO/DO_7.enc", pkey);
    datasetReader.encryptData(path+"DO_8.data", path+"CRYPTO/DO_8.enc", pkey);

    datasetReader.encryptData(path+"training.data", path+"CRYPTO/training.enc", pkey);
    datasetReader.encryptData(path+"testing.data", path+"CRYPTO/testing.enc", pkey);

    //datasetReader.decryptFile(path+"CRYPTO/testing.enc", path+"testDecTEST.data");

    // Generate public key
    //dtpkc.getKey(newPkey,newSkey);
    //dtpkc.updatePkw(newPkey);

    //datasetReader.changeCryptoKey(path+"CRYPTO/training.enc", path+"CRYPTO/training2.enc",newPkey);
    //datasetReader.decryptFile(path+"CRYPTO/training2.enc", path+"testDec2.data");

    //dtpkc.deserializeDtpkc();
    //PPDatasetReader datasetReader (path,full,chunk_prefix,3, 10, dtpkc);

    /**
     * Test Record getTrainRecord();
     */

     //PPRecord * r = datasetReader.getTrainRecord();
     //r->print();


    /**
     * Test Record getTestRecord();
     */

    //r = datasetReader.getTestRecord();
    //r->print();

    /**
     *
     * Test void nextChunk();
     */

     //datasetReader.closeTraining();
     //datasetReader.closeTesting();

     PPDatasetReader datasetReader2(path,!full,chunk_prefix,3,10,dtpkc);
     PPRecord * r=datasetReader2.getTrainRecord();
     r->print();

     datasetReader2.nextChunk();
     r=datasetReader2.getTrainRecord();
     r->print();

     datasetReader2.nextChunk();
     r=datasetReader2.getTrainRecord();
     r->print();

    /**
     * Test closeTraining();
     */

     datasetReader2.closeTraining();

    /**
     * Test void closeTesting();
     */

    datasetReader2.closeTesting();

}

