#include "corpus.hpp"
#include "POP.hpp"
#include "BPRMF.hpp"
#include "FPMC.hpp"
#include "SPMC.hpp"
#include "SBPR.hpp"
#include "GBPR.hpp"
#include <time.h>
#include <string.h>

void go_POP(corpus* corp) {
    POP md(corp);
    double valid, test, std;
    md.AUC(&valid, &test, &std, false);
    fprintf(stderr, "\n\n <<< Popularity >>> Test AUC = %f, Test Std = %f\n", test, std);
}

void go_BPRMF(corpus* corp, int K, double lambda, double biasReg, int iterations, const char* corp_name) {
    BPRMF md(corp, K, lambda, biasReg);
    md.init();
    md.saveModel((string(corp_name) + "__" + md.toString() + "__init").c_str());
    md.train(iterations, 0.005);
    md.coldStart(1, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
    md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
    md.cleanUp();
}

void go_FPMC(corpus* corp, int K, double lambda, double biasReg, int iterations, const char* corp_name) {
    FPMC md(corp, K, lambda, biasReg);
    md.init();
    md.saveModel((string(corp_name) + "__" + md.toString() + "__init").c_str());
    md.train(iterations, 0.05);
    md.coldStart(1, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
    md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
    md.cleanUp();
}

void go_SPMC(corpus* corp, int K, double lambda, double biasReg, int iterations, const char* corp_name) {
    SPMC md(corp, K, lambda, biasReg);
    md.init();
    md.saveModel((string(corp_name) + "__" + md.toString() + "__init").c_str());
    md.train(iterations, 0.05);
    md.coldStart(1, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
    md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
    md.cleanUp();
}


void go_SBPR(corpus* corp, int K, double lambda, double biasReg, int iterations, const char* corp_name) {
    SBPR md(corp, K, lambda, biasReg);
    md.init();
    md.saveModel((string(corp_name) + "__" + md.toString() + "__init").c_str());
    md.train(iterations, 0.005);
    md.coldStart(1, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
    md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
    md.cleanUp();
}

void go_GBPR(corpus* corp, int K, double lambda, double biasReg, int iterations, const char* corp_name) {
    GBPR md(corp, K, lambda, biasReg);
    md.init();
    md.saveModel((string(corp_name) + "__" + md.toString() + "__init").c_str());
    md.train(iterations, 0.005);
    md.coldStart(1, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
    md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
    md.cleanUp();
}

void calculateMetrics(char* modelName, corpus* corp, int K, double lambda, double biasReg, const char* corp_name, int n) {
    if (strcmp(modelName, "--bprmf") == 0) {
        BPRMF md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.metrics(dataPath, n);
        md.cleanUp();
    } else

    if (strcmp(modelName, "--fpmc") == 0) {
        FPMC md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.metrics(dataPath, n);
        md.cleanUp();
    } else

    if (strcmp(modelName, "--sbpr") == 0) {
        SBPR md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.metrics(dataPath, n);
        md.cleanUp();
    } else

    if (strcmp(modelName, "--gbpr") == 0) {
        GBPR md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.metrics(dataPath, n);
        md.cleanUp();
    } else

    if (strcmp(modelName, "--spmc") == 0) {
        SPMC md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.metrics(dataPath, n);
        md.cleanUp();
    }
}

void calculateColdStart(char* modelName, corpus* corp, int K, double lambda, double biasReg, const char* corp_name, int n) {
    if (strcmp(modelName, "--bprmf") == 0) {
        BPRMF md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.loadModel(dataPath);
        md.loadBestModel();
        md.coldStart(n, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
        md.cleanUp();
    } else

    if (strcmp(modelName, "--fpmc") == 0) {
        FPMC md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.loadModel(dataPath);
        md.loadBestModel();
        md.coldStart(n, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
        md.cleanUp();
    } else

    if (strcmp(modelName, "--sbpr") == 0) {
        SBPR md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.loadModel(dataPath);
        md.loadBestModel();
        md.coldStart(n, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
        md.cleanUp();
    } else

    if (strcmp(modelName, "--gbpr") == 0) {
        GBPR md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.loadModel(dataPath);
        md.loadBestModel();
        md.coldStart(n, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
        md.cleanUp();
    } else

    if (strcmp(modelName, "--spmc") == 0) {
        SPMC md(corp, K, lambda, biasReg);
        md.init();
        const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
        md.loadModel(dataPath);
        md.loadBestModel();
        md.coldStart(n, (string(corp_name) + "__" + md.toString() + ".coldstart").c_str());
        md.cleanUp();
    }
}

void analyze(corpus* corp, int K, double lambda, double biasReg, const char* corp_name, int n) {
    SPMC md(corp, K, lambda, biasReg);
    md.init();
    const char* dataPath = (string(corp_name) + "__" + md.toString()).c_str();
    md.loadModel(dataPath);
    md.loadBestModel();
    md.analyze(n, (string(corp_name) + "__" + md.toString() + ".visual").c_str());
    md.cleanUp();
}

int main(int argc, char** argv) {
    srand(0);
    if (argc <= 1) {
        printf(" Please enter the name of algorithm: \n");
        printf(" --pop        POP\n");
        printf(" --bprmf      BPRMF\n");
        printf(" --fpmc       FPMC\n");
        printf(" --spmc       SPMC\n");
        printf(" --sbpr       SBPR\n");
        printf(" --gbpr       GBPR\n");
        printf(" \nTo load a model, use --load at the end.\n");
        printf(" \nTo calculate coldstart, use --coldstart at the end.\n");
        printf(" \nTo `visualize', use --analyze at the beginning.\n");
        exit(1);
    }

    if (strcmp(argv[1], "--pop") != 0 &&
         strcmp(argv[1], "--bprmf") != 0 &&
         strcmp(argv[1], "--spmc") != 0 &&
         strcmp(argv[1], "--sbpr") != 0 &&
         strcmp(argv[1], "--gbpr") != 0 &&
         strcmp(argv[1], "--fpmc") != 0 &&
         strcmp(argv[1], "--analyze") != 0) {
        printf(" No such an algorithm. \n");
        printf(" Please enter the name of algorithm: \n");
        printf(" --pop        POP\n");
        printf(" --bprmf      BPRMF\n");
        printf(" --fpmc       FPMC\n");
        printf(" --spmc      SPMC\n");
        printf(" --sbpr       SBPR\n");
        printf(" --gbpr       GBPR\n");
        printf(" Exiting...\n");
        exit(1);
    }

    if (strcmp(argv[1], "--pop") == 0 && argc != 3) {
        printf(" Parameters as following: \n");
        printf(" 1. Review file path\n");
        exit(1);
    }
    
    if (strcmp(argv[1], "--bprmf") == 0 && (argc != 9 and argc!= 10)) {
        printf(" Parameters as following: \n");
        printf(" 1. Review file path\n");
        printf(" 2. Trust file path\n");
        printf(" 3. Latent Feature Dim. (K)\n");
        printf(" 4. biasReg (regularize bias terms)\n");
        printf(" 5. lambda  (regularize general terms)\n");
        printf(" 6. Max #iter \n");
        printf(" 7. corpus name \n");
        exit(1);
    }

    if ((strcmp(argv[1], "--fpmc") == 0) && (argc != 9 and argc!= 10)) {
        printf(" Parameters as following: \n");
        printf(" 1. Review file path\n");
        printf(" 2. Trust file path\n");
        printf(" 3. Latent Feature Dim. (K)\n");
        printf(" 4. biasReg (regularize bias terms)\n");
        printf(" 5. lambda  (regularize general terms)\n");
        printf(" 6. Max #iter \n");
        printf(" 7. corpus name \n");
        exit(1);
    }

    if ((strcmp(argv[1], "--spmc") == 0) && (argc != 9 and argc!= 10)) {
        printf(" Parameters as following: \n");
        printf(" 1. Review file path\n");
        printf(" 2. Trust file path\n");
        printf(" 3. Latent Feature Dim. (K)\n");
        printf(" 4. biasReg (regularize bias terms)\n");
        printf(" 5. lambda  (regularize general terms)\n");
        printf(" 6. Max #iter \n");
        printf(" 7. corpus name \n");
        exit(1);
    }

    if ((strcmp(argv[1], "--sbpr") == 0) && (argc != 9 and argc!= 10)) {
        printf(" Parameters as following: \n");
        printf(" 1. Review file path\n");
        printf(" 2. Trust file path\n");
        printf(" 3. Latent Feature Dim. (K)\n");
        printf(" 4. biasReg (regularize bias terms)\n");
        printf(" 5. lambda  (regularize general terms)\n");
        printf(" 6. Max #iter \n");
        printf(" 7. corpus name \n");
        exit(1);
    }

    if ((strcmp(argv[1], "--gbpr") == 0) && (argc != 9 and argc!= 10)) {
        printf(" Parameters as following: \n");
        printf(" 1. Review file path\n");
        printf(" 2. Trust file path\n");
        printf(" 3. Latent Feature Dim. (K)\n");
        printf(" 4. biasReg (regularize bias terms)\n");
        printf(" 5. lambda  (regularize general terms)\n");
        printf(" 6. Max #iter \n");
        printf(" 7. corpus name \n");
        exit(1);
    }

    char* reviewPath = argv[2];
    char* trustPath = argv[3];
    fprintf(stderr, "{\n");
    fprintf(stderr, "  \"corpus\": \"%s\",\n", reviewPath);
    fprintf(stderr, "  \"trusts\": \"%s\",\n", trustPath);

    corpus corp;
    corp.loadData(reviewPath, trustPath, 4, 0);

    if (argc == 10 and strcmp(argv[9], "--load") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        //int iter = atoi(argv[7]);
        char* corpName = argv[8];

        calculateMetrics(argv[1], &corp, K, lambda, biasReg, corpName, 20);
        return 0;
    }

    if (argc == 10 and strcmp(argv[9], "--coldstart") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        //int iter = atoi(argv[7]);
        char* corpName = argv[8];

        calculateColdStart(argv[1], &corp, K, lambda, biasReg, corpName, 1);
        return 0;
    }


    if (strcmp(argv[1], "--pop") == 0) {
        go_POP(&corp);
    }

    if (strcmp(argv[1], "--bprmf") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        int iter = atoi(argv[7]);
        char* corpName = argv[8];

        go_BPRMF(&corp, K, lambda, biasReg, iter, corpName);
    }

    if (strcmp(argv[1], "--fpmc") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        int iter = atoi(argv[7]);
        char* corpName = argv[8];

        go_FPMC(&corp, K, lambda, biasReg, iter, corpName);
    }

    if (strcmp(argv[1], "--spmc") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        int iter = atoi(argv[7]);
        char* corpName = argv[8];

        go_SPMC(&corp, K, lambda, biasReg, iter, corpName);
    }

    if (strcmp(argv[1], "--sbpr") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        int iter = atoi(argv[7]);
        char* corpName = argv[8];

        go_SBPR(&corp, K, lambda, biasReg, iter, corpName);
    }

    if (strcmp(argv[1], "--gbpr") == 0) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        int iter = atoi(argv[7]);
        char* corpName = argv[8];

        go_GBPR(&corp, K, lambda, biasReg, iter, corpName);
    }

    // Analyze the trained SPMC model
    if ((strcmp(argv[1], "--analyze") == 0)) {
        int K  = atoi(argv[4]);
        double biasReg = atof(argv[5]);
        double lambda = atof(argv[6]);
        char* corpName = argv[8];
        analyze(&corp, K, lambda, biasReg, corpName, 50);
    }

    corp.cleanUp();
    fprintf(stderr, "}\n");
    return 0;
}
