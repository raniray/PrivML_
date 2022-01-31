#include <iostream>
#include <map>
#include "omp.h"
#include "LR.h"


void LR::shuffle(DatasetReader * dt)
{
  if (dt->full) {
      dt->closeTraining();
      string datasetPath = dt->path + dt->chunk_prefix + "_" + std::to_string(dt->current_chunk) + ".data";
      string filename = mainpath + "DATA/Scripts/Shuffle.py ";
      std::string command = "python3 ";
      std::string arguments = datasetPath;
      command += filename + arguments;
      system(command.c_str());
      dt->current_chunk--;
      dt->nextChunk();
  } else {

      dt->closeTraining();
      string datasetPath = dt->path + "training.data";
      string filename = mainpath + "DATA/Scripts/Shuffle.py ";
      std::string command = "python3 ";
      std::string arguments = datasetPath;
      command += filename + arguments;
      system(command.c_str());
      delete dt->train;
      dt->train=new ifstream(dt->path+"training.data");

  }
}

Record * LR::recvRecord(int sockfd, sockaddr_in cliaddr, int dim, int  id)
{

  if (debug) cout << "recieving record " << id << endl;
  socklen_t addrlen = sizeof(cliaddr);

  int size = dim*200+dim+200;
  string recievedRec="";
  int n=0;
  while (size>0)
  {
    if ( size < 65000 )
    {
      char buf[size];
      n = recvfrom(sockfd, buf, size, 0, (struct sockaddr *)&cliaddr, &addrlen);
      //buf[n] = '\0';
      recievedRec+=buf;

    } else {

      char buf[65000];
      n = recvfrom(sockfd, buf, 65000, 0, (struct sockaddr *)&cliaddr, &addrlen);
      //buf[n] = '\0';
      recievedRec+=buf;

    }

    size -= n;

  }

  if (debug) cout << "I have recieved this message : " << recievedRec << endl;
  if (debug) cout << "The message size is : " << recievedRec.size() << endl;

  std::vector<int> values;
  Record * r;


  char * dup = strdup(recievedRec.c_str());
  char * v=strtok (dup,",");

  while (v != NULL) {

    values.push_back(atoi(v));
    v=strtok (NULL,",");
  }
  try {
    free(dup);
    free(v);
  }catch (exception e )
  {
    cout << "trying to remove dup value  in PPrecord receive " << endl;
    cout << e.what() << endl;
  }
  r = new Record (id,values);
  r->print();
  return r;

}

void LR::sendResponse(int sockfd, sockaddr_in cliaddr, int res)
{
  char buffer[200];
  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer,"%s", std::to_string(res).c_str());
  sendto(sockfd, buffer, 200, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
  if (debug) cout << "sent response " << endl;
}

int
LR ::sendMsg(string msg, int sockfd, sockaddr_in cliaddr)
{
  int n = sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
  return  n;
}

string
LR :: recvMsg(int sockfd,  sockaddr_in cliaddr)
{
  string res ="";
  socklen_t addrlen = sizeof(cliaddr);
  char buf[200];
  int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);
  if (n!=-1) {
    buf[n] = '\0';
    res = buf;
    if (debug) cout << "received msg " << res << endl;
  }
  if (debug) cout << "received nothing " << res << endl;

  return res;
}


float LR::Sigmoid(float x) {

    //cout << "The value of x is : " <<  x << endl;
    return 1.0/(1.0 +exp(-x));

}

LR::LR(vector<float> w, double alpha, int epochs, int batchSize, float th, DatasetReader *dt, string logfile,  sockaddr_in cliaddr, int sockfd,
       bool debug, string mainpath, int sgdWorkers) {


  trainTime =0.0;
  testTime =0.0;
  extTrainBd =0;
  extTestBd = 0;
  clientTestTime =0.0;
  this->debug =debug;
  this->sockfd = sockfd;
  this->cliaddr = cliaddr;
  this->w = w;
  this->alpha = alpha;
  this->batchSize=batchSize;
  this->th = th;
  this->dt = dt;
  this->logfile=logfile;
  this->epochs=epochs;
  this->mainpath = mainpath;
  this->sgdWorkers=sgdWorkers;

  if (debug )
  {
    for (int i=0; i< w.size(); i++)
      cout << "w["<<i<<"]= " << w[i] <<endl;

    cout << "alpha " << alpha << endl;
    cout << "batchSize " << batchSize<< endl;
    cout << "th " << th << endl;
    cout << "logfile " << logfile << endl;
    cout << ""<< endl;
  }

}

int LR::predict(Record *r, bool test ) {

  float wx =0.0, proba =0.0;
  int res = 0;
  int size = r->values.size()-1;
  if (test) size = size + 1;

  for (int i=0; i < size ; i++)
    wx += w[i] * r->values[i];

  proba = Sigmoid(wx);

  if (proba >= th )
    res = 1;
  else
    res = 0;


  if (debug)
  {
    cout << "WX " << wx << endl;
    cout << "Proba " << proba << endl;
    cout << "Res " << res << endl;
  }

  return res;
}


float dotprod ( vector <float> a , vector < float> b )
{
  float res = 0.0;
  for (int i =0; i < a.size(); i++)
    res += a[i] * b [i];

  return res;

}

