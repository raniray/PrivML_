#include "Stats.h"
#include <map>
#include "math.h"


bool Stats::initialised;

Stats::Stats(int classNumber, int classAtt ) {
    this->classNumber=classNumber;
    this->classAtt = classAtt;
    this->totSampleNum=0;
}


void
Stats::updateDataAVC(Record *r, int i) {

    map <int, map <int, map <int, int>> >:: iterator it = dataAVC.find(i);

    if( it != dataAVC.end() ) {

        Stats::st &innerMap = it->second;
        Stats::st::iterator innerit = innerMap.find( r->values[i] );

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
        Stats::st newAttMap;
        newAttMap.insert(std::make_pair(r->values[i],newClassOcc));
        dataAVC.insert(std::make_pair(i,newAttMap));

    }

}

void
Stats::incrementClassOccurence(Record *r) {

    std::map<int, int>::iterator iter = this->classOccurrence.find(r->values[classAtt]);
    if (iter != this->classOccurrence.end())
        iter->second = iter->second + 1;
    else
        this->classOccurrence.insert(std::make_pair(r->values[classAtt],1));

    for(int i=0; i< r->values.size() - 1; i++)
        this->updateDataAVC(r, i);

}

void
Stats::UpdateStatistics(Record *r) {

    this->totSampleNum++;
    this->incrementClassOccurence(r);

}

void
Stats::InitStats(int laplace) {


    std::map <int, float> proProba;
    std :: map <int, stf > condProba;
    std::map <int, float> LogproProba;
    std :: map <int, stf > LogcondProba;

    for (auto & i : classOccurrence ) {
        float proba = (1.0 * i.second) / (1.0*totSampleNum );
        proProba.insert(std::make_pair(i.first, proba));
        LogproProba.insert(std::make_pair(i.first, log(1.0+proba)));
        cout << "i.second " << i.second << " total sample num  " << totSampleNum << " proba " << proba << " log(1.0+proba) " << log(1.0+proba) << endl;
    }

    this->priorProba = proProba;
    this->LogpriorProba = LogproProba;
        // update conditional probability

        // Parse attributes

         for (auto & j : dataAVC )
         {
            Stats::st VC = j.second;

            // Parse values
            Stats::stf cdProbaValues;
            Stats::stf LogcdProbaValues;

            for ( auto & k : VC) {


                map <int, int> classFreq = k.second;
                map <int, float> cdProbaClasses;
                map <int, float> LogcdProbaClasses;
                // Parse classes
                for (auto & l : classFreq) {
                    float probaxjkl =0.0;
                     map <int, int> :: iterator i = this->classOccurrence.find(l.first);
                    if (i != this->classOccurrence.end())
                        probaxjkl = (1.0 *l.second)/(1.0 *i->second);
                    cdProbaClasses.insert(std::make_pair(l.first,probaxjkl));
                    LogcdProbaClasses.insert(std::make_pair(l.first,log(laplace+probaxjkl)));
                    //cout << "Cond proba " << log(laplace+probaxjkl) << endl;
                }
                cdProbaValues.insert(std::make_pair(k.first, cdProbaClasses));
                LogcdProbaValues.insert(std::make_pair(k.first, LogcdProbaClasses));
            }

             condProba.insert(std::make_pair(j.first, cdProbaValues));
             LogcondProba.insert(std::make_pair(j.first, LogcdProbaValues));


        }

        this->conditionalProba = condProba;
        this->LogconditionalProba = LogcondProba;

}

int Stats::Classify(Record *r) {

    map<int,float> proba;


    //cout << "*----------------------------------------------*" <<  endl;
    for ( auto & i : LogpriorProba ) {
        //cout << " prior proba " << i.second << " Value of i " << i.first << endl;
        float  finalProbaCi = i.second;
        for ( auto & j : LogconditionalProba)
        {
            map<int, map <int, float>> innerMap =  j.second;
            map<int, map <int, float>> :: iterator probaValues = innerMap.find(r->values[j.first]);

            //cout << " attribut " <<  j.first << endl;

            if (probaValues != innerMap.end())
            {
                map<int, float>::iterator probaClasses = probaValues->second.find(i.first);
                if (probaClasses != probaValues->second.end())
                {
                    finalProbaCi += 1.0* probaClasses->second;
                    //cout << " pobaClasses->second.T1 " << probaClasses->second << endl;
                }
                else
                    finalProbaCi = 0.0;
            }
            else
            finalProbaCi = 0.0;
        }
        proba.insert(std::make_pair(i.first,finalProbaCi));
    }
    //cout << "*----------------------------------------------*" <<  endl;


    float maxProba = -100000;
    int maxClass = 0;

    //cout << "----------------------------------------------" <<  endl;
    for (auto & i : proba )
    {
        cout  << "i " << i.first <<  " proba "  << i.second << endl;

        if (i.second > maxProba )
        {
            maxProba = i.second;
            maxClass = i.first;
        }

    }
    //cout << "----------------------------------------------" <<  endl;
    return maxClass;
}



void Stats::print ()
{
    for ( auto & i : dataAVC )
    {

        for ( auto & j : i.second )
        {
            for ( auto & k : j.second )
            {
                cout << "(A = " << i.first << ", a = " << j.first << ", c = " << k.first<< ", occ = " <<  k.second << " ) " <<  endl;
            }

        }


    }

    cout << "$$$$$$$$$$$$" << endl;


    for ( auto & i : classOccurrence )
    {
        cout << "(C = " << i.first << ", occ = " << i.second << " ) " <<  endl;

    }

    cout << "$$$$$$$$$$$$" << endl;



    for ( auto & i : LogpriorProba )
    {
        cout << "(C = " << i.first << ", Proba = " << i.second << " ) " <<  endl;

    }

    cout << "$$$$$$$$$$$$" << endl;

    for ( auto & i : LogconditionalProba )
    {

        for ( auto & j : i.second )
        {
            for ( auto & k : j.second )
            {
                cout << "(A = " << i.first << ", a = " << j.first << ", c = " << k.first<< ", proba = " << k.second<< " ) " <<  endl;
            }

        }


    }


}
