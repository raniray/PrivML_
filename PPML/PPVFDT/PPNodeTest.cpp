/**
** Class :  NodeTest
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : This class models a test in a given node in a decision tree
**/

#include "PPNodeTest.h"
#include "../../CRYPTO/EvalCmp100.h"
#include "PPVFDT_Classifier.h"


PPNodeTest:: PPNodeTest(int attribute, Cipher value)
{
    this->attribute=attribute;
    this->value=value;
}


bool PPNodeTest::Eval(PPRecord *r, DTPKC dtpkc)
{

    auto begin = chrono::high_resolution_clock::now();
    EvalCmp100 evalCmp(r->values [this->attribute],this->value, dtpkc);
    bool test = evalCmp.EvalCmp_U2_step2(evalCmp.EvalCmp_U1_step1());
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    PPVFDT_Classifier::cmpTime += duration.count();
    PPVFDT_Classifier::cmpCtr++;
    PPVFDT_Classifier::testBdw +=evalCmp.bdwSU + evalCmp.bdwMU;

    if (test)

        return true;

    else

        return false;
}

