#include "model.hpp"

void model::AUC(double* AUC_val, double* AUC_test, double* std, bool is_val) {
    double *AUC_u_val = new double[nUsers]; 
    double *AUC_u_test = new double[nUsers];
    *AUC_val = 0;
    *AUC_test = 0;
    *std = 0;
    for (int u = 0; u < nUsers; ++u) {
        int val_item = val_per_user[u].first; 
        int test_item = test_per_user[u].first; 
        int last_item_val = pos_per_user_seq[u][0].first;
        int last_item_test = val_item;

        long long user_time_val = val_per_user[u].second;
        long long user_time_test = test_per_user[u].second;
        vector<int> user_friends = trustMap[u];
        map<int, int> friend_items_val;
        map<int, int> friend_items_test;
        for (int i = 0; i < (int) user_friends.size(); ++i) {
            int user_friend = user_friends[i];
            long long friend_time = -1;
            int friend_item = -1;
            int j = (int) pos_per_user_seq[user_friend].size() - 1;
            while (j >= 0 && pos_per_user_seq[user_friend][j].second < user_time_val) {
                friend_time = pos_per_user_seq[user_friend][j].second;
                friend_item = pos_per_user_seq[user_friend][j].first;
                j--;
            }

            if (friend_time != -1 and friend_item != -1) {
                friend_items_val[user_friend] = friend_item; 
            }
        }

        for (int i = 0; i < (int) user_friends.size(); ++i) {
            int user_friend = user_friends[i];
            if (val_per_user[user_friend].second < user_time_test) {
                friend_items_test[user_friend] = val_per_user[user_friend].first;
                continue;
            }
            long long friend_time = -1;
            int friend_item = -1;
            int j = (int) pos_per_user_seq[user_friend].size() - 1;
            while (j >= 0 && pos_per_user_seq[user_friend][j].second < user_time_test) {
                friend_time = pos_per_user_seq[user_friend][j].second;
                friend_item = pos_per_user_seq[user_friend][j].first;
                j--;
            }

            if (friend_time != -1 and friend_item != -1) {
                friend_items_test[user_friend] = friend_item; 
            }
        }

        double score_val = prediction(u, val_item, last_item_val, friend_items_val);
        double score_test = prediction(u, test_item, last_item_test, friend_items_test); 
        int behind_val = 0;
        int behind_test = 0;
        int count = 0;
        set<int> item_selected;
        int num_iter = min(5000, nItems);

        for (int it = 0; it < num_iter; ++it) {
            int i; 
            do {
                i = rand() % nItems;
            } while (item_selected.find(i) != item_selected.end());
            item_selected.insert(i);
            bool viewed_by_user = (pos_per_user[u].find(i) != pos_per_user[u].end() || test_per_user[u].first == i || val_per_user[u].first == i); 
            if (viewed_by_user) continue;
            double pred_score_val = prediction(u, i, last_item_val, friend_items_val);
            double pred_score_test = prediction(u, i, last_item_test, friend_items_test);
            ++count; 

            if (pred_score_val < score_val) {
                ++behind_val;
            }

            if (pred_score_test < score_test) {
                ++behind_test;
            }
        }
        AUC_u_val[u] = (1.0 * behind_val / count);
        AUC_u_test[u] = (1.0 * behind_test / count);
    }
    for (int u = 0; u < nUsers; ++u) {
        *AUC_val += AUC_u_val[u];
    }
    *AUC_val /= nUsers;
    for (int u = 0; u < nUsers; ++u) {
        *AUC_test += AUC_u_test[u];
    }
    *AUC_test /= nUsers;
    double var = 0;
    if (is_val) {
        for (int u = 0; u < nUsers; ++u) {
            var += square(AUC_u_val[u] - *AUC_val);
        }
    } else {
        for (int u = 0; u < nUsers; ++u) {
            var += square(AUC_u_test[u] - *AUC_test);
        }
    }
    *std = sqrt(var/nUsers);
    delete[] AUC_u_val;
    delete[] AUC_u_test;
}

void model::copyBestModel() {

}

