#pragma once

#include "model.hpp"

class POP : public model {
public:
    POP(corpus* corp) : model(corp) {}
    ~POP(){}

    double prediction(int user, int item, int last_item, map<int, int> friend_items);
        void loadBestModel();
    string toString();
};
