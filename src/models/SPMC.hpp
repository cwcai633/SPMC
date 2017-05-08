#pragma once

#include "model.hpp"

class SPMC : public model {
public:
    SPMC(corpus *corp, int K, double lambda, double biasReg) : model(corp),
    K(K),
    lambda(lambda),
    biasReg(biasReg) {}

    ~SPMC() {}
    void init();
    void cleanUp();
    double prediction(int user, int item, int last_item, map<int, int> friend_items);

    int sampleUser();
    void train(int iterations, double learn_rate);
    void oneiteration(double learn_rate);
    void updateFactors(int user_id, int pos_item_id, int neg_item_id, int last_item, map<int, int> friend_items, double learn_rate);
    string toString();
    void copyBestModel();
    void loadBestModel();
    void analyze(int n, const char* path);

    /* variables */
    int K;
    double *gamma_u;
    double *gamma_i;
    double *theta_i;
    double *phi_i;
    double *psi_u;
    double *b_i;

    /* hyper-parameters */
    double lambda;
    double biasReg;
}; 
