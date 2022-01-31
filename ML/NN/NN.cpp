#include <iostream>
#include <map>
#include "omp.h"
#include "NN.h"



NN::NN(double alpha, int epochs, int batchSize, float th, DatasetReader *dt, string logfile,
       bool debug, string mainpath) {


    trainTime =0.0;
    testTime =0.0;
    extTrainBd =0;
    extTestBd = 0;
    clientTestTime =0.0;
    this->debug =debug;
    this->alpha = alpha;
    this->batchSize=batchSize;
    this->th = th;
    this->dt = dt;
    this->logfile=logfile;
    this->epochs=epochs;
    this->mainpath = mainpath;


    network.resize(3);
    // initialize the weights of the newtork

    for(int i=1; i < network.size()+1; i++)
    {

        for (int j=0; j < network_dimensions[i]; j++)
        {
            vector<float> weights;
            for (int k=0; k < network_dimensions[i-1]+1; k++)
                weights.push_back(0.0); // je compte le biais

            neuron *n = new neuron(weights, alpha,epochs, batchSize, th, dt,debug);
            network[i-1].push_back(n);

        }
    }

    if (debug )
    {
        for (int i=0; i< network_dimensions.size(); i++)
            cout << "dimension["<<i<<"]= " << network_dimensions[i] <<endl;

        cout << "alpha " << alpha << endl;
        cout << "batchSize " << batchSize<< endl;
        cout << "th " << th << endl;
        cout << "logfile " << logfile << endl;
        cout << ""<< endl;
    }

}


vector<vector<float>> NN::forward_layer(vector<neuron*> layer, vector<vector<float>> x, bool test, bool first ){


    vector<vector<float>> res;
    if(!first)
    {
        for (int i=0; i<x.size(); i++)
        {
           x[i].insert(x[i].begin(), 1);
        }
    }


    for (int j=0; j < layer.size(); j++)
    {
        neuron *n = layer[j];

        res.push_back(n->predict_batch(x,test));
    }

    //todo : add an additional step to invert: batch_layes


    vector<vector<float>> res_final(x.size(), vector<float > (layer.size(),0.0));


    for (int i=0; i<x.size(); i++)
    {

        for (int k=0; k< layer.size(); k++)
        {
            float e = res[k][i];
            res_final[i][k] = e;

        }
    }

    return  res_final;

}


vector<int> NN::predict(vector<Record *>R, bool test ) {

    // todo: edit so that the final output is only a class label and make sure itrs the same thing as ytrue
    bool first = true;
    vector<vector<float>> XB;
    for (int i=0; i < R.size(); i++)
    {
        Record *r = R[i];
        vector<float> x = vector<float> (r->values.begin(), r->values.end());
        XB.push_back(x);
    }

    for (int i=0; i < network.size(); i++)
    {

        XB = forward_layer(network[i], XB, test, first);
        first =  false;
    }


    vector<int> res;

    for (int j=0; j < XB.size(); j++)
    {

        vector<float> x  = XB[j];
        float max = -1.0;
        int argmax =0;
            for (int k=0; k < x.size(); k++) {

                if (x[k]>max)
                {
                    max = x[k];
                    argmax = k;
                }
            }
            res.push_back(argmax);
    }



    return res;
}



vector<vector<float>> NN::backpropagate_layer(vector<neuron*> layer, vector<vector<float>> ytrue) {

    vector<vector<float>> new_output_layer;
    for(int i=0; i < ytrue.size(); i++)
    {
        vector<vector<float>> XB = layer[i]->previous_input;
        layer[i]->train(XB, ytrue[i]);
        vector <float > new_output_neuron = layer[i]->new_output;
        new_output_layer.push_back(new_output_neuron);
    }
    return new_output_layer;

}



void NN::backpropagate(vector<Record *> XB){

    vector<int> prediction = predict(XB, false);
    vector<vector<float>> R;
    vector<vector<float>> ytrue(2, vector<float>(XB.size(),0));
    int dim = XB[0]->values.size()-1;
    int numberClasses = 2;

        for(int i=0; i<XB.size(); i++)
        {
            //todo: try to understand why is there a double amount of values per class in ytrue
            vector<float> r =  vector<float> (XB[i]->values.begin(), XB[i]->values.end());
            r.pop_back();
            R.push_back(r);
            std::vector<float> hot_label(2); // hard coded the number of classes
            for (int s=0; s<numberClasses; s++) {
                    if(s!= XB[i]->values[dim])
                        ytrue[s].push_back(0);
                    else
                        ytrue[s].push_back(1);
            }

        }






    for(int j= network.size()-1; j>=0; j-- )
    {
        vector<vector<float>> new_output_layer = backpropagate_layer(network[j],ytrue);
        ytrue = new_output_layer;
    }

}
void NN::train () //
{
    int counter =0; // use the backprpagation function here
    int recordCounter =0;
    int sizeBatch=batchSize;
    int size = dt->train_size;
    Record * record;
    vector<Record*> XB;
    extTrainBd = 0;

    map<int, vector <Record*>> workerBatches;

    auto begin = chrono::high_resolution_clock::now();

    for (int epochCpt = 0; epochCpt < epochs ; epochCpt ++ ) {

        while (counter < size) {
            if (size - counter < batchSize)
                sizeBatch = size - counter;


            for (recordCounter = 0; recordCounter < sizeBatch; recordCounter++) {
                try {

                    record = dt->getTrainRecord();
                    XB.push_back(record);
                    extTrainBd += record->values.size() + 1;
                    counter++;
                }
                catch (std::exception const &e) {
                    cout << e.what() << endl;
                }

            }


            backpropagate(XB);


            for (int i = 0; i < XB.size(); i++) {
                delete XB[i];
            }

            XB.clear();


        }

        counter = 0;



    }

    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - begin;

    trainTime = duration.count();

}

void NN::Test( ){

    int counter =0;
    int recordCounter = 0;
    int size= dt->test_size;
    Record * record;
    vector<Record*> XB;
    int label;
    int sizeBatch=batchSize;
    std::ofstream classOutput;
    classOutput.open (logfile);

    extTestBd = 0;
    auto begin = chrono::high_resolution_clock::now();


    while (counter < size) {


        if (size - counter < batchSize)
            sizeBatch = size - counter;


        for (recordCounter = 0; recordCounter < sizeBatch; recordCounter++) {
            try {

                record = dt->getTestRecord();
                XB.push_back(record);
                extTrainBd += record->values.size() + 1;
                counter++;
            }
            catch (std::exception const &e) {
                cout << e.what() << endl;
            }

        }


        vector<int> prediction = predict(XB, true);

        for (int k=0; k < prediction.size();k++) {
            int label  = prediction[k];

            if (classOutput.is_open()) {
                classOutput << label << endl;

            }
        }


        for (int i = 0; i < XB.size(); i++) {
            delete XB[i];
        }

        XB.clear();


    }



    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end  - begin ;
    this->testTime = duration.count(); //- removeTime;
    cout << this->testTime << endl;
    classOutput.close();
}



