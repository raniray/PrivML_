/**
** Class :  DatasetReader
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : This class models a test in a given node in a decision tree
 * Note : The test is of the form A<Th
**/

#ifndef DAPPLE_NODETEST_H
#define DAPPLE_NODETEST_H
#include <map>
#include "../IO/Record.h"



class NodeTest {

public :
    double attribute;
    double value;

    /******* Functions **********/

public :
    NodeTest(int attribute, double value);


public:
    bool Eval(Record * r);




};
#endif

