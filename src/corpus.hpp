#pragma once

#include "common.hpp"

class corpus
{
public:
	corpus() {}
	~corpus() {}

	vector<vote*> V; // vote

	int nUsers; // Number of users
	int nItems; // Number of items
	int nVotes; // Number of ratings

	map<string, int> userIds; // Maps a user's string-valued ID to an integer
	map<string, int> itemIds; // Maps an item's string-valued ID to an integer

	map<int, string> rUserIds; // Inverse of the above maps
	map<int, string> rItemIds;
	map<int, vector<int> > trustMap;

	/* For pre-load */
	map<string, int> uCounts;
	map<string, int> bCounts;

	void loadData(const char* voteFile, const char* trustFile, int userMin, int itemMin);
        void loadTrusts(const char* trustFile);
	void cleanUp();

private:
	void loadVotes(const char* voteFile, int userMin, int itemMin);
	void generateVotes(map<pair<int, int>, long long>& voteMap);
};
