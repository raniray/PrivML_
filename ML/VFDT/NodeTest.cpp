/**
** Class :  NodeTest
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : This class models a test in a given node in a decision tree
**/

#include "NodeTest.h"

NodeTest:: NodeTest(int attribute, double value)
{
    this->attribute=attribute;
    this->value=value;
}


bool NodeTest:: Eval( Record * r )
{

    bool a = r->values [this->attribute] < this->value;
   // cout << "Cmp " << r->values [this->attribute] << "< " << this->value << " : " << a << endl;

    if (r->values [this->attribute] < this->value)

        return true;

    else

        return false;
}

