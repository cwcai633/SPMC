#include "FPMC.hpp"
#define V_UI(i, j) (v_ui[(i) * K + j])
#define V_IU(i, j) (v_iu[(i) * K + j])
#define V_IL(i, j) (v_il[(i) * K + j])
#define V_LI(i, j) (v_li[(i) * K + j])

void FPMC::init() {
    v_ui = new double[nUsers * K];
    v_iu = new double[nItems * K];
    v_il = new double[nItems * K];
    v_li = new double[nItems * K];
    b_i = new double[nItems];
    NW = nUsers * K + nItems * K + nItems * K + nItems * K + nItems;
    bestW = new double[NW];         

    for (int i = 0; i < nUsers; ++i) {
        for (int j = 0; j < K; ++j) {
            V_UI(i, j) = normal_frand(0, 0.1);
        }
    }

    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            V_IU(i, j) = normal_frand(0, 0.1);
        }
    }

    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            V_IL(i, j) = normal_frand(0, 0.1);
        }
    }

    for (int i = 0; i < nItems; ++i) {
        for (int j = 0; j < K; ++j) {
            V_LI(i, j) = normal_frand(0, 0.1);
        }
    }

    for (int i = 0; i < nItems; ++i) {
        b_i[i] = 0;
    }

    copyBestModel();
}

void FPMC::cleanUp() {
    delete [] v_ui;
    delete [] v_iu;
    delete [] v_il;
    delete [] v_li;
    delete [] b_i;
}

double FPMC::prediction(int user, int item, int last_item, map<int, int> friend_items) {
    // time = -1 means now
    double res = b_i[item];
    for (int k = 0; k < K; ++k) {
        res += V_UI(user, k) * V_IU(item, k);
    }

    for (int k = 0; k < K; ++k) {
       res += V_IL(item, k) * V_LI(last_item, k);
    }
    return res;
}

int FPMC::sampleUser() {
    int u;
    do {
        u = (rand() % nUsers);
    } while (pos_per_user_seq[u].size() == 0 || pos_per_user_seq[u].size() == 1 || (int) pos_per_user_seq[u].size() == nItems);
    return u;   
}

void FPMC::updateFactors(int user_id, int pos_item_id, int neg_item_id, int last_item_id, double learn_rate) {
    map<int, int> null_map;
    double x_uti = prediction(user_id, pos_item_id, last_item_id, null_map);
    double x_utj = prediction(user_id, neg_item_id, last_item_id, null_map);
    double delta = 1 - sigmoid(x_uti - x_utj);

    b_i[pos_item_id] += learn_rate * (delta - biasReg * b_i[pos_item_id]);
    b_i[neg_item_id] += learn_rate * (-delta - biasReg * b_i[neg_item_id]);

    for (int k = 0; k < K; ++k) {
        double v_ui_uk = V_UI(user_id, k);
        double v_iu_ik = V_IU(pos_item_id, k);
        double v_iu_jk = V_IU(neg_item_id, k);
        V_UI(user_id, k) += learn_rate * (delta * (v_iu_ik - v_iu_jk) - lambda * v_ui_uk);
        V_IU(pos_item_id, k) += learn_rate * (delta * v_ui_uk - lambda * v_iu_ik);
        V_IU(neg_item_id, k) += learn_rate * (-delta * v_ui_uk - lambda * v_iu_jk); 
    }

    for (int k = 0; k < K; ++k) {
        double v_il_ik = V_IL(pos_item_id, k);
        double v_il_jk = V_IL(neg_item_id, k);
        double v_li_lk = V_LI(last_item_id, k);
        V_IL(pos_item_id, k) += learn_rate * (delta * v_li_lk - lambda * v_il_ik);
        V_IL(neg_item_id, k) += learn_rate * (-delta * v_li_lk - lambda * v_il_jk);
        V_LI(last_item_id, k) += learn_rate * (delta * (v_il_ik - v_il_jk) - lambda * v_li_lk);
    }
}

void FPMC::oneiteration(double learn_rate) {
    int u, i, j, l, t;
    vector<int> *user_time_matrix = new vector<int>[nUsers];
    for (int uc = 0; uc < nUsers; ++uc) {
        for (int time = 0; time < (int) pos_per_user_seq[uc].size(); ++time) {
            // the real sequential time
            user_time_matrix[uc].push_back(time);
        }
    }

    for (int iter = 0; iter < num_pos_events; ++iter) {
        u = sampleUser();
        vector<int>& user_times = user_time_matrix[u];
        if (user_times.size() == 1) {
            user_times.pop_back();
            // reinstall
            for (int time = 0; time < (int) pos_per_user_seq[u].size(); ++time) {
                user_times.push_back(time);
            }
        }
        int rand_num;
        // t shouldn't be the last one
        do {
            rand_num = rand() % user_times.size();
            t = user_times[rand_num];
        } while (t == (int) (pos_per_user_seq[u].size() - 1));
        i = pos_per_user_seq[u][t].first;
        l = pos_per_user_seq[u][t+1].first;
        user_times[rand_num] = user_times.back();
        user_times.pop_back();
        do {
             j = rand() % nItems;
        } while (j == i);
        //} while (pos_per_user[u].find(j) != pos_per_user[u].end());
        updateFactors(u, i, j, l, learn_rate);
    }
    delete [] user_time_matrix;
}

void FPMC::loadBestModel() {
    int w = 0;
    for (int i = 0; i < nUsers * K; ++i) {
        v_ui[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        v_iu[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        v_il[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        v_li[i] = bestW[w++];
    } 
    for (int i = 0; i < nItems; ++i) {
        b_i[i] = bestW[w++];
    }
}


void FPMC::copyBestModel() {
    int w = 0;
    for (int i = 0; i < nUsers * K; ++i) {
        bestW[w++] = v_ui[i];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = v_iu[i];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = v_il[i];
    } 
    for (int i = 0; i < nItems * K; ++i) {
        bestW[w++] = v_li[i];
    } 
    for (int i = 0; i < nItems; ++i) {
        bestW[w++] = b_i[i];
    } 
}

void FPMC::train(int iterations, double learn_rate) {
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
    fprintf(stderr, "\n\n <<< FPMC >>> Test AUC = %f, Test Std = %f\n", test, std);
    fprintf(stderr, "Best Iteration = %d, Val AUC = %f\n\n", best_iter, valid);
}

string FPMC::toString() {
    char str[10000];
    sprintf(str, "FPMC__K_%d_lambda_%.4f_biasReg_%.4f", K, lambda, biasReg);
    return str;
}
