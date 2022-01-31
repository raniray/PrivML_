#include <iostream>
#include <map>
#include "omp.h"
#include "neuron.h"



float neuron::Sigmoid(float x) {

    return 1.0/(1.0 +exp(-x));

}

neuron::neuron(vector<float> w, double alpha, int epochs, int batchSize, float th, DatasetReader *dt,bool debug) {



    this->debug =debug;
    this->w = w;
    this->alpha = alpha;
    this->batchSize=batchSize;
    this->th = th;
    this->dt = dt;
    this->epochs=epochs;


}

float neuron::predict (vector <float> r, bool test){

    float wx =0.0, proba =0.0;
    int res = 0;
    int size = r.size()-1;
    if (test) size = size + 1;

    for (int i=0; i < size ; i++)
        wx += w[i] * r[i];

    proba = Sigmoid(wx);

    if (debug)
    {
        cout << "WX " << wx << endl;
        cout << "Proba " << proba << endl;

    }


    return proba;
}



vector<float> neuron::predict_batch (vector<vector<float>> XB, bool test){
    int ypred;
    vector <float > res;
    vector <float> r;

    // Predict labels, compute error and transpose
    for (int j=0; j< XB.size(); j++ )
    {
        r = XB[j];
        ypred = predict(r,false);
        res.push_back(ypred);

    }


    if(!test){
        this->previous_input = XB;
        this->previous_output = this->new_output;
        this->new_output = res;
    }


    return res;
}


float neuron::dotprod ( vector <float> a , vector < float> b )
{
    float res = 0.0;
    for (int i =0; i < a.size(); i++)
        res += a[i] * b [i];

    return res;

}


vector<float> neuron::miniBatchGrad( vector<float> ypred,   vector<float> ytrue ) {


    vector <float > diff;
    vector <float> r;
    float inter = 0.0;
    int dim = this->previous_input[0].size();
    vector<vector<float>> XB = this->previous_input;
    // Compute XB transpose
    float transpose [dim] [XB.size()];


    for (int k=0; k < ypred.size(); k++)
    {
        diff[k] = ypred[k] - ytrue[k];
    }

    // Predict labels, compute error and transpose
    for (int j=0; j< XB.size(); j++ )
    {
        r = XB[j];

        for (int i=0; i < dim ; i++ )
        {
            transpose [i][j] = r[i];
        }

    }

    vector<float> prod;

    for (int i=0; i < dim ; i++ )
    {

        vector<float> line;

        for (int j=0; j< XB.size(); j++ )
        {
            line.push_back(transpose[i][j]);

        }

        inter = dotprod(line,diff)/ XB.size();
        prod.push_back(inter);
        line.clear();
    }


    if (debug ) {
        for (int i = 0; i < w.size(); i++)
            cout << "prod[" << i << "]= " << prod[i] << endl;
    }
    return  prod;

}

void neuron::train (vector<vector<float>> XB, vector<float> ytrue)
{

    vector<float> ypred = predict_batch (XB,false);
    vector<float> grad = miniBatchGrad(ypred, ytrue);
}
