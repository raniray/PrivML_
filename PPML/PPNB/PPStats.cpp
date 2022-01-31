#include "PPStats.h"
#include <map>
#include "math.h"
#include "PPNB_Classifier.h"

void PPStats::h()
{

    /*Cipher res;
    res.T1= one.T1 * one.T1;
    res.T2= one.T2 * one.T2;

    res.T1= one.T1 * one.T1;
    res.T2= one.T2 * one.T2;*/
}

DTPKC PPStats::dtpkc;
Cipher PPStats::one;
Cipher PPStats::zero;

PPStats::PPStats(int classNumber, int classAtt ) {

    this->classNumber=classNumber;
    this->classAtt = classAtt;
    this->one=dtpkc.enc(1,dtpkc.pkw);
    this->zero=dtpkc.enc(0,dtpkc.pkw);
    this->totSampleNum=zero;
}


void
PPStats::updateDataAVC(PPRecord *r, int i) {

    map <int, map <Cipher, map <Cipher, Cipher>> >:: iterator it = dataAVC.find(i);

    if( it != dataAVC.end() ) {

        PPStats::st &innerMap = it->second;
        PPStats::st::iterator innerit = innerMap.find( r->values[i] );

        if( innerit != innerMap.end() )
        {
            map<Cipher,Cipher> &innerInnerMap = innerit->second;
            map<Cipher,Cipher> :: iterator classOcc = innerInnerMap.find( r->values[classAtt] );
            if ( classOcc != innerInnerMap.end())
            {
                EvalAdd evalAdd (classOcc->second,one);
                classOcc->second=evalAdd.EvalAdd_U1();
                //PPNB_Classifier::addCtr++;

            }else{

                h();
                innerInnerMap.insert(std::make_pair(r->values[classAtt],one));
            }
        }else{

            h();
            map<Cipher,Cipher> newClassOcc;
            newClassOcc.insert(std::make_pair(r->values[classAtt],one));
            innerMap.insert(std::make_pair(r->values[i],newClassOcc));
        }

    } else {

        h();
        map<Cipher,Cipher> newClassOcc;
        newClassOcc.insert(std::make_pair(r->values[classAtt],one));
        PPStats::st newAttMap;
        newAttMap.insert(std::make_pair(r->values[i],newClassOcc));
        dataAVC.insert(std::make_pair(i,newAttMap));

    }

}

void
PPStats::incrementClassOccurence(PPRecord *r) {

    auto begin = chrono::high_resolution_clock::now();
    std::map<Cipher, Cipher>::iterator iter = this->classOccurrence.find(r->values[classAtt]);
    auto end = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end  - begin ;
    PPNB_Classifier::cmpTime += duration.count();
    PPNB_Classifier::cmpCtr += this->classOccurrence.size();

    if (iter != this->classOccurrence.end())
    {

        EvalAdd evalAdd (iter->second ,one);
        iter->second =evalAdd.EvalAdd_U1();

    }
    else
    {
        h();
        this->classOccurrence.insert(std::make_pair(r->values[classAtt],one));
    }

    begin = chrono::high_resolution_clock::now();
#pragma omp parallel for
    for(int i=0; i< r->values.size() - 1; i++)
    {
        this->updateDataAVC(r, i);
    }

   end = chrono::high_resolution_clock::now();
   duration = end  - begin ;
   PPNB_Classifier::cmpTime += duration.count();
   PPNB_Classifier::cmpCtr +=r->values.size() * r->values.size()/2 * this->classNumber;

}

void
PPStats::UpdateStatistics(PPRecord *r) {


    EvalAdd evalAdd (this->totSampleNum ,one);
    this->totSampleNum =evalAdd.EvalAdd_U1();
    this->incrementClassOccurence(r);

}

