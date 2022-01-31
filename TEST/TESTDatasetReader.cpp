

#include<iostream>
#include "TESTDatasetReader.h"
#include "../ML/IO/DatasetReader.cpp"
#include "../LIB/config4cpp/include/config4cpp/Configuration.h"
using namespace std;
using namespace config4cpp;



void TESTDatasetReader :: run () {

    /**
     * Test config4cpp :: worked
     */

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

     DatasetReader datasetReader(path,full,chunk_prefix,3,15);


    /**
     * Test Record getTrainRecord();
     */

     Record * r = datasetReader.getTrainRecord();
     r->print();


    /**
     * Test Record getTestRecord();
     */

    r = datasetReader.getTestRecord();
    r->print();

    /**
     *
     * Test void nextChunk();
     */
     datasetReader.closeTraining();
     datasetReader.closeTesting();

     DatasetReader datasetReader2(path,!full,chunk_prefix,3,15);
     r=datasetReader2.getTrainRecord();
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

