#include "POP.hpp"

double POP::prediction(int user, int item, int last_item, map<int, int> friend_items) {
    return (double)pos_per_item[item].size();	// return popularity of item
}

void POP::loadBestModel() {}

string POP::toString()
{
    return "POP";
}
