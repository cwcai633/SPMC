#include "corpus.hpp"

void corpus::loadData(const char* voteFile, const char* trustFile, int userMin, int itemMin) {
    nItems = 0;
    nUsers = 0;
    nVotes = 0;

    /// Note that order matters here
    loadVotes(voteFile, userMin, itemMin);
    loadTrusts(trustFile);
    
    fprintf(stderr, "\n  \"nUsers\": %d, \"nItems\": %d, \"nVotes\": %d\n", nUsers, nItems, nVotes);
}

void corpus::cleanUp() {
    for (vector<vote*>::iterator it = V.begin(); it != V.end(); it++) {
        delete *it;
    }
}

void corpus::loadVotes(const char* voteFile, int userMin, int itemMin) {
    fprintf(stderr, "  Loading votes from %s, userMin = %d, itemMin = %d  ", voteFile, userMin, itemMin);


    string uName; // User name
    string bName; // Item name
    float category; 
    float rating;
    float helpfulness;
    float voteTime; // Time rating was entered
    map<pair<int, int>, long long> voteMap;

    int nRead = 0; // Progress
    string line;

    igzstream in;
    in.open(voteFile);
    if (! in.good()) {
        fprintf(stderr, "Can't read votes from %s.\n", voteFile);
        exit(1);
    }

    // The first pass is for filtering

    set<pair<string, string> > userItemPairs;

    while (getline(in, line)) {
        stringstream ss(line);
        ss >> uName >> bName >> category >> rating >> helpfulness >> voteTime;

        nRead++;
        if (nRead % 100000 == 0) {
            fprintf(stderr, ".");
            fflush(stderr);
        }

        if (userItemPairs.find(make_pair(uName, bName)) != userItemPairs.end()) {
            continue;
        }

        if (rating < 4) {
            continue;
        }

        if (uCounts.find(uName) == uCounts.end()) {
            uCounts[uName] = 0;
        }
        if (bCounts.find(bName) == bCounts.end()) {
            bCounts[bName] = 0;
        }
        uCounts[uName]++;
        bCounts[bName]++;
                userItemPairs.insert(make_pair(uName, bName));
    }
    in.close();
    fprintf(stderr, "\n  \"nUsers\": %d\n", (int)uCounts.size());

    // Re-read
    nUsers = 0;
    nItems = 0;
    
    igzstream in2;
    in2.open(voteFile);
    if (! in2.good()) {
        fprintf(stderr, "Can't read votes from %s.\n", voteFile);
        exit(1);
    }

    ofstream userMap, itemMap;
    userMap.open("user_map_30.out");
    itemMap.open("item_map_30.out");

    nRead = 0;
    while (getline(in2, line)) {
        stringstream ss(line);
        ss >> uName >> bName >> category >> rating >> helpfulness >> voteTime;

        nRead++;
        if (nRead % 100000 == 0) {
            fprintf(stderr, ".");
            fflush(stderr);
        }

        if (uCounts[uName] < userMin or bCounts[bName] < itemMin) {
            continue;
        }

        // new item
        if (itemIds.find(bName) == itemIds.end()) {
            itemMap << bName << " " << nItems << endl;
            rItemIds[nItems] = bName;
            itemIds[bName] = nItems++;
        }
        // new user
        if (userIds.find(uName) == userIds.end()) {
            userMap << uName << " " << nUsers << endl;
            rUserIds[nUsers] = uName;
            userIds[uName] = nUsers++;
        }

        if(voteMap.find(make_pair(userIds[uName], itemIds[bName])) != voteMap.end()) {
            continue;
        }

        voteMap[make_pair(userIds[uName], itemIds[bName])] = voteTime;    
    }
    in2.close();
    userMap.close();
    itemMap.close();

    fprintf(stderr, "\n");
    generateVotes(voteMap);
}

void corpus::loadTrusts(const char* trustFile) {
    fprintf(stderr, "  Loading trusts from %s  ", trustFile);
    string line;

    igzstream in;
    in.open(trustFile);

    if (!in.good()) {
        fprintf(stderr, "Can't read votes from %s.\n", trustFile);
        exit(1);
    }

    int nRead = 0;
    string thisUser;
    string otherUser;

    while (getline(in, line)) {
        stringstream ss(line);
        ss >> thisUser >> otherUser;
        nRead++;
        if (nRead % 100000 == 0) {
            fprintf(stderr, ".");
            fflush(stderr);
        }

        if (userIds.find(thisUser) != userIds.end() && userIds.find(otherUser) != userIds.end()) {
            int thisUserId = userIds[thisUser];
            int otherUserId = userIds[otherUser];
            trustMap[thisUserId].push_back(otherUserId);
        }
    }
    in.close();

    int trust_counter = 0;
    for (auto it = trustMap.begin(); it != trustMap.end(); it++) {
        trust_counter += it->second.size();
    }
    fprintf(stderr, " \n  \"nTrusts\": %d\n", trust_counter);
}

void corpus::generateVotes(map<pair<int, int>, long long>& voteMap) {
    fprintf(stderr, "\n  Generating votes data \n");
    
    for(map<pair<int, int>, long long>::iterator it = voteMap.begin(); it != voteMap.end(); it ++) {
        vote* v = new vote();
        v->user = it->first.first;
        v->item = it->first.second;
        v->voteTime = it->second;
        V.push_back(v);
    }
    
    nVotes = V.size();
    random_shuffle(V.begin(), V.end());
}
