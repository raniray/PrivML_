/**
** Class :  Node
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : Represents a node in the decision tree
 *
**/

#include "../IO/Record.h"
#include "NodeStats.h"
#include "NodeTest.h"

#ifndef DAPPLE_NODE_H
#define DAPPLE_NODE_H


class Node {

    typedef struct Children {
        Node* rightChild;
        Node* leftChild;

    } Children ;

public :

    Children * children;
    Node * father;
    NodeStats * nodeStats;
    NodeTest * nodeTest;
    bool isLeaf;


    /******* Functions **********/

public :
    Node(int classAtt, int classNumber);

public :
    Node(Node *node);

public:
    Node * Classify (Record * r);

public :
    void addChildren();


public :
    int treeDepth();

public :
    void resetCildren();

public :
    void resetNodeStats();

public :
    void dump();

public :
    int depth();

public :
    int numberOfNodes();


public :
    ~Node(){

        try {

            if (this->children != NULL) {

                delete children->rightChild;
                delete children->leftChild;
                free(children);

            }

            if (!isLeaf)
                delete nodeTest;



            delete nodeStats;

        }catch (exception e )
        {
            cout << "Trying to destruct node " << endl;
            cout << e.what() << endl;
        }

    }


};


#endif //DAPPLE_NODE_H
