/**
** Class :  DatasetReader
 * Authors : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
** Purpose : Read a dataset
 *
 * Note to me  : testing dataset does not contain class labels : may be  I should open it in the dataset reader
 *
**/

#ifndef DAPPLE_DATASETREADER_H
#define DAPPLE_DATASETREADER_H

#include <fstream> // to manage file streams
#include <stdio.h> //
#include <string.h>// needed for strtok
#include <vector>
#include <cstring>
#include "Record.h"
#include "../../UTIL/mpz_class.hh"
#include <time.h>
#include <string>
#include "omp.h"

using  namespace std;

class DatasetReader {

        public :
                static std::string path, chunk_prefix;
                static int test_size, train_size, current_chunk, current_train_record, current_test_record, dimension, classNumber;
                static bool full;
                static std::ifstream *train, *test;
                bool lr = false;


    /******* Functions **********/

    public :
    DatasetReader(string path,bool full,string chunk_prefix, int classNumber, int dimension, bool lr=false);

    public:
    Record * getTrainRecord();

    public :
    Record * getTestRecord();

    public :
    void nextChunk();

    public :
    void closeTraining();

    public:
    void closeTesting();

    public:
    void reinit();

    public:
    void reloadTest();

};


#endif //DAPPLE_DATASETREADER_H

