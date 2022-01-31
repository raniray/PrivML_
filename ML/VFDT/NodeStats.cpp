/**
** Class :  NodeStats
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : The class that contains the stats a given node : these stats are used
 * to compute the splitting criterion and to achieve the classification
**/

#include "NodeStats.h"
#include <map>
#include "math.h"

NodeStats:: NodeStats(int classAtt, int classNumber)
{
    this->classAtt=classAtt;
    this->classNumber=classNumber;
    for (int i=0;i<this->classAtt+1;i++)
    {
        this->sumOfValues.insert(std::make_pair(i,0));
        this->sumOfSquares.insert(std::make_pair(i,0));
    }
    this->mostFrequentClass=-1;
    this->mostFrequentClassOccurrence=-1;
    this->diffClass=false;
    this->totSampleNum=0;
}

NodeStats:: NodeStats(NodeStats * nodeStats)
{
    classOccurrence = nodeStats->classOccurrence;
    mostFrequentClass = nodeStats->mostFrequentClass;
    diffClass = nodeStats->diffClass;
    mostFrequentClassOccurrence = nodeStats->mostFrequentClassOccurrence;
    classAtt = nodeStats->classAtt;
    totSampleNum = nodeStats->totSampleNum;
    classNumber = nodeStats->classNumber;
    sumOfSquares = nodeStats->sumOfSquares;
    sumOfValues = nodeStats ->sumOfValues;
    dataAVC = nodeStats->dataAVC;
}


void NodeStats:: UpdateNodeStats(Record * r)
{
    this->totSampleNum++;
    this->incrementClassOccurence(r);

}


void NodeStats::updateDataAVC (Record * r, int i )
{

    map <int, map <int, map <int, int>> >:: iterator it = dataAVC.find(i);

    if( it != dataAVC.end() ) {

        st &innerMap = it->second;
        st::iterator innerit = innerMap.find( r->values[i] );

        if( innerit != innerMap.end() )
        {
            map<int,int> &innerInnerMap = innerit->second;
            map<int,int> :: iterator classOcc = innerInnerMap.find( r->values[classAtt] );
            if ( classOcc != innerInnerMap.end())
            {
                classOcc->second++;

            }else{
                innerInnerMap.insert(std::make_pair(r->values[classAtt],1));
            }
        }else{

            map<int,int> newClassOcc;
            newClassOcc.insert(std::make_pair(r->values[classAtt],1));
            innerMap.insert(std::make_pair(r->values[i],newClassOcc));
        }

    } else {

        map<int,int> newClassOcc;
        newClassOcc.insert(std::make_pair(r->values[classAtt],1));
        st newAttMap;
        newAttMap.insert(std::make_pair(r->values[i],newClassOcc));
        dataAVC.insert(std::make_pair(i,newAttMap));

    }
}

void NodeStats::incrementClassOccurence(Record * r )
{
    std::map<int, int>::iterator iter = this->classOccurrence.find(r->values[classAtt]);
    if (iter != this->classOccurrence.end())
        iter->second = iter->second + 1;
    else
        this->classOccurrence.insert(std::make_pair(r->values[classAtt],1));

    if(this->mostFrequentClass!= r->values[classAtt]) diffClass= true;

    for(int i=0; i< r->values.size(); i++) {
        std::map<int, mpz_class>::iterator iter2;
        iter2 = this->sumOfSquares.find(i);
        if (iter2 != this->sumOfSquares.end())
            iter2->second = iter2->second + mpz_class_powm (r->values[i],2,100000000000000000);

        iter2 = this->sumOfValues.find(i);
        if (iter2 != this->sumOfValues.end())
            iter2->second = iter2->second + r->values[i];

        this->updateDataAVC(r, i);
    }
}

void  NodeStats::computeMostFrequentClass() {

    map <int,int> ::iterator i;
    for (i=this->classOccurrence.begin(); i != this->classOccurrence.end() ; ++i )
    {
        if ( i->second > this->mostFrequentClassOccurrence)
        {
            this->mostFrequentClass=i->first;
            this->mostFrequentClassOccurrence= i->second;

        }
    }


}