vector<float> LR::miniBatchGrad(vector<Record *> XB) {

  int ypred, ytrue;
  vector <float > diff;
  Record *r;
  int dim = XB[0]->values.size() - 1;
  float inter = 0.0;
  // Compute XB transpose
  float transpose [dim] [XB.size()];


  // Predict labels, compute error and transpose
  for (int j=0; j< XB.size(); j++ )
  {
    r = XB[j];
    if (debug) r->print();
    ypred = predict(r,false);
    ytrue = r->values[r->values.size()-1];
    diff.push_back(ypred-ytrue);

    for (int i=0; i < dim ; i++ )
    {
      transpose [i][j] = r->values[i];
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

void LR::train ()
{
  int counter =0;
  int recordCounter =0;
  int sizeBatch=batchSize;
  int size = dt->train_size;
  Record * record;
  extTrainBd = 0;

  map<int, vector <Record*>> workerBatches;

 auto begin = chrono::high_resolution_clock::now();

 for (int epochCpt = 0; epochCpt < epochs ; epochCpt ++ ) {

     shuffle(dt);
     while (counter < size) {
         int worker = 0;
         if (size - counter < sgdWorkers * batchSize)
             sgdWorkers = (size - counter)/batchSize + 1;

         for (worker = 0; worker < sgdWorkers; worker++) {
             vector<Record *> XB;
             if (size - counter < batchSize)
                 sizeBatch = size - counter +1;

             for (recordCounter = 0; recordCounter < sizeBatch; recordCounter++) {
                 try {

                     record = dt->getTrainRecord();
                     XB.push_back(record);
                     extTrainBd += sizeof(int) * record->values.size();
                     counter++;
                 }
                 catch (std::exception const &e) {
                     cout << e.what() << endl;
                 }
             }

             workerBatches.insert(std::make_pair(worker, XB));
         }

         vector<float> averageGrad(w.size(), 0.0);
         map<int, vector<float>> workerGrads;

        #pragma omp parallel
         {

            map<int, vector<Record *>>::iterator it;
            map<int, vector<float>>::iterator itGrad;
            

            for (it = workerBatches.begin(); it != workerBatches.end(); ++it) {

                 vector<Record *> XB = it->second;
                 vector<float> grad = miniBatchGrad(XB);
                #pragma omp critical
                 {
                     workerGrads.insert(std::make_pair(worker, grad));
                     for (int i = 0; i < grad.size(); i++)
                         averageGrad[i] += grad[i];
                 }
             }
         }





         for (int i = 0; i < w.size(); i++)
             w[i] = w[i] - alpha * averageGrad[i];


         if (debug) {
             for (int i = 0; i < w.size(); i++)
                 cout << "W after update " << w[i] << endl;
         }

         workerGrads.clear();
         workerBatches.clear();


     }

     counter = 0;
     dt->test->clear();
     dt->test->seekg(0);
     dt->current_test_record=0;
     dt->train->clear();
     dt->train->seekg(0);
     dt->current_train_record=0;
 }


     //cout << epochCpt << endl;



  auto end = chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - begin;

  trainTime = duration.count();

}

void LR::Test( ){

  int counter =0;
  int size= dt->test_size;
  Record * record;
  int label;

  std::ofstream classOutput;
  classOutput.open (logfile);

  extTestBd = 0;
  auto begin = chrono::high_resolution_clock::now();
  while (counter < size) {

    try {
      record = dt->getTestRecord();
      //record->print();

        extTestBd += sizeof(int)*record->values.size();
    }
    catch (std::exception const &e) {
      //std::cout << "Exception: " << e.what() << "\n";

    }

    counter++;

    label = predict(record,true);

    if(classOutput.is_open())
    {
      classOutput<<label<< endl;

    }
    delete record;
  }

  auto end = chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end  - begin ;
  this->testTime = duration.count(); //- removeTime;
  cout << this->testTime << endl;
  classOutput.close();
}

void LR::RemoteTest(){

  int counter =0;
  int size=dt->test_size;
  Record * record;
  std::ofstream classOutput;
  int label;

  std::ofstream inter;

  classOutput.open (logfile);
  extTestBd = 0;

  auto begin = chrono::high_resolution_clock::now();

  while (counter < size) {

    try {

      record = recvRecord(sockfd,cliaddr,dt->dimension-1, counter);

      if (debug) record->print();

      extTestBd += sizeof(int)*record->values.size();
    }
    catch (std::exception const &e) {
      std::cout << "Exception: " << e.what() << "\n";
    }

    counter++;
    label = predict(record,true);
    sendResponse(sockfd, cliaddr, label);
    classOutput<<label<< endl;
    extTestBd+=sizeof(label);
    delete record;

  }

  auto en = chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = en  - begin ;
  this->testTime = duration.count();


  char buf[200];
  socklen_t addrlen = sizeof(cliaddr);
  int n = recvfrom(sockfd, buf, 200, 0, (struct sockaddr *)&cliaddr, &addrlen);
  buf[n] = '\0';
  this->clientTestTime = atof(buf);

  classOutput.close();
}
