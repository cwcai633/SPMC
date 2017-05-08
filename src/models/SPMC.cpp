#include "SPMC.hpp"
#define GAMMA_U(i, j) (gamma_u[(i) * K + j])
#define GAMMA_I(i, j) (gamma_i[(i) * K + j])
#define THETA_I(i, j) (theta_i[(i) * K + j])
#define THETA_L(i, j) (theta_l[(i) * K + j])
#define PHI_I(i, j)   (phi_i[(i) * K + j])
#define PHI_L(i, j)   (phi_l[(i) * K + j])
#define PSI_U(i, j)   (psi_u[(i) * K + j])
#define ALPHA         1.0

void SPMC::init() {
    gamma_u = new double[nUsers * K];
    gamma_i = new double[nItems * K];
    theta_i = new double[nItems * K];
    phi_i = new double[nItems * K];
    psi_u = new double[nUsers * K];
    b_i = new double[nItems];
    NW = 2 * nUsers * K + 3 * nItems * K + nItems;

    for (int i = 0; i < nUsers; ++i) {
        for (int j = 0; j < K; ++j) {
            GAMMA_U(i, j) = frand();
        }
    }

    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            GAMMA_I(i, j) = frand();
        }
    }

    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            THETA_I(i, j) = frand();
        }
    }

    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            PHI_I(i, j) = frand();
        }
    }

    for (int i = 0; i < nUsers; ++i) {
        for (int j = 0; j < K; ++j) {
            PSI_U(i, j) = 1.0;
        }
    }

    for (int i = 0; i < nItems; ++i) {
        b_i[i] = 0;
    }

    bestW = new double[NW]; 
    copyBestModel();
}

void SPMC::cleanUp() {
    delete [] gamma_u;
    delete [] gamma_i;
    delete [] theta_i;
    delete [] phi_i;
    delete [] psi_u;
    delete [] b_i;
}

double SPMC::prediction(int user, int item, int last_item, map<int, int> friend_items) {
    double res = b_i[item];

    for (int k = 0; k < K; ++k) {
        res += GAMMA_U(user, k) * GAMMA_I(item, k);
    }

    for (int k = 0; k < K; ++k) {
       res += THETA_I(item, k) * THETA_I(last_item, k);
    }

    double F = pow(friend_items.size(), ALPHA);

    for (auto it = friend_items.begin(); it != friend_items.end(); ++it) {
        int user_friend = it->first;
        int friend_item = it->second;
        double weight = 0.0;
        for (int k = 0; k < K; ++k) {
            weight += PSI_U(user, k) * PSI_U(user_friend, k);
        }
        weight = sigmoid(weight);
        for (int k = 0; k < K; ++k) {
            res += 2 * weight * PHI_I(item, k) * PHI_I(friend_item, k) / F;
        }
    }
    
    return res;
}

int SPMC::sampleUser() {
    int u;
    do {
        u = (rand() % nUsers);
    } while (pos_per_user_seq[u].size() == 0 || pos_per_user_seq[u].size() == 1 || (int) pos_per_user_seq[u].size() == nItems);
    return u; 
}

void SPMC::updateFactors(int user_id, int pos_item_id, int neg_item_id, int last_item_id, map<int, int> friend_items, double learn_rate) {
    double x_uti = prediction(user_id, pos_item_id, last_item_id, friend_items);
    double x_utj = prediction(user_id, neg_item_id, last_item_id, friend_items);
    double delta = 1 - sigmoid(x_uti - x_utj);

    b_i[pos_item_id] += learn_rate * (delta - biasReg * b_i[pos_item_id]);
    b_i[neg_item_id] += learn_rate * (-delta - biasReg * b_i[neg_item_id]);

    for (int k = 0; k < K; ++k) {
        double g_u_uk = GAMMA_U(user_id, k);
        double g_i_ik = GAMMA_I(pos_item_id, k);
        double g_i_jk = GAMMA_I(neg_item_id, k);
        GAMMA_U(user_id, k) += learn_rate * (delta * (g_i_ik - g_i_jk) - lambda * g_u_uk);
        GAMMA_I(pos_item_id, k) += learn_rate * (delta * g_u_uk - lambda * g_i_ik);
        GAMMA_I(neg_item_id, k) += learn_rate * (-delta * g_u_uk - lambda * g_i_jk); 
    }

    for (int k = 0; k < K; ++k) {
        double t_i_ik = THETA_I(pos_item_id, k);
        double t_i_jk = THETA_I(neg_item_id, k);
        double t_l_lk = THETA_I(last_item_id, k);
        THETA_I(pos_item_id, k) += learn_rate * (delta * t_l_lk - lambda * t_i_ik);
        THETA_I(neg_item_id, k) += learn_rate * (-delta * t_l_lk - lambda * t_i_jk);
        THETA_I(last_item_id, k) += learn_rate * (delta * (t_i_ik - t_i_jk) - lambda * t_l_lk);
    }


    double F = pow(friend_items.size(), ALPHA);
    int friends_size = friend_items.size();

    double* psi_u_temp = new double[K];
    memcpy(psi_u_temp, psi_u + K * user_id, sizeof(double) * K);
    for (auto it = friend_items.begin(); it != friend_items.end(); ++it) {
        int user_friend_id = it->first;
        int friend_item_id = it->second;
        double phi_i_f = 0; // inner product of phi_i and phi_i'
        double phi_j_f = 0; // inner product of phi_j and phi_i'
        double psi_u_f = 0; // inner product of psi_u and psi_u'

        for (int k = 0; k < K; ++k) {
            phi_i_f += PHI_I(pos_item_id, k) * PHI_I(friend_item_id, k); 
        }

        for (int k = 0; k < K; ++k) {
            phi_j_f += PHI_I(neg_item_id, k) * PHI_I(friend_item_id, k); 
        }

        for (int k = 0; k < K; ++k) {
            psi_u_f += psi_u_temp[k] * PSI_U(user_friend_id, k); 
        }
        psi_u_f = sigmoid(psi_u_f);

        for (int k = 0; k < K; ++k) {
            double phi_i_k = PHI_I(pos_item_id, k);
            double phi_j_k = PHI_I(neg_item_id, k);
            double phi_l_k = PHI_I(friend_item_id, k);
            double psi_u_k = psi_u_temp[k];
            double psi_f_k = PSI_U(user_friend_id, k);

            PSI_U(user_id, k) += learn_rate * (delta * 2 * (psi_f_k * psi_u_f * (1.0 - psi_u_f)) * (phi_i_f - phi_j_f) / F - lambda * psi_u_k / friends_size);
            PSI_U(user_friend_id, k) += learn_rate * (delta * 2 * psi_u_k * psi_u_f * (1.0 - psi_u_f) * (phi_i_f -phi_j_f) / F - lambda * psi_f_k);
            PHI_I(pos_item_id, k) += learn_rate * (delta * 2 * psi_u_f * phi_l_k / F - lambda * phi_i_k / friends_size);
            PHI_I(neg_item_id, k) += learn_rate * (-delta * 2 * psi_u_f * phi_l_k / F - lambda * phi_j_k / friends_size);
            PHI_I(friend_item_id, k) += learn_rate * (delta * 2 * psi_u_f * (phi_i_k - phi_j_k) / F - lambda * phi_l_k);
        }
    }
    delete [] psi_u_temp;
}

