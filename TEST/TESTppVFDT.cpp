#include "TESTppVFDT.h"



void
TESTppVFDT::run() {

    config4cpp::Configuration *  cfg = config4cpp::Configuration::create();
    const char *     scope = "dataset";
    const char *     configFile = "../CONFIG/DAPPLE.config";
    string  path;
    bool full;
    int classNumber, grace = 0;
    int max_depth = 4;
    int thresholdNumber =2;
    float tau, delta;
    string chunk_prefix="";
    double Err = 0.0;
    int dimension =0;
    int portQ = 3333;
    int numbTh =1;

    try {

        cfg->parse(configFile);
        path        = cfg->lookupString(scope, "path");
        full        = cfg->lookupBoolean(scope, "full");
        chunk_prefix=  cfg->lookupString(scope, "chunk_prefix");
        classNumber = cfg->lookupInt(scope,"class_number");
        dimension = cfg->lookupInt(scope,"dimension");

        scope = "vfdt";
        delta = cfg->lookupFloat(scope,"delta");
        tau = cfg->lookupFloat(scope,"tau");
        grace = cfg->lookupInt(scope, "grace");
        max_depth = cfg->lookupInt(scope, "max_depth");
        thresholdNumber = cfg->lookupInt(scope, "threshold_number");
        numbTh =  cfg->lookupInt(scope, "thread_number");

        scope = "network";
        portQ = cfg->lookupInt(scope, "portQ");

    } catch(const config4cpp::ConfigurationException & ex) {
        cerr << ex.c_str() << endl;
        cfg->destroy();
    }

    cfg->destroy();


    gmp_randstate_t randstate;
    DTPKC dtpkc;


    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));


    dtpkc.deserializeDtpkc("/home/rania/CLionProjects/DAPPLE/SER");

    cout << "Config file read " << endl;

    PPDatasetReader * datasetReader = new PPDatasetReader (path,false,chunk_prefix,classNumber, dimension, dtpkc);
    std::cout << "Client is runing" << std::endl;
    cout << "Dataset read " << endl;
    PPNode * Tree = new PPNode (datasetReader->dimension -1, datasetReader->classNumber);
    const clock_t begin_time = clock();
    //PPVFDT_Classifier * classifier = new PPVFDT_Classifier(Tree, datasetReader, delta, tau, grace, max_depth, thresholdNumber,dtpkc, "");
    cout << "Classification started  " << datasetReader->test_size <<  endl;
    //classifier->train();
    //classifier->intTrainBd += dtpkc.bd;
    //dtpkc.bd = 0;
    cout << "Classification ended : err = " << Err << endl;
    std::cout << "Time : " << float( clock () - begin_time ) /  CLOCKS_PER_SEC;
    //classifier->Test();
    //classifier->RemoteTest(portQ);
    //classifier->printStats();
    cout << "Classification eval ended" << endl;

}
