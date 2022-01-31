/**
** Class :  PPNode
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : Represents a node in the decision tree
 *
**/

#include "../IO/PPRecord.h"
#include "PPNodeStats.h"
#include "PPNodeTest.h"

#ifndef DAPPLE_PPNODE_H
#define DAPPLE_PPNODE_H

class PPNode {

    typedef struct Children {
        PPNode* rightChild;
        PPNode* leftChild;

    } Children ;

public :
    Children * children;
    PPNode * father;
    PPNodeStats * nodeStats;
    PPNodeTest * nodeTest;
    bool isLeaf;


    /******* Functions **********/

public :
    PPNode(int classAtt, int classNumber);


public:
    PPNode * Classify (PPRecord * r, DTPKC dtpkc);


public :
    void addChildren();

public :
    int treeDepth();

public :
    void resetCildren();

public :
    void resetNodeStats();

public:
    PPNode(PPNode *node);

public:
    int depth();

public:
    void dump();

public:
    int numberOfNodes();

public :
    ~PPNode(){

        try {

            if (children != NULL) {

                delete children->rightChild;
                children->rightChild=NULL;
                delete children->leftChild;
                children->leftChild=NULL;
                free(children);
                children=NULL;

            }
            if (!isLeaf)  delete nodeTest;

            delete nodeStats;
            nodeStats = NULL;

        }catch (exception e )
        {
            cout << "Trying to destruct node " << endl;
            cout << e.what() << endl;
        }

    }


};


#endif