void
PPStats::InitStats() {

    std::map <Cipher, Cipher> proProba;
    std::map <Cipher, Cipher> LogproProba;
    double maxDuration = 0.0;
#pragma omp parallel
    {
        size_t cnt = 0;
        int ithread = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        for (auto i = classOccurrence.begin(); i!=classOccurrence.end(); ++i, cnt++) {
            if(cnt%nthreads != ithread) continue;
            auto begin = chrono::high_resolution_clock::now();
            EvalDiv100 evalDiv100(i->second, totSampleNum, dtpkc);
            vector<Cipher> res1 = evalDiv100.EvalDiv_U1_step1();
            Cipher proba = evalDiv100.EvalDiv_U1_step3(evalDiv100.EvalDiv_U2_step2(res1[0], res1[1], res1[2], res1[3]));
            auto end = chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end  - begin ;
            double tdiv_local= duration.count();
#pragma omp critical
            {

                proProba.insert(std::make_pair(i->first, proba));
                auto begin = chrono::high_resolution_clock::now();
                EvalLog evalLog(proba,10000,dtpkc);
                std::vector<Cipher> in = evalLog.EvalLog_U1_step1();
                Cipher C_C=evalLog.EvalLog_U1_step3(evalLog.EvalLog_U2_step2(in[0],in[1]));
                auto end = chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end  - begin ;
                PPNB_Classifier::logTime += duration.count();
                if (tdiv_local > maxDuration) maxDuration = tdiv_local;
                PPNB_Classifier::trainBdw += evalDiv100.bdwMU + evalDiv100.bdwSU + evalLog.bdwMU + evalLog.bdwSU;
                LogproProba.insert(std::make_pair(i->first, C_C));


            };
        }

    }
    this->priorProba=proProba;
    this->LogpriorProba=LogproProba;


    PPNB_Classifier::divTime += maxDuration;
    PPNB_Classifier::divCtr++;
    PPNB_Classifier::logCtr++;


    //std :: map <int, stf > condProba;
    std :: map <int, stf > LogcondProba;
#pragma omp parallel
    {
        size_t cnt = 0;
        int ithread = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        double costTimeDiv = 0.0;
        double costTimeDivSU = 0.0;
        double costBdwDiv =0.0;
        double costTimeLog= 0.0;
        double costTimeLogSU= 0.0;
        double costBdwLog =0.0;
        double tdiv_local =0.0;
        for (auto j = dataAVC.begin(); j!=dataAVC.end(); ++j, cnt++) {
            if(cnt%nthreads != ithread) continue;
            PPStats::st VC = j->second;

            //PPStats::stf cdProbaValues;
            PPStats::stf LogcdProbaValues;

            for (auto &k : VC) {


                map<Cipher, Cipher> classFreq = k.second;
                //map<Cipher, Cipher> cdProbaClasses;
                map<Cipher, Cipher> LogcdProbaClasses;
                // Parse classes
                for (auto &l : classFreq) {
                    Cipher probaxjkl = zero;
                    map<Cipher, Cipher>::iterator i = this->classOccurrence.find(l.first);
                    if (i != this->classOccurrence.end()) {
                        EvalDiv100 evalDiv100(l.second, i->second, dtpkc);
                        auto begin = chrono::high_resolution_clock::now();
                        vector<Cipher> res1 = evalDiv100.EvalDiv_U1_step1();
                        probaxjkl = evalDiv100.EvalDiv_U1_step3(
                                evalDiv100.EvalDiv_U2_step2(res1[0], res1[1], res1[2], res1[3]));
                        auto end = chrono::high_resolution_clock::now();
                        std::chrono::duration<double, std::milli> duration = end  - begin ;
                        tdiv_local= duration.count();
                        //costTimeDiv = evalDiv100.timeMU;
                        //costTimeDivSU = evalDiv100.timeSU;
                        costBdwDiv = evalDiv100.bdwMU + evalDiv100.bdwSU;

                    }
                    //cdProbaClasses.insert(std::make_pair(l.first, probaxjkl));
                    auto begin = chrono::high_resolution_clock::now();
                    EvalLog evalLog(probaxjkl,10000,dtpkc);
                    std::vector<Cipher> in = evalLog.EvalLog_U1_step1();
                    Cipher C_C=evalLog.EvalLog_U1_step3(evalLog.EvalLog_U2_step2(in[0],in[1]));
                    auto end = chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> duration = end  - begin ;
                    PPNB_Classifier::logTime += duration.count();

                    LogcdProbaClasses.insert(std::make_pair(l.first, C_C));
                    //COUT << "cond proba " << C_C << " res " <<mpz_class_log (dtpkc.Sdec(probaxjkl) + 1 , 100) << endl;
                    //costTimeLog = evalLog.timeMU;
                    //costTimeLogSU = evalLog.timeSU;
                    //costBdwLog = evalLog.bdwMU + evalLog.bdwSU;
                }
                //cdProbaValues.insert(std::make_pair(k.first, cdProbaClasses));
                LogcdProbaValues.insert(std::make_pair(k.first, LogcdProbaClasses));
            }

#pragma omp critical
            {
                int size = 0;
                for (auto &ptr1 : VC) {

                        size += ptr1.second.size();

                }
                //condProba.insert(std::make_pair(j->first, cdProbaValues));
                LogcondProba.insert(std::make_pair(j->first, LogcdProbaValues));
                //PPNB_Classifier::trainTimeMU += size*(costTimeLog + costTimeDiv);
                //PPNB_Classifier::trainTimeSU += size*(costTimeLogSU + costTimeDivSU);
                PPNB_Classifier::divCtr+=size;
                PPNB_Classifier::logCtr+=size;
                PPNB_Classifier::divTime += tdiv_local;
                PPNB_Classifier::trainBdw += size*(costBdwLog + costBdwDiv);
            };

        }

    }
    //this->conditionalProba=condProba;
    this->LogconditionalProba=LogcondProba;
}