void model::saveModel(const char* path) {
    FILE* f = fopen(path, "w");    
    fprintf(f, "{\n");
    fprintf(f, " \"NW\": %d,\n", NW);
    fprintf(f, " \"W\": [");
    for (int w = 0; w < NW; ++w) { 
        fprintf(f, "%f", bestW[w]);
        if (w < NW - 1) fprintf(f, ", ");
    }
    fprintf(f, "]\n");
    fprintf(f, "}\n");
}

void model::loadModel(const char* path) {
    fprintf(stderr, "\n loading parameters from %s.\n", path);
    ifstream in;
    in.open(path);
    if (!in.good()) {
        fprintf(stderr, "Cannot read init solution from %s.\n", path);
        exit(1);
    }
    string line;
    string st;
    char ch;
    while(getline(in, line)) {
        stringstream ss(line);
        ss >> st;
        if (st == "\"NW\":") {
            int nw;
            ss >> nw;
            if (nw != NW) {
                fprintf(stderr, "NW not match. NW should be %d\n", NW);
                exit(1);
            }
            continue;
        }

        if (st == "\"W\":") {
            ss >> ch; // skip '['
            for (int w = 0; w < NW; w ++) {
                if (! (ss >> bestW[w] >> ch)) {
                    fprintf(stderr, "Read W[] error.");
                    exit(1);
                }
            }
            break;
        }
    }
    in.close();
}

void model::coldStart(int interval, const char* path) {
    int uCountMax = INT_MIN;
    int uCountMin = INT_MAX;
    for (int u = 0; u < nUsers; ++u) {
        int uCount = pos_per_user[u].size(); 
        if (uCount > uCountMax) {
            uCountMax = uCount;
        }

        if (uCount < uCountMin) {
            uCountMin = uCount;
        }
    }
    
    int binNum = (uCountMax - uCountMin) / interval + 1;
    pair<double, int> *binAUC = new pair<double, int> [binNum];
    map<int, double> AUC;

    for (int i = 0; i < binNum; ++i) {
        binAUC[i].first = 0.0;
        binAUC[i].second = 0;
    } 

    for (int u = 0; u < nUsers; ++u) {
        int item = test_per_user[u].first;
        int last_item = val_per_user[u].first; 
        long long user_time = test_per_user[u].second;
        vector<int> user_friends = trustMap[u];
        map<int, int> friend_items;
        for (int i = 0; i < (int) user_friends.size(); ++i) {
            int user_friend = user_friends[i];
            if (val_per_user[user_friend].second < user_time) {
                friend_items[user_friend] = val_per_user[user_friend].first;
                continue;
            }
            long long friend_time = -1;
            int friend_item = -1;
            int j = (int) pos_per_user_seq[user_friend].size() - 1;
            while (j >= 0 && pos_per_user_seq[user_friend][j].second < user_time) {
                friend_time = pos_per_user_seq[user_friend][j].second;
                friend_item = pos_per_user_seq[user_friend][j].first;
                j--;
            }

            if (friend_time != -1 and friend_item != -1) {
                friend_items[user_friend] = friend_item; 
            }
        }
        double score = prediction(u, item, last_item, friend_items);
        int behind = 0;
        int count = 0; 
        set<int> item_selected;
        int num_iter = min(5000, nItems);
        for (int it = 0; it < num_iter; ++it) {
            int i; 
            do {
                i = rand() % nItems;
            } while (item_selected.find(i) != item_selected.end());
            item_selected.insert(i);
            bool viewed_by_user = (pos_per_user[u].find(i) != pos_per_user[u].end() || test_per_user[u].first == i || val_per_user[u].first == i); 
            if (viewed_by_user) continue;
            double pred_score = prediction(u, i, last_item, friend_items);
            ++count;
            
            if (pred_score < score) {
                ++behind;
            }
        }
        AUC[u] = 1.0 * behind / count;  
    }
    
    for (int u = 0; u < nUsers; ++u) {
        int uCount = pos_per_user[u].size(); 
        int bin = (uCount - uCountMin) / interval;
        binAUC[bin].first += AUC[u];
        binAUC[bin].second += 1;
    } 

    FILE* f = fopen(path, "w"); 
    fprintf(f, "%d\n", uCountMin);
    fprintf(f, "%d\n", uCountMax);
    fprintf(f, "%d\n", binNum);
    
    for (int b = 0; b < binNum; ++b) {
        if (binAUC[b].second != 0) {
            fprintf(f, "%d %f %d\n", b * interval + uCountMin, binAUC[b].first/binAUC[b].second, binAUC[b].second);
        }
    }
    delete [] binAUC;
}

