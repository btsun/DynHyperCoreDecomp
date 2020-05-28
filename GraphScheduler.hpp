#ifndef __GRAPHSCHEDULER__
#define __GRAPHSCHEDULER__

#include <vector>
#include <fstream>
#include "Hypergraph.hpp"

enum Update {INS, DEL};

struct EdgeUpdate {
	Hyperedge e;
	int timestamp;
	Update updType;
};

class GraphScheduler {
public:
	GraphScheduler(const char fileName[]);
	EdgeUpdate nextUpdate();
	inline bool hasNext() {
		return position < updates.size();
	}
	unsigned numberOfNodes, maxDegree;
private:
	void load();
	std::vector<EdgeUpdate> updates;
	std::ifstream fin;
	unsigned position;
};
#endif
