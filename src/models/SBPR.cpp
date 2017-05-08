#include "SBPR.hpp"
#define R_U(i, j) (r_u[(i) * K + j])
#define R_I(i, j) (r_i[(i) * K + j])

void SBPR::init() {
    b_i = new double[nItems];
    r_u = new double[nUsers * K];
    r_i = new double[nItems * K];
    NW = nItems + nUsers * K + nItems * K;
    bestW = new double[NW];         
    
    for (int i = 0; i < nItems; ++i) {
        b_i[i] = 0;
    }
    
    for (int i = 0; i < nUsers; ++i) {
        for (int j = 0; j < K; ++j) {
            R_U(i, j) = frand();
        }
    }
    
    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            R_I(i, j) = frand();
        }
    }

    for (int u = 0; u < nUsers; ++u) {
        map<int, int> item_friend; // the item-#friends map that record every item
        vector<int> friends = trustMap[u];
        for (int fu = 0; fu < (int) friends.size(); ++fu) {
            for (auto it = pos_per_user[friends[fu]].begin(); it != pos_per_user[friends[fu]].end(); ++it) {
                // u did not view, but his friends did
                if (pos_per_user[u].find(it->first) == pos_per_user[u].end()) {
                    if (item_friend.find(it->first) != item_friend.end()) {
                        item_friend[it->first]++;
                    } else {
                        item_friend[it->first] = 1;
                    }
                }
            }
        }

        friend_items_per_user.push_back(item_friend);
    }

    copyBestModel();
}

void SBPR::cleanUp() {
    delete [] b_i;
    delete [] r_u;
    delete [] r_i;
}

double SBPR::prediction(int user, int item, int last_item, map<int, int> friend_items) {
    double res = b_i[item];
    for (int k = 0; k < K; ++k) {
        res += R_U(user, k) * R_I(item, k);
    }
    return res;
}

int SBPR::sampleUser() {
    int u;
    do {
        u = (rand() % nUsers);
    } while (pos_per_user_seq[u].size() == 0 || (int) pos_per_user_seq[u].size() == nItems || trustMap[u].size() == 0 || friend_items_per_user[u].empty());
    return u; 
}

void SBPR::updateFactors(int user_id, int pos_item_id, int neg_item_id, int friend_item_id, int s_uk, double learn_rate) {
    map<int, int> dummy;
    double x_uik = prediction(user_id, pos_item_id, 0, dummy) - prediction(user_id, friend_item_id, 0, dummy);
    double x_ukj = prediction(user_id, friend_item_id, 0, dummy) - prediction(user_id, neg_item_id, 0, dummy);
    double delta1 = sigmoid(-x_uik / (1.0 + s_uk));
    double delta2 = sigmoid(-x_ukj);

    b_i[pos_item_id] += learn_rate * ((delta1 / (1.0 + s_uk)) - biasReg * b_i[pos_item_id]);
    b_i[neg_item_id] += learn_rate * ((-delta2) - biasReg * b_i[neg_item_id]);
    b_i[friend_item_id] += learn_rate * ((-delta1 / (1.0 + s_uk) + delta2) - biasReg * b_i[friend_item_id]);

    for (int k = 0; k < K; ++k) {
        double r_uk = R_U(user_id, k);
        double r_ik = R_I(pos_item_id, k);
        double r_jk = R_I(neg_item_id, k);
        double r_kk = R_I(friend_item_id, k);
        R_U(user_id, k) += learn_rate * (delta1 / (1.0 + s_uk) * (r_ik - r_kk) + delta2 * (r_kk - r_jk) - lambda * r_uk);
        R_I(pos_item_id, k) += learn_rate * (delta1 / (1.0 + s_uk) * r_uk - lambda * r_ik);
        R_I(neg_item_id, k) += learn_rate * (-delta2 * r_uk - lambda * r_jk);
        R_I(friend_item_id, k) += learn_rate * (-delta1 / (1.0 + s_uk) * r_uk + delta2 * r_uk - lambda * r_kk / (1.0 + s_uk));
    }
}

void SBPR::oneiteration(double learn_rate) {
    int u, i, j, k;
    vector<int> *user_matrix = new vector<int>[nUsers];
    for (int uc = 0; uc < nUsers; ++uc) {
        for (map<int, long long>::iterator it = pos_per_user[uc].begin(); it != pos_per_user[uc].end(); ++it) {
            user_matrix[uc].push_back(it->first);
        }
    }

    for (int iter = 0; iter < num_pos_events; ++iter) {
        u = sampleUser();
        vector<int>& user_items = user_matrix[u];
        if (user_items.size() == 0) {
            for (map<int, long long>::iterator it = pos_per_user[u].begin(); it != pos_per_user[u].end(); ++it) {
                user_items.push_back(it->first);
            }
        }

        int rand_num = rand() % user_items.size();
        i = user_items[rand_num];
        user_items[rand_num] = user_items.back();
        user_items.pop_back();

        map<int, int> item_friend = friend_items_per_user[u];
        do {
             j = rand() % nItems;
        } while (pos_per_user[u].find(j) != pos_per_user[u].end() or item_friend.find(j) != item_friend.end());
        map<int, int>::iterator it = item_friend.begin();
        advance(it, rand() % item_friend.size());
        k = it->first;
        int s_uk = item_friend[k];
        
        updateFactors(u, i, j, k, s_uk, learn_rate);
    }    
    delete [] user_matrix;
}

void SBPR::copyBestModel() {
    int w = 0;
    for (int i = 0; i < nItems; ++i) {
        bestW[w++] = b_i[i]; 
    }
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = r_i[i];
    }
    for (int i = 0; i < nUsers * K; ++i) {
        bestW[w++] = r_u[i];
    }
}

void SBPR::loadBestModel() {
    int w = 0;
    for (int i = 0; i < nItems; ++i) {
        b_i[i] = bestW[w++]; 
    }
    for (int i = 0; i < nItems * K; ++i) {
        r_i[i] = bestW[w++];
    }
    for (int i = 0; i < nUsers * K; ++i) {
        r_u[i] = bestW[w++];
    }

} 

void SBPR::train(int iterations, double learn_rate) {
    fprintf(stderr, "%s", ("\n<<< " + toString() + " >>>\n\n").c_str());

    double bestValidAUC = -1;
    int best_iter = 0;

    // SGD begins
    for (int iter = 1; iter <= iterations; ++iter) {
        
        // perform one iter of SGD
        double l_dlStart = clock_();
        oneiteration(learn_rate);
        fprintf(stderr, "Iter: %d, took %f\n", iter, clock_() - l_dlStart);

        if(iter % 5 == 0 && iter != 0) {
            double valid, test, std;
            AUC(&valid, &test, &std, true);
            fprintf(stderr, "[Valid AUC = %f], Test AUC = %f, Test Std = %f\n", valid, test, std);
            
            if (bestValidAUC < valid) {
                bestValidAUC = valid;
                best_iter = iter;
                copyBestModel();
            } else if (valid <= bestValidAUC && iter > best_iter + 50) {
                fprintf(stderr, "Overfitted. Exiting... \n");
                break;
            }
        }
    }

    loadBestModel();
    double valid, test, std;
    AUC(&valid, &test, &std, false);
    fprintf(stderr, "\n\n <<< SBPR >>> Test AUC = %f, Test Std = %f\n", test, std);
    fprintf(stderr, "Best Iteration = %d, Val AUC = %f\n\n", best_iter, valid);
}

string SBPR::toString() {
    char str[10000];
    sprintf(str, "SBPR__K_%d_lambda_%.4f_biasReg_%.4f", K, lambda, biasReg);
    return str;
}
