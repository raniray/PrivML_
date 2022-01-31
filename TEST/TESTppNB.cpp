#include "TESTppNB.h"



void
TESTppNB::run() {

    config4cpp::Configuration *  cfg = config4cpp::Configuration::create();
    const char *     scope = "dataset";
    const char *     configFile = "../CONFIG/DAPPLE.config";
    string  path;
    bool full;
    int classNumber;
    double Err = 0.0;
    string chunk_prefix="";
    int dimension =0;
    int  chunk_number = 0;
    int thread_number =1;

    try {
        cfg->parse(configFile);
        path        = cfg->lookupString(scope, "path");
        full        = cfg->lookupBoolean(scope, "full");
        chunk_prefix=  cfg->lookupString(scope, "chunk_prefix");
        classNumber = cfg->lookupInt(scope,"class_number");
        dimension = cfg->lookupInt(scope,"dimension");
        chunk_number = cfg->lookupInt(scope,"chunk_number");
        scope ="nb";
        thread_number = cfg->lookupInt(scope,"thread_number");

    } catch(const config4cpp::ConfigurationException & ex) {
        cerr << ex.c_str() << endl;
        cfg->destroy();
    }

    cfg->destroy();

    gmp_randstate_t randstate;
    DTPKC dtpkc;


    // Generate public parameters of DTPKC
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));


    dtpkc.deserializeDtpkc("/home/rania/CLionProjects/DAPPLE/SER");

    cout << "Config file read" << endl;
    PPDatasetReader * datasetReader = new PPDatasetReader (path,false,chunk_prefix,classNumber, dimension, dtpkc);
    cout << "Dataset read " << endl;
    PPStats * stats = new PPStats (classNumber, dimension-1);
    //PPNB_Classifier * classifier = new PPNB_Classifier(datasetReader,stats,1,"");
    cout << "Classification started  " << endl;
    //classifier->NB_Initialise();
    cout << " Chunk n°: " << 1 << " from: " << chunk_number - 1 << endl;
    datasetReader->nextChunk();
    datasetReader->nextChunk();
    datasetReader->nextChunk();
    datasetReader->nextChunk();
    //Err= classifier->train();
    /*for (int i=0; i < chunk_number -1 ; i++)
    {
        cout << " Chunk n°: " << i+1 << " from: " << chunk_number - 1 << endl;
        datasetReader->nextChunk();
        Err= classifier->train();
    }*/
    cout << "Classification ended : err = " << Err << endl;
    //classifier->Test();
    cout << "Classification eval ended" << endl;
    datasetReader->closeTraining();
    datasetReader->closeTesting();

}
