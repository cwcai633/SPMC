#pragma once

#include "model.hpp"

class BPRMF : public model {
public:
    BPRMF(corpus* corp, int K, double lambda, double biasReg) : model(corp)
    , K(K)
    , lambda(lambda)
    , biasReg(biasReg)
    {}
    
    ~BPRMF(){}
    
    void init();
    void cleanUp();
    
    double prediction(int user, int item, int last_item, map<int, int> friend_items);
    
    int sampleUser();
    void train(int iterations, double learn_rate);
    void oneiteration(double learn_rate);
    void updateFactors(int user_id, int pos_item_id, int neg_item_id, double learn_rate);
    string toString();
    void copyBestModel();
    void loadBestModel();
    
    /* variables */
    int K;
    double* b_i;
    double* r_u;
    double* r_i;
    
    /* hyper-parameters */
    double lambda;
    double biasReg;
};
