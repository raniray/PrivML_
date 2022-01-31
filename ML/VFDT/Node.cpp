/**
** Class :  Node
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : Represents a node in the decision tree
 *
**/

#include "Node.h"
#include <vector>
#include "NodeStats.h"
#include "NodeTest.h"



Node::Node(int classAtt, int classNumber)
{
    father=NULL;
    nodeStats = new NodeStats (classAtt,classNumber);
    isLeaf=true; // no children and no splitting criterion
    this->children = NULL;
    this->nodeTest=NULL;

}

Node::Node(Node *node)
{

    father=NULL;
    nodeStats = new NodeStats(node->nodeStats);
    isLeaf=true;
    this->children = NULL;


}


Node* Node::Classify (Record * r)
{

    if (isLeaf)

        return this;

    else{

        if (nodeTest->Eval(r))

            return children->rightChild->Classify(r);

        else

            return children->leftChild->Classify(r);

    }
}

void Node:: addChildren() {
    Children *c;
    c = (Children *) malloc(sizeof(Children));
    c->rightChild = new Node(nodeStats->classAtt, nodeStats->classNumber);
    c->leftChild = new Node(nodeStats->classAtt, nodeStats->classNumber);
    //this->isLeaf = false;
    c->rightChild->father = this;
    c->leftChild->father = this;
    this->children = c;
}

int Node::treeDepth()
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

void Node :: resetCildren()
{
    delete children->rightChild;
    delete children->leftChild;
    free(children);
    delete nodeTest;
    this->children = NULL;
}

void Node :: resetNodeStats()
{
    int classAtt =  this->nodeStats->classAtt;
    int classNumber = this->nodeStats->classNumber;
    int classe = this->nodeStats->mostFrequentClass;
    int occ = this->nodeStats->mostFrequentClassOccurrence;
    delete this->nodeStats;
    this->nodeStats = new NodeStats(classAtt,classNumber);
    this->nodeStats->mostFrequentClassOccurrence=occ;
    this->nodeStats->mostFrequentClass=classe;
}


int Node::depth()
{

    if(this->father == NULL)
        return 0;
    else {

        return this->father->depth()+1;
    }

}




void Node::dump() {

    cout << "----------------------dump---------------------" << endl;

    map <int,int> ::iterator i;
    for (i=nodeStats->classOccurrence.begin(); i != nodeStats->classOccurrence.end() ; ++i )
    {
        cout << "C( " << i->first << " ) = " << i->second << endl;
    }

    cout << "Most frequent class " << nodeStats->mostFrequentClass << " occ " << nodeStats->mostFrequentClassOccurrence << endl;
    cout << "Diff class  " << nodeStats->diffClass << endl;
    cout << "ClassAtt " << nodeStats->classAtt << endl;
    cout << "totSampleNum" << nodeStats->totSampleNum << endl;

    cout << "Sum of squares : " << endl;
    map <int,mpz_class> ::iterator j;
    for (j=nodeStats->sumOfSquares.begin(); j != nodeStats->sumOfSquares.end() ; ++j )
    {
        cout << "( " << j->first << "," << j->second << " )" << endl;
    }


    cout << "Sum of values : " << endl;

    for (j=nodeStats->sumOfValues.begin(); j != nodeStats->sumOfValues.end() ; ++j )
    {
        cout << "( " << j->first << "," << j->second << " ) " << endl;
    }

    map <int, map <int, map <int, int>> >:: iterator it;

    for (it=nodeStats->dataAVC.begin(); it != nodeStats->dataAVC.end() ; ++it ) {

        NodeStats::st &innerMap = it->second;
        NodeStats::st::iterator innerit;

        for (innerit = innerMap.begin(); innerit != innerMap.end(); ++innerit) {

            map<int, int> &innerMap2 = innerit->second;
            map<int, int>::iterator innerit2;

            for (innerit2 = innerMap2.begin(); innerit2 != innerMap2.end(); ++innerit2) {


                cout << "( " << it->first << ", " << innerit->first << ", "<< innerit2->first << ", " << innerit2->second << " )" << endl;


            }
        }
    }

}

int Node::numberOfNodes() {

    if (this->isLeaf) {

        return 1;
    }
    else
    {
        return this->children->rightChild->numberOfNodes() + this->children->leftChild->numberOfNodes() + 1 ;

    }

}

