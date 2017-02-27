#pragma once

#include "model.hpp"

class FPMC : public model {
public:
    struct comparator {
        bool operator() (pair<int, double> i, pair<int, double> j) {
            return i.second > j.second;
        }
    };

    FPMC(corpus *corp, int K, double lambda, double biasReg) : model(corp),
    K(K),
    lambda(lambda),
    biasReg(biasReg) {}

    ~FPMC() {}
    void init();
    void cleanUp();
    double prediction(int user, int item, int last_item, map<int, int> friend_items);

    int sampleUser();
    void train(int iterations, double learn_rate);
    void oneiteration(double learn_rate);
    void updateFactors(int user_id, int pos_item_id, int neg_item_id, int last_item, double learn_rate);
    string toString();
    void copyBestModel();
    void loadBestModel();

    int K;

    double *v_ui;
    double *v_iu;
    double *v_il;
    double *v_li;
    double *b_i;

    double lambda;
    double biasReg;
}; 
