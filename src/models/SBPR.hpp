#pragma once

#include "model.hpp"

class SBPR : public model {
public:
    SBPR(corpus* corp, int K, double lambda, double biasReg) : model(corp)
    , K(K)
    , lambda(lambda)
    , biasReg(biasReg)
    {}
    
    ~SBPR(){}
    
    void init();
    void cleanUp();
    
    double prediction(int user, int item, int last_item, map<int, int> friend_items);
    
    int sampleUser();
    void train(int iterations, double learn_rate);
    void oneiteration(double learn_rate);
    void updateFactors(int user_id, int pos_item_id, int neg_item_id, int friend_item_id, int s_uk, double learn_rate);
    string toString();
    void copyBestModel();
    void loadBestModel();
    
    /* variables */
    int K;
    double* b_i;
    double* r_u;
    double* r_i;

    vector<map<int, int> > friend_items_per_user;
     
    
    /* hyper-parameters */
    double lambda;
    double biasReg;
};
