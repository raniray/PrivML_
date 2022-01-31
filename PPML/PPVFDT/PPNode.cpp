/**
** Class :  PPNode
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : Represents a node in the decision tree
 *
**/

#include "PPNode.h"
#include <vector>
#include "PPNodeStats.h"
#include "PPNodeTest.h"
#include "PPVFDT_Classifier.h"

void PPNode::dump() {

    cout << "----------------------dump ----------------------" << endl;

    map <Cipher,Cipher> ::iterator i;
    for (i=nodeStats->classOccurrence.begin(); i != nodeStats->classOccurrence.end() ; ++i )
    {
        cout << "C( " << i->first << " ) = " << i->second << endl;
    }

    cout << "Most frequent class " << nodeStats->mostFrequentClass << " occ " << nodeStats->mostFrequentClassOccurrence << endl;
    cout << "Diff class  " << nodeStats->diffClass << endl;
    cout << "ClassAtt " << nodeStats->classAtt << endl;
    cout << "totSampleNum" << nodeStats->totSampleNum << endl;

    cout << "Sum of squares : " << endl;
    map <int,Cipher> ::iterator j;
    for (j=nodeStats->sumOfSquares.begin(); j != nodeStats->sumOfSquares.end() ; ++j )
    {
        cout << "( " << j->first << "," << j->second << " )" << endl;
    }


    cout << "Sum of values : " << endl;

    for (j=nodeStats->sumOfValues.begin(); j != nodeStats->sumOfValues.end() ; ++j )
    {
        cout << "( " << j->first << "," << j->second << " ) " << endl;
    }

    map <int, map <Cipher, map <Cipher, Cipher>> >:: iterator it;

    for (it=nodeStats->dataAVC.begin(); it != nodeStats->dataAVC.end() ; ++it ) {

        PPNodeStats::Stats &innerMap = it->second;
        PPNodeStats::Stats::iterator innerit;

        for (innerit = innerMap.begin(); innerit != innerMap.end(); ++innerit) {

            map<Cipher, Cipher> &innerMap2 = innerit->second;
            map<Cipher, Cipher>::iterator innerit2;

            for (innerit2 = innerMap2.begin(); innerit2 != innerMap2.end(); ++innerit2) {


                cout << "( " << it->first << ", " << innerit->first << ", "<< innerit2->first << ", " << innerit2->second << " )" << endl;


            }
        }
    }

}


PPNode::PPNode(int classAtt, int classNumber)
{
    father=NULL;
    nodeStats = new PPNodeStats (classAtt,classNumber);
    isLeaf=true;
    children=NULL;
    this->nodeTest=NULL;

}

int PPNode::depth()
{

    if(this->father == NULL)
        return 0;
    else {

        return this->father->depth()+1;
    }

}


PPNode* PPNode::Classify (PPRecord * r,DTPKC dtpkc)
{

    if (isLeaf)

        return this;

    else{


        if (nodeTest->Eval(r,dtpkc))

            return children->rightChild->Classify(r,dtpkc);

        else

            return children->leftChild->Classify(r,dtpkc);

    }
}

PPNode::PPNode(PPNode *node)
{

    father=NULL;
    nodeStats = new PPNodeStats(node->nodeStats);
    isLeaf=true;
    children=NULL;

}


void PPNode:: addChildren() {

    Children *c;
    c = (Children *) malloc(sizeof(Children));
    c->rightChild = new PPNode(nodeStats->classAtt, nodeStats->classNumber);
    c->leftChild = new PPNode(nodeStats->classAtt, nodeStats->classNumber);
    //this->isLeaf = false;
    c->rightChild->father = this;
    c->leftChild->father = this;
    this->children = c;
}

int PPNode::treeDepth()
{
    int rightDepth=0, leftDepth=0, depth=0;

    if(this->isLeaf)
        return depth;
    else {
        depth ++;
        rightDepth = this->children->rightChild->treeDepth();
        leftDepth = this->children->leftChild->treeDepth();
        if (rightDepth >= depth) depth = rightDepth + 1;
        if (leftDepth >= depth) depth = leftDepth + 1 ;
    }

    return depth;

}

void PPNode :: resetCildren()
{
    try {
        delete this->children->leftChild;
        this->children->leftChild = NULL;
        delete this->children->rightChild;
        this->children->rightChild = NULL;
        free(this->children);
        this->children=NULL;
        delete this->nodeTest;
        this->nodeTest  =NULL;

    } catch (exception e )
    {
        cout << "Trying to delete children" << endl;
        cout << e.what() << endl;
    }


}


int PPNode::numberOfNodes() {

    if (this->isLeaf) {

        return 1;
    }
    else
    {
        return this->children->rightChild->numberOfNodes() + this->children->leftChild->numberOfNodes() + 1 ;

    }

}


void PPNode :: resetNodeStats()
{
    int classAtt =  this->nodeStats->classAtt;
    int classNumber = this->nodeStats->classNumber;
    Cipher classe = this->nodeStats->mostFrequentClass;
    Cipher occ = this->nodeStats->mostFrequentClassOccurrence;
    try {
        delete this->nodeStats;

    } catch (exception e )
    {
        cout << "Trying to delete stats " << endl;
        cout << e.what() << endl;
    }

    this->nodeStats = new PPNodeStats(classAtt,classNumber);
    this->nodeStats->mostFrequentClassOccurrence=occ;
    this->nodeStats->mostFrequentClass=classe;
    //cout <<  this->nodeStats->mostFrequentClassOccurrence << endl;
    //this->dump();
}