Cipher PPStats::Classify(PPRecord *r) {



    map<Cipher,Cipher> proba;

//#pragma omp parallel
    {
        size_t cnt = 0;
        int ithread = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        //cout << "*----------------------------------------------*" <<  endl;
        for (auto i = this->LogpriorProba.begin(); i != this->LogpriorProba.end(); ++i, cnt++) {
           // if (cnt % nthreads != ithread) continue;

            Cipher finalProbaCi2 = i->second;

            //cout << " prior proba " << i->second << " Value of i " << i->first << endl;

            for (auto &j : this->LogconditionalProba) {

                //cout << " attribut " <<  j.first << endl;

                //r->print();
                auto begin = chrono::high_resolution_clock::now();
                map<Cipher, map<Cipher, Cipher>> innerMap = j.second;
                map<Cipher, map<Cipher, Cipher >>::iterator probaValues = innerMap.find(r->values[j.first]);
                auto en = chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = en  - begin ;
                PPNB_Classifier::cmpTime += duration.count() ;
                PPNB_Classifier::cmpCtr+=r->values.size();

                if (probaValues != innerMap.end()) {
                    auto begin = chrono::high_resolution_clock::now();
                    map<Cipher, Cipher>::iterator probaClasses = probaValues->second.find(i->first);
                    auto en = chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> duration = en  - begin ;
                    PPNB_Classifier::cmpTime += duration.count() ;
                    PPNB_Classifier::cmpCtr+=probaValues->second.size();
                    if (probaClasses != probaValues->second.end()) {

                        //cout << " pobaClasses->second.T1 " << probaClasses->second << endl;

                        auto begin = chrono::high_resolution_clock::now();
                        finalProbaCi2.T1 = finalProbaCi2.T1 *  probaClasses->second.T1;
                        finalProbaCi2.T2 = finalProbaCi2.T2 *  probaClasses->second.T2;
                        auto en = chrono::high_resolution_clock::now();
                        std::chrono::duration<double, std::milli> duration = en  - begin ;
                        PPNB_Classifier::addTime += duration.count() ;




                    } else {
                        h();
                        finalProbaCi2 = zero;
                        //break;
                    }
                } else {
                    h();
                    finalProbaCi2 = zero;
                    //break;
                }

            }
#pragma omp critical
            {proba.insert(std::make_pair(i->first, finalProbaCi2));
            PPNB_Classifier::addCtr++;}
        }
    }

    Cipher maxProba =zero;
    Cipher maxClass = zero;
    auto begin = chrono::high_resolution_clock::now();
    for (auto & i : proba )
    {

        if (i.second > maxProba )
        {
            maxProba = i.second;
            maxClass = i.first;
            PPNB_Classifier::cmpCtr++;
        }

    }
    auto en = chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli>  duration = en  - begin ;
    PPNB_Classifier::cmpTime += duration.count() ;


    //COUT << " maxProba " <<  maxProba << " maxClass " << maxClass << endl;

    return maxClass;
}