void SPMC::oneiteration(double learn_rate) {
    int user, pos_item, neg_item, last_item, t;
    vector<int> *user_time_matrix = new vector<int>[nUsers];
    for (int uc = 0; uc < nUsers; ++uc) {
        for (int time = 0; time < (int) pos_per_user_seq[uc].size(); ++time) {
            user_time_matrix[uc].push_back(time);
        }
    }

    for (int iter = 0; iter < num_pos_events; ++iter) {
        user = sampleUser();
        vector<int>& user_times = user_time_matrix[user];
        if (user_times.size() == 1) {
            user_times.pop_back();
            // reinstall
            for (int time = 0; time < (int) pos_per_user_seq[user].size(); ++time) {
                user_times.push_back(time);
            }
        }
        int rand_num;
        // t shouldn't be the last one
        do {
            rand_num = rand() % user_times.size();
            t = user_times[rand_num];
        } while (t == (int) (pos_per_user_seq[user].size() - 1));
        pos_item = pos_per_user_seq[user][t].first;
        last_item = pos_per_user_seq[user][t+1].first;
        user_times[rand_num] = user_times.back();
        user_times.pop_back();
        do {
             neg_item = rand() % nItems;
        } while (neg_item == pos_item);

        long long user_time = pos_per_user[user][pos_item];
        vector<int> user_friends = trustMap[user];
        map<int, int> friend_items;
        for (int i = 0; i < (int) user_friends.size(); ++i) {
            long long friend_time = -1;
            int friend_item_id = -1;
            int user_friend_id = user_friends[i];
            int j = (int) pos_per_user_seq[user_friend_id].size() - 1;
            while (j >= 0 && pos_per_user_seq[user_friend_id][j].second < user_time) {
                friend_time = pos_per_user_seq[user_friend_id][j].second;
                friend_item_id = pos_per_user_seq[user_friend_id][j].first;
                j--;
            }

            if (friend_time != -1 and friend_item_id != -1) {
                friend_items[user_friend_id] = friend_item_id; 
            }
        }
        updateFactors(user, pos_item, neg_item, last_item, friend_items, learn_rate);
    }
    delete [] user_time_matrix;
}

void SPMC::loadBestModel() {
    int w = 0;
    for (int i = 0; i < nUsers * K; ++i) {
        gamma_u[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        gamma_i[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        theta_i[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        phi_i[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems; ++i) {
        b_i[i] = bestW[w++];
    }
    for (int i = 0; i < nUsers * K; ++i) {
        psi_u[i] = bestW[w++];
    } 
}

void SPMC::copyBestModel() {
    int w = 0;
    for (int i = 0; i < nUsers * K; ++i) {
        bestW[w++] = gamma_u[i];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = gamma_i[i];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = theta_i[i];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = phi_i[i];
    } 
    for (int i = 0; i < nItems; ++i) {
        bestW[w++] = b_i[i];
    }
    for (int i = 0; i < nUsers * K; ++i) {
        bestW[w++] = psi_u[i];
    } 
}

void SPMC::train(int iterations, double learn_rate) {
    fprintf(stderr, "%s", ("\n<<< " + toString() + " >>>\n\n").c_str());

    double bestValidAUC = -1;
    int best_iter = 0;

    // SGD begins
    for (int iter = 1; iter <= iterations; iter ++) {
        
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
    fprintf(stderr, "\n\n <<< SPMC >>> Test AUC = %f, Test Std = %f\n", test, std);
    fprintf(stderr, "Best Iteration = %d, Val AUC = %f\n\n", best_iter, valid);
}

string SPMC::toString() {
    char str[10000];
    sprintf(str, "SPMC__K_%d_lambda_%.4f_biasReg_%.4f", K, lambda, biasReg);
    return str;
}

string SPMC::analyze(int n, const char* path) {
    for (int iter = 0; iter < n; ++iter) {
        int u = rand() % nUsers;
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
}
