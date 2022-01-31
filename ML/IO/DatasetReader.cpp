/**
** Class :  DatasetReader
 * Authors : Talbi Rania
** Creation date : 13/11/2018
** Last Update : 13/11/2018
** Purpose : Read a dataset
 *
**/

#include "DatasetReader.h"

using namespace std;

std::string DatasetReader::path, DatasetReader::chunk_prefix;
int DatasetReader::test_size, DatasetReader::classNumber, DatasetReader::train_size, DatasetReader::current_chunk, DatasetReader::current_train_record, DatasetReader::current_test_record, DatasetReader::dimension;
bool DatasetReader::full;
std::ifstream *DatasetReader::train, *DatasetReader::test;

DatasetReader:: DatasetReader(string path,bool full,string chunk_prefix, int classNumber, int dimension, bool lr )
{
    std::string line;

    this->classNumber=classNumber;
    this->path=path;
    this->full=full;
    this->dimension=dimension;
    this->chunk_prefix=chunk_prefix;
    this->test= new std::ifstream(path+"testing.data");
    current_chunk=1;
    current_train_record=current_test_record=0;
    if (full)
        this->train=new ifstream(path+chunk_prefix+"_1.data");
    else
        this->train=new ifstream(path+"training.data");

    while (getline(*this->train, line)) {
        ++this->train_size;
    }
    this->train->clear();
    this->train->seekg(0);


    while (getline(*this->test, line)) {
        ++this->test_size;
    }
    this->test->clear();
    this->test->seekg(0);
    this->lr = lr;


}



void DatasetReader::reinit() {

    std::string line;
    delete this->test;
    this->test= new std::ifstream(path+"testing.data");
    current_chunk=1;
    current_train_record=0;
    current_test_record=0;
    delete this->train;
    if (full)
        this->train=new ifstream(path+chunk_prefix+"_1.data");
    else
        this->train=new ifstream(path+"training.data");

    this->train_size = 0;
    while (getline(*this->train, line)) {
        ++this->train_size;
    }
    this->train->clear();
    this->train->seekg(0);

    this->test_size=0;
    while (getline(*this->test, line)) {
        ++this->test_size;
    }
    this->test->clear();
    this->test->seekg(0);

}

void DatasetReader::reloadTest()
{
    string line;
    test->clear();
    test->seekg(0);
    current_test_record=0;

}

Record *
DatasetReader::getTrainRecord()
{
    string line;
    char *v;
    std::vector<int> values;
    // remove if not lr
    //values.push_back(1);
    Record * r;


    if (train->is_open()) {

        std::getline(*train, line);
        if (train->eof())
            train->close();

        if (lr)
            values.push_back(1);
        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());
        v=strtok (dup,",");
        while (v != NULL)
        {
            values.push_back(std::atoi(v));
            v=strtok (NULL,",");
        }
        delete [] dup;
        this->current_train_record++;
        r = new  Record (this->current_train_record,values);

        return r;

    }
    else {

        throw std::runtime_error("There is no train next record");

    }
}

Record *
DatasetReader::getTestRecord()
{
    string line;
    char *v;
    std::vector<int> values;
    // remove if not lr
    //values.push_back(1);
    Record * r;

    if (test->is_open()) {


        std::getline(*test, line);

        if (test->eof())
        {
            test->clear();
            test->seekg(0);
        }
        if (lr)
            values.push_back(1);

        char * dup =  new char[line.length() + 1];
        std::strcpy(dup,line.c_str());

        v=strtok (dup,",");
        while (v != NULL)
        {
            values.push_back(std::atoi(v));
            v=strtok (NULL,",");
        }
        delete [] dup;
        free (v);
        this->current_test_record++;
        r = new Record (this->current_test_record,values);

        return r;

    }
    else {

        throw std::runtime_error("There is no test next record");

    }
}

void
DatasetReader::nextChunk()
{
    string line;
    this->current_chunk++;
    delete this->train;
    this->train=new std::ifstream(path+chunk_prefix+"_"+ std::to_string(current_chunk)+".data");
    this->train_size=0;

    while (getline(*this->train, line)) {
        ++this->train_size;
    }
    this->train->clear();
    this->train->seekg(0);
}


void
DatasetReader::closeTraining()
{
    this->train->close();
}


void
DatasetReader::closeTesting()
{
    this->test->close();
}