void PPStats::print ()
{
    for ( auto & i : dataAVC )
    {

        for ( auto & j : i.second )
        {
            for ( auto & k : j.second )
            {
                cout << "(A = " << i.first << ", a = " << dtpkc.Sdec (j.first) << ", c = " << dtpkc.Sdec (k.first)<< ", occ = " <<  dtpkc.Sdec (k.second) << " ) " <<  endl;
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
                cout << "(A = " << i.first << ", a = " << dtpkc.Sdec (j.first) << ", c = " << dtpkc.Sdec (k.first)<< ", proba = " <<  dtpkc.Sdec (k.second) << " ) " <<  endl;
            }

        }


    }


}
void PPStats::UpdateClassifier(PPRecord *r) {


    map <Cipher,Cipher> ::iterator i = this->classOccurrence.find(r->values[classAtt]);
    if (i != this->classOccurrence.end())
    {
        EvalAdd evalAdd (i->second,one);
        i->second =evalAdd.EvalAdd_U1();

    }
    else
    {
        this->classOccurrence.insert(std::make_pair(r->values[classAtt],one));
    }

    EvalAdd evalAdd (this->totSampleNum,one);
    this->totSampleNum=evalAdd.EvalAdd_U1();

    //#pragma omp parallel
    //{
    /*size_t cnt = 0;
    int ithread = omp_get_thread_num();
    int nthreads = omp_get_num_threads();*/

    //for (auto j = priorProba.begin(); j != priorProba.end(); ++j, cnt++) {
    for (auto &j : priorProba) {
        //if (cnt % nthreads != ithread) continue;

        map<Cipher, Cipher>::iterator s = this->classOccurrence.find(j.first);
        if (s != classOccurrence.end()) {
            EvalDiv100 evalDiv100(s->second, totSampleNum, dtpkc);
            vector<Cipher> res1 = evalDiv100.EvalDiv_U1_step1();
            j.second = evalDiv100.EvalDiv_U1_step3(evalDiv100.EvalDiv_U2_step2(res1[0], res1[1], res1[2], res1[3]));
        }


    }
    //}

#pragma omp parallel for
    for (int b =0; b < r->values.size() - 1; ++b)
    {
        updateDataAVC(r,b);
    }


    for (auto &k : conditionalProba) {
        for (auto &l : k.second) {
            map<Cipher, Cipher>::iterator m = l.second.find(r->values[classAtt]);
            //print();
            if (m != l.second.end()) {
                Cipher tmp;
                map<Cipher, Cipher> tm = dataAVC.find(k.first)->second.find(l.first)->second;
                map<Cipher, Cipher>::iterator tmpIt = tm.find(r->values[classAtt]);
                if (tmpIt != tm.end()) {
                    tmp = tmpIt->second;
                    EvalDiv100 evalDiv100(tmp, i->second, dtpkc);
                    vector<Cipher> res1 = evalDiv100.EvalDiv_U1_step1();
                    m->second = evalDiv100.EvalDiv_U1_step3(
                            evalDiv100.EvalDiv_U2_step2(res1[0], res1[1], res1[2], res1[3]));
                } else {

                    m->second = zero;
                }


            } else {
                l.second.insert(std::make_pair(r->values[classAtt], zero));
            }

        }
    }
    //}


}
