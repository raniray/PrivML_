/**
** Class :  PPNodeStats
 * Authors : Talbi Rania
** Creation date : 16/11/2018
** Last Update : 16/11/2018
** Purpose : The class that contains the stats a given node : these stats are used
 * to compute the splitting criterion and to achieve the classification
**/

#include "PPNodeStats.h"
#include <map>
#include "math.h"
#include "omp.h"
#include "PPVFDT_Classifier.h"


PPNodeStats:: PPNodeStats(int classAtt, int classNumber)
{
    this->classAtt=classAtt;
    this->classNumber=classNumber;
    this->one=dtpkc.enc(1,dtpkc.pkw);
    this->zero=dtpkc.enc(0,dtpkc.pkw);
    for (int i=0;i<classAtt+1;i++)
    {
        this->sumOfValues.insert(std::make_pair(i,zero));
        this->sumOfSquares.insert(std::make_pair(i,zero));
    }
    this->mostFrequentClass= zero;

    this->mostFrequentClassOccurrence=zero;
    this->diffClass=false;
    this->totSampleNum= zero;

}



PPNodeStats:: PPNodeStats(PPNodeStats * nodeStats) {

    classOccurrence = nodeStats->classOccurrence;
    mostFrequentClass = nodeStats->mostFrequentClass;
    diffClass = nodeStats->diffClass;
    mostFrequentClassOccurrence = nodeStats->mostFrequentClassOccurrence;
    classAtt = nodeStats->classAtt;
    totSampleNum = nodeStats->totSampleNum;
    classNumber = nodeStats->classNumber;
    sumOfSquares = nodeStats->sumOfSquares;
    sumOfValues = nodeStats->sumOfValues;
    dataAVC = nodeStats->dataAVC;

}

void PPNodeStats:: UpdateNodeStats(PPRecord * r)
{

    EvalAdd evalAdd (this->totSampleNum,one);
    this->totSampleNum=evalAdd.EvalAdd_U1();
    this->incrementClassOccurence(r);




}



void PPNodeStats::updateDataAVC (PPRecord * r, int i )
{

    map <int, map <Cipher, map <Cipher, Cipher>> >:: iterator it = dataAVC.find(i);

    if( it != dataAVC.end() ) {

        Stats &innerMap = it->second;
        Stats::iterator innerit = innerMap.find( r->values[i] );

        if( innerit != innerMap.end() )
        {
            map<Cipher,Cipher> &innerInnerMap = innerit->second;
            map<Cipher,Cipher> :: iterator classOcc = innerInnerMap.find( r->values[classAtt] );

            if ( classOcc != innerInnerMap.end())
            {
                EvalAdd addClassOcc( classOcc->second, one);
                classOcc->second=addClassOcc.EvalAdd_U1();


            }else{
                innerInnerMap.insert(std::make_pair(r->values[classAtt], one));

            }
        }else{

            map<Cipher,Cipher> newClassOcc;
            newClassOcc.insert(std::make_pair(r->values[classAtt], one));
            innerMap.insert(std::make_pair(r->values[i],newClassOcc));
        }

    } else {

        map<Cipher,Cipher> newClassOcc;
        newClassOcc.insert(std::make_pair(r->values[classAtt], one));
        Stats newAttMap;
        newAttMap.insert(std::make_pair(r->values[i],newClassOcc));
        dataAVC.insert(std::make_pair(i,newAttMap));

    }

}

void PPNodeStats::incrementClassOccurence(PPRecord * r )
{

    vector <int> bdd(r->values.size());


    /**Time**/

    auto begin = chrono::high_resolution_clock::now();
    std::map<Cipher, Cipher>::iterator iter = this->classOccurrence.find(r->values[classAtt]);
    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end  - begin ;
    PPVFDT_Classifier::cmpTime+= duration.count();
    PPVFDT_Classifier::cmpCtr += this->classOccurrence.size();

    if (iter != this->classOccurrence.end())
    {

        EvalAdd addIter ( iter->second,one);
        iter->second= addIter.EvalAdd_U1();
    }
    else
        this->classOccurrence.insert(std::make_pair(r->values[classAtt], one));


    if(this->mostFrequentClass != r->values[classAtt])
        diffClass= true;

    begin = chrono::high_resolution_clock::now();

   #pragma omp parallel for schedule(dynamic)
    for(int i=0; i< r->values.size(); i++) {
        std::map<int, Cipher>::iterator iter2;
        iter2 = this->sumOfSquares.find(i);

        if (iter2 != this->sumOfSquares.end())
        {
            EvalAddPow2 evalAddPow2(iter2->second,r->values[i],dtpkc);
            iter2->second = evalAddPow2.EvalAddPow2_U1_step3(evalAddPow2.EvalAddPow2_U2_step2(evalAddPow2.EvalAddPow2_U1_step1()));
        }


        iter2 = this->sumOfValues.find(i);
        if (iter2 != this->sumOfValues.end())
        {
            EvalAdd addIter2 (iter2->second, r->values[i]);
            iter2->second=addIter2.EvalAdd_U1();


        }

        this->updateDataAVC(r, i);
    }


    end = chrono::high_resolution_clock::now();
    duration = end  - begin ;
    PPVFDT_Classifier::cmpTime+= (1 - (1/(r->values.size()* this->classNumber  +1 )))*duration.count();
    PPVFDT_Classifier::addPowTime+= ((1/(r->values.size()* this->classNumber + 1)))*duration.count();
    PPVFDT_Classifier::cmpCtr+= r->values.size()* this->classNumber + r->values.size();
    PPVFDT_Classifier::addPowCtr+=  r->values.size() ;


}


void  PPNodeStats::computeMostFrequentClass() {

    map <Cipher,Cipher> ::iterator i;
    auto begin = chrono::high_resolution_clock::now();
    for (i=this->classOccurrence.begin(); i != this->classOccurrence.end() ; ++i )
    {
        if ( this->mostFrequentClassOccurrence < i->second)
        {
            this->mostFrequentClass=i->first;
            this->mostFrequentClassOccurrence= i->second;
        }
    }
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    PPVFDT_Classifier::cmpTime+= duration.count();
    PPVFDT_Classifier::cmpCtr += this->classOccurrence.size();

}



