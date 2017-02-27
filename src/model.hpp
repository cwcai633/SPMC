#pragma once

#include "common.hpp"
#include "corpus.hpp"

enum action_t { COPY, INIT, FREE };

struct compScore {
    bool operator() (pair<int, double> item1, pair<int, double> item2) {
        return item1.second > item2.second;
    }
};


class model {
public:
    struct comparator {
        bool operator() (pair<int, long long> i, pair<int, long long> j) {
            // reversed order
            return (i.second > j.second);
        }
    } comp;

    model() {}
    model(corpus* corp) : corp(corp) {
        nUsers = corp->nUsers;
        nItems = corp->nItems;
        nVotes = corp->nVotes;
        trustMap = corp->trustMap;
        
        // leave out `two' for each user
        test_per_user = new pair<int,long long> [nUsers];
        val_per_user  = new pair<int,long long> [nUsers];
        for (int u = 0; u < nUsers; u ++) {
            test_per_user[u] = make_pair(-1, -1);  // -1 means empty
            val_per_user[u]  = make_pair(-1, -1);
        }

        // split into training set AND valid set AND test set 
        // NOTE: never use corp->V as the training set
        pos_per_user = new map<int,long long>[nUsers];
        pos_per_item = new map<int,long long>[nItems];
        pos_per_user_seq = new vector<pair<int, long long> >[nUsers];

        for (int x = 0; x < nVotes; x ++) {
            vote* v = corp->V.at(x);
            int user = v->user;
            int item = v->item;
            long long voteTime = v->voteTime; 

            pos_per_user[user][item] = voteTime;
            pos_per_item[item][user] = voteTime;
            pos_per_user_seq[user].push_back(make_pair(item, voteTime));
        }

        for (int u = 0; u < nUsers; ++u) {
            if (pos_per_user_seq[u].size() < 2) {
                continue;
            }
            sort(pos_per_user_seq[u].begin(), pos_per_user_seq[u].end(), comp);
            test_per_user[u] = pos_per_user_seq[u][0];
            val_per_user[u] = pos_per_user_seq[u][1];
            int test_item = test_per_user[u].first;
            int val_item = val_per_user[u].first;
            pos_per_user_seq[u].erase(pos_per_user_seq[u].begin(), pos_per_user_seq[u].begin() + 2);
            pos_per_user[u].erase(test_item);
            pos_per_user[u].erase(val_item);
            pos_per_item[test_item].erase(u);
            pos_per_item[val_item].erase(u);
        } 

        // sanity check
        for (int u = 0; u < nUsers; u ++) {
            if (test_per_user[u].first == -1 || val_per_user[u].first == -1) {
                fprintf(stderr, "\n\n Corpus split exception when spliting %d.\n", u);
                exit(1);
            }
        }

        // calculate num_pos_events
        num_pos_events = 0;
        for (int u = 0; u < nUsers; u ++) {
            num_pos_events += pos_per_user[u].size();
        }
    }

    ~model() {
        delete [] pos_per_user;
        delete [] pos_per_item;
        delete [] pos_per_user_seq;

        delete [] test_per_user;
        delete [] val_per_user;
    }

    /* Model parameters */
    int NW; // Total number of parameters
    double* bestW;

    /* Corpus related */
    corpus* corp; // dangerous
    int nUsers; // Number of users
    int nItems; // Number of items
    int nVotes; // Number of ratings

    map<int,long long>* pos_per_user;
    map<int,long long>* pos_per_item;
    vector<pair<int,long long> >* pos_per_user_seq;

    pair<int,long long>* val_per_user;
    pair<int,long long>* test_per_user;

    int num_pos_events;

    /* additional information for demo paper */
    map<int, double> itemPrice;
    map<int, string> itemBrand;

    map<int, vector<int> > trustMap;
    
    virtual void AUC(double* AUC_val, double* AUC_test, double* std, bool is_val);
    
    virtual void copyBestModel();
    virtual void loadBestModel() = 0;
    virtual void saveModel(const char* path);
    virtual void loadModel(const char* path);
    virtual void coldStart(int interval, const char* path); 
    virtual int hitsAt(set<pair<int, double>, compScore>& ranked_items, set<int>& correct_items, int n);
    virtual double recallAt(set<pair<int, double>, compScore>& ranked_items, set<int>& correct_items, int n);
    virtual double precisionAt(set<pair<int, double>, compScore>& ranked_items, set<int>& correct_items, int n);
    virtual void metrics(const char* path, int n);
    
    virtual string toString();

private:
    virtual double prediction(int user, int item, int last_item, map<int, int> friend_items) = 0;
};
