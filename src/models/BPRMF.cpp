#include "BPRMF.hpp"
#define R_U(i, j) (r_u[(i) * K + j])
#define R_I(i, j) (r_i[(i) * K + j])

void BPRMF::init() {
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
    copyBestModel();
}

void BPRMF::cleanUp() {
    delete [] b_i;
    delete [] r_u;
    delete [] r_i;
}

double BPRMF::prediction(int user, int item, int last_item, map<int, int> friend_items) {
    double res = b_i[item];
    for (int k = 0; k < K; ++k) {
        res += R_U(user, k) * R_I(item, k);
    }
    return res;
}

int BPRMF::sampleUser() {
    int u;
    do {
        u = (rand() % nUsers);
    } while (pos_per_user_seq[u].size() == 0 || (int) pos_per_user_seq[u].size() == nItems);
    return u; 
}

void BPRMF::updateFactors(int user_id, int pos_item_id, int neg_item_id, double learn_rate) {
    double x_uij = b_i[pos_item_id] - b_i[neg_item_id];
    for (int k = 0; k < K; ++k) {
        x_uij += R_U(user_id, k) * (R_I(pos_item_id, k) - R_I(neg_item_id, k));
    }
    double tau = sigmoid(-x_uij);

    b_i[pos_item_id] += learn_rate * (tau - biasReg * b_i[pos_item_id]);
    b_i[neg_item_id] += learn_rate * (-tau - biasReg * b_i[neg_item_id]);

    for (int k = 0; k < K; ++k) {
        double r_uk = R_U(user_id, k);
        double r_ik = R_I(pos_item_id, k);
        double r_jk = R_I(neg_item_id, k);
        R_U(user_id, k) += learn_rate * (tau * (r_ik - r_jk) - lambda * r_uk) ;
        R_I(pos_item_id, k) += learn_rate * (tau * r_uk - lambda * r_ik);
        R_I(neg_item_id, k) += learn_rate * (-tau * r_uk - lambda * r_jk);
    }
}

void BPRMF::oneiteration(double learn_rate) {
    int u, i, j;
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

        do {
             j = rand() % nItems;
        } while (pos_per_user[u].find(j) != pos_per_user[u].end());
        updateFactors(u, i, j, learn_rate);
    }    
    delete [] user_matrix;
}

void BPRMF::copyBestModel() {
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

void BPRMF::loadBestModel() {
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

void BPRMF::train(int iterations, double learn_rate) {
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
    fprintf(stderr, "\n\n <<< BPR-MF >>> Test AUC = %f, Test Std = %f\n", test, std);
    fprintf(stderr, "Best Iteration = %d, Val AUC = %f\n\n", best_iter, valid);
}

string BPRMF::toString() {
    char str[10000];
    sprintf(str, "BPR-MF__K_%d_lambda_%.4f_biasReg_%.4f", K, lambda, biasReg);
    return str;
}
