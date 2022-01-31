/**
** Class :  DatasetReader
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : This class models a test in a given node in a decision tree
 * Note : The test is of the form A<Th
**/

#ifndef DAPPLE_PPNODETEST_H
#define DAPPLE_PPNODETEST_H
#include <map>
#include "../IO/PPRecord.h"
#include "../../CRYPTO/EvalCmp.h"



class PPNodeTest {


public :
    int attribute;
    Cipher value;


public :
    PPNodeTest(int attribute, Cipher value);

public:
    bool Eval(PPRecord * r, DTPKC dtpkc);


};
#endif

