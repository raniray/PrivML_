#include "TESTNppNB.h"

void
TESTNppNB::run() {

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

    try {
        cfg->parse(configFile);
        path        = cfg->lookupString(scope, "path");
        full        = cfg->lookupBoolean(scope, "full");
        chunk_prefix=  cfg->lookupString(scope, "chunk_prefix");
        classNumber = cfg->lookupInt(scope,"class_number");
        dimension = cfg->lookupInt(scope,"dimension");
        chunk_number = cfg->lookupInt(scope,"chunk_number");

    } catch(const config4cpp::ConfigurationException & ex) {
        cerr << ex.c_str() << endl;
        cfg->destroy();
    }

    cfg->destroy();

    cout << "Config file read" << endl;
    DatasetReader * datasetReader = new DatasetReader (path,false,chunk_prefix,classNumber, dimension);
    cout << "Dataset read " << endl;
    Stats * stats = new Stats (classNumber, dimension-1);
    Stats::initialised=false;
    //NB_Classifier * classifier = new NB_Classifier(datasetReader,stats,1, "");
    cout << "Classification started  " << endl;
    //classifier->NB_Initialise();
    for (int i=0; i < chunk_number -1 ; i++)
    {
        cout << " Chunk n°: " << i+1 << " from: " << chunk_number - 1 << endl;
        datasetReader->nextChunk();
        //Err= classifier->train();
    }

    cout << "Classification ended : err = " << Err << endl;
    //classifier->Test();
    cout << "Classification eval ended" << endl;
    datasetReader->closeTraining();
    datasetReader->closeTesting();

}
