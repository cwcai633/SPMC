#include "GBPR.hpp"
#define R_U(i, j) (r_u[(i) * K + j])
#define R_I(i, j) (r_i[(i) * K + j])
#define GROUP_SIZE 3
#define RHO 0.8

void GBPR::init() {
    b_i = new double[nItems];
    r_u = new double[nUsers * K];
    r_i = new double[nItems * K];
    group_size = GROUP_SIZE;
    rho = RHO;
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

void GBPR::cleanUp() {
    delete [] b_i;
    delete [] r_u;
    delete [] r_i;
}

double GBPR::prediction(int user, int item, int last_item, map<int, int> friend_items) {
    double res = b_i[item];
    for (int k = 0; k < K; ++k) {
        res += R_U(user, k) * R_I(item, k);
    }
    return res;
}

int GBPR::sampleUser() {
    int u;
    do {
        u = (rand() % nUsers);
    } while (pos_per_user_seq[u].size() == 0 || (int) pos_per_user_seq[u].size() == nItems);
    return u; 
}

void GBPR::updateFactors(int user_id, int pos_item_id, int neg_item_id, set<int> group, double learn_rate) {
    map<int, int> dummy_map;
    double r_gui = (1.0 - rho) * prediction(user_id, pos_item_id, 0, dummy_map);
    int group_size = group.size();
    for (set<int>::iterator it = group.begin(); it != group.end(); it++) {
        int group_mate = *it;
        r_gui += rho * prediction(group_mate, pos_item_id, 0, dummy_map) / group_size;
    }
    double r_uj = prediction(user_id, neg_item_id, 0, dummy_map);
    double r_guiuj = r_gui - r_uj;
    
    double delta = -sigmoid(-r_guiuj);
    b_i[pos_item_id] -= learn_rate * (delta + biasReg * b_i[pos_item_id]);
    b_i[neg_item_id] -= learn_rate * (-delta + biasReg * b_i[neg_item_id]);

    for (int k = 0; k < K; k++) {
        double r_uk = R_U(user_id, k);
        double r_ik = R_I(pos_item_id, k);
        double r_jk = R_I(neg_item_id, k);
        double r_gk = 0;
        for (set<int>::iterator it = group.begin(); it != group.end(); it++) {
            r_gk += R_U(*it, k) / group_size;
        }

        for (set<int>::iterator it = group.begin(); it != group.end(); it++) {
            R_U(*it, k) -= learn_rate * (delta * rho * r_ik / group_size + lambda * R_U(*it, k));
        }
        R_U(user_id, k) -= learn_rate * (delta * ((1 - rho) * r_ik + rho * r_ik / group_size - r_jk) + lambda * r_uk);
        R_I(pos_item_id, k) -= learn_rate * (delta * ((1 - rho) * r_uk + rho * r_gk) + lambda * r_ik);
        R_I(neg_item_id, k) -= learn_rate * (delta * (-r_uk) + lambda * r_jk);
    }
}

void GBPR::oneiteration(double learn_rate) {
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
        int group_size_count = 0;
        set<int> group;

        // u has to be in the group
        group.insert(u);
        group_size_count++;
        map<int, long long>::iterator it_i = pos_per_item[i].begin();
        
        while (group_size_count < group_size and group_size_count < (int)pos_per_item[i].size()) {
            int gu; 
            do {
                gu = rand() % pos_per_item[i].size();
            } while (group.find(gu) != group.end());
            advance(it_i, gu);
            group.insert(it_i->first);
            group_size_count++;
        }

        do {
             j = rand() % nItems;
        } while (pos_per_user[u].find(j) != pos_per_user[u].end());
        updateFactors(u, i, j, group, learn_rate);
    }    
    delete [] user_matrix;
}


void GBPR::copyBestModel() {
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

void GBPR::loadBestModel() {
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

void GBPR::train(int iterations, double learn_rate) {
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
    fprintf(stderr, "\n\n <<< GBPR >>> Test AUC = %f, Test Std = %f\n", test, std);
    fprintf(stderr, "Best Iteration = %d, Val AUC = %f\n\n", best_iter, valid);
}

string GBPR::toString() {
    char str[10000];
    sprintf(str, "GBPR__K_%d_lambda_%.4f_biasReg_%.4f", K, lambda, biasReg);
    return str;
}