int model::hitsAt(set<pair<int, double>, compScore>& ranked_items, set<int>& correct_items, int n) {
    if (n < 1) {
        fprintf(stderr, "[HitAt]: n must be at least 1.");
        exit(1);
    }

    int hit_count = 0;

    for (set<pair<int, double>, compScore>::iterator it = ranked_items.begin(); it != ranked_items.end(); it++) {
        int item_id = it->first;

        if (correct_items.find(item_id) != correct_items.end()) {
            hit_count++;
            cout << item_id << endl;
        }
    }
    return hit_count;
}

double model::recallAt(set<pair<int, double>, compScore>& ranked_items, set<int>& correct_items, int n) {
    return 1.0 * hitsAt(ranked_items, correct_items, n) / correct_items.size();
}

double model::precisionAt(set<pair<int, double>, compScore>& ranked_items, set<int>& correct_items, int n) {
    return 1.0 * hitsAt(ranked_items, correct_items, n) / n;
}

void model::metrics(const char* path, int n) {
    fprintf(stderr, " \nCalculating metrics from %s...\n", path);
    loadModel(path);
    loadBestModel();
    double AUC_val, AUC_test, std;
    AUC(&AUC_val, &AUC_test, &std, false);

    double recall = 0;
    double precision = 0;
    vector<int> ranked_items;
    for (int user = 0; user < nUsers; user++) {
        set<pair<int, double>, compScore> ranked_items;
        set<int> correct_items;
        correct_items.insert(test_per_user[user].first);

        int last_item_test = val_per_user[user].first;;
        long long user_time_test = test_per_user[user].second;
        vector<int> user_friends = trustMap[user];
        map<int, int> friend_items_test;

        for (int i = 0; i < (int) user_friends.size(); ++i) {
            int user_friend = user_friends[i];
            if (val_per_user[user_friend].second < user_time_test) {
                friend_items_test[user_friend] = val_per_user[user_friend].first;
                continue;
            }
            long long friend_time = -1;
            int friend_item = -1;
            int j = (int) pos_per_user_seq[user_friend].size() - 1;
            while (j >= 0 && pos_per_user_seq[user_friend][j].second < user_time_test) {
                friend_time = pos_per_user_seq[user_friend][j].second;
                friend_item = pos_per_user_seq[user_friend][j].first;
                j--;
            }

            if (friend_time != -1 and friend_item != -1) {
                friend_items_test[user_friend] = friend_item; 
            }
        }

        for (int item = 0; item < nItems; item++) {
            bool viewed_by_user = (pos_per_user[user].find(item) != pos_per_user[user].end() || val_per_user[user].first == item); 
            if (viewed_by_user) {
                continue;
            }
            double pred_score_test = prediction(user, item, last_item_test, friend_items_test);
            ranked_items.insert(make_pair(item, pred_score_test));
            if ((int) ranked_items.size() > n) {
                ranked_items.erase(prev(ranked_items.end()));
            }
        }

        recall += recallAt(ranked_items, correct_items, n);
        precision += precisionAt(ranked_items, correct_items, n);
    }
    recall /= nUsers;
    precision /= nUsers;
    fprintf(stderr, "\n\n Test AUC = %f, Test Std = %f\n", AUC_test, std);
    fprintf(stderr, " Recall@%d = %f\n", n, recall);
    fprintf(stderr, " Precision@%d = %f\n", n, precision);
    fprintf(stderr, "}");
}

string model::toString() {
    return "Empty Model!";
}
