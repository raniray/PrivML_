#include "TESTNppVFDT.h"


void
TESTNppVFDT::run() {

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
    int number_runs = 1;

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


    } catch(const config4cpp::ConfigurationException & ex) {
        cerr << ex.c_str() << endl;
        cfg->destroy();
    }

    cfg->destroy();

    cout << "Config file read " << endl;
    DatasetReader * datasetReader = new DatasetReader (path,!full,chunk_prefix,classNumber, dimension);
    cout << "Dataset read " << endl;
    Node * Tree = new Node (datasetReader->dimension -1, datasetReader->classNumber);
    //VFDT_Classifier * classifier = new VFDT_Classifier(Tree, datasetReader, delta, tau, grace, max_depth, thresholdNumber,"");
    cout << "Classification started  " << endl;
    //Err= classifier->train();
    cout << "Classification ended : err = " << Err << endl;
    //classifier->Test();
    cout << "Classification eval ended" << endl;


}
