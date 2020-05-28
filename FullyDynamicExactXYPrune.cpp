/*
The fully dynamic algorithm of Li et al. ("Efficient Core Maintenance in Large Dynamic Graphs", TKDE 2014) generalized to hypergraphs.

[To compile]
g++ -std=c++11 -O3 FullyDynamicExactXYPrune.cpp GraphScheduler.cpp Hypergraph.cpp HypergraphCoreDecomp.cpp -Wl,--stack=167772160 -o FullyDynamicExactXYPrune -lpsapi

[To run]
FullyDynamicExactXYPrune filename

[Format of input]
The file should contain an update in each line.
Hyperedge insertion: + [node IDs] timestamp
Hyperedge deletion: - [node IDs]
So be careful that the last number of a line beginning with "+" is not an endpoint of the inserted hyperedge.
The timestamps are actually useless (for our purpose). You will need them only when you want to trace the insertion time of hyperedges.
When deleting a hyperedge, make sure that exactly the same sequence appeared in an insertion update before.
Therefore, it is recommended that in each line, the node IDs are sorted in some particular order, e.g., increasing order.

[Remark]
"-Wl,--stack=167772160" is used to set stack size. (Note that this program has a DFS.)
This program is developed on Windows. "-lpsapi" is related to reporting memory usage. You may have to remove it and "OutputMemory.cpp" (which includes a function outputMemory()) and reimplement this part when you want to compile and run the code on a different operating system.
*/

#include <cstdio>
#include <ctime>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "HypergraphCoreDecomp.hpp"
#include "OutputMemory.cpp"
using namespace std;

class FullyDynamic {
public:
	FullyDynamic(char fileName[]): scheduler(fileName) {}
    void run() {
    	FILE *ofpTime = fopen("StatFullyDynamicExactTime.txt", "a");
    	FILE *ofpMem = fopen("StatTimeMemory.txt", "a");
    	unsigned cnt = 0;
    	time_t t0 = clock();
    	while (scheduler.hasNext()) {
			EdgeUpdate edgeUpdate = scheduler.nextUpdate();
			if (edgeUpdate.updType == INS)
				insertEdge(edgeUpdate.e);
			else
				deleteEdge(edgeUpdate.e);
			++cnt;
			if (cnt % 100000 == 0) {
				fprintf(stderr, "%d...\t", cnt);
				time_t t1 = clock();
				fprintf(ofpTime, "%d\n", t1 - t0);
				fflush(ofpTime);
				t0 = t1;
				fprintf(ofpMem, "%f MB.\n", outputMemory());
				fflush(ofpMem);
				// Verify correctness
			//	HypergraphCoreDecomp hcd(h);
			//	hcd.solve();
			//	for (auto &p: c) {
			//		Node u = p.first;
			//		assert(hcd.c[u] == c[u]);
			//	}
			}
		//	for (auto& p: hcd.c) {
		//		printf("%d %d\n", p.first, p.second);
		//	}
		//	printf("---\n");
    	}
    	fclose(ofpTime);
    	fclose(ofpMem);
    }
    Hypergraph h;
private:
	GraphScheduler scheduler;
	unordered_map<Node, unsigned> c;
	unordered_set<Node> candidates;
	unordered_set<Node> demotedNodes;
	unordered_set<unsigned> visited; // IDs of visited hyperedges
	unsigned newEdgeId;
	void insertEdge(const Hyperedge& e) {
		newEdgeId = h.insertEdge(e);
		visited.clear();
		candidates.clear();
		unsigned val = INT_MAX;
		for (const Node u: e)
			val = min(val, c[u]);
		for (const Node u: e)
			if (c[u] == val)
				XYPruneColorInsert(u, val);
		recolorInsert(val);
		for (const Node u: candidates)
			++c[u];
	}
	void deleteEdge(const Hyperedge& e) { // Algorithm 9
		h.deleteEdge(e);
		visited.clear();
		candidates.clear();
		demotedNodes.clear();
		unsigned val = INT_MAX;
		for (const Node u: e)
			val = min(val, c[u]);
		for (const Node u: e) {
			if (c[u] == val) {
				// Compute X_u (line 4 of Algorithm 9)
				unsigned x = 0;
				for (const unsigned eId: h.eList[u]) {
					const Hyperedge &e = h.edgePool[eId];
					bool ok = true;
					for (const Node w: e) {
						if (c[w] < val) {
							ok = false;
							break;
						}
					}
					if (ok) ++x;
				}
				if (x < val) // Line 8, 14, 20, 30 and 34 of Algorithm 9
					YPruneColorDelete(u, val);
			}
		}
		recolorDelete(val);
		for (const Node u: demotedNodes)
			--c[u];
	}
	/*void color(Node u, unsigned val) {
		candidates.insert(u);
		for (const unsigned eId: h.eList[u]) {
			if (visited.count(eId)) continue;
			visited.insert(eId);
			const Hyperedge &e = h.edgePool[eId];
			unsigned b = INT_MAX;
			for (const Node w: e)
				b = min(b, c[w]);
			if (b < val) continue;
			for (const Node w: e)
				if (!candidates.count(w) && c[w] == val)
					color(w, val);
		}
	}*/
	void XYPruneColorInsert(Node u, unsigned val) {
		unsigned x = 0, y = 0;
		for (const unsigned eId: h.eList[u]) {
			const Hyperedge &e = h.edgePool[eId];
			unsigned b = INT_MAX;
			for (const Node w: e)
				if (w != u)
					b = min(b, c[w]);
			if (b >= val) ++x;
			if (b > val && eId != newEdgeId) ++y;
		}
		if (x > val) {
			candidates.insert(u); // Line 14 - 16 of Algorithm 11
			if (y < val) { // It seems that the condition c = 0 in line 10 of Algorithm 11 is unnecessary
				for (const unsigned eId: h.eList[u]) {
					if (visited.count(eId)) continue;
					visited.insert(eId);
					const Hyperedge &e = h.edgePool[eId];
					bool ok = true;
					for (const Node w: e) {
						if (c[w] < val) {
							ok = false;
							break;
						}
					}
					if (!ok) continue;
					for (const Node w: e)
						if (!candidates.count(w) && c[w] == val)
							XYPruneColorInsert(w, val);
				}
			}
		}
	}
	void YPruneColorDelete(Node u, unsigned val) { // Algorithm 10
		candidates.insert(u);
		unsigned y = 0;
		for (const unsigned eId: h.eList[u]) {
			const Hyperedge &e = h.edgePool[eId];
			bool ok = true;
			for (const Node w: e) {
				if (w != u && c[w] <= val) {
					ok = false;
					break;
				}
			}
			if (ok) ++y;
		}
		if (y < val) {
			for (const unsigned eId: h.eList[u]) {
				if (visited.count(eId)) continue;
				visited.insert(eId);
				const Hyperedge &e = h.edgePool[eId];
				bool ok = true;
				for (const Node w: e) {
					if (c[w] < val) {
						ok = false;
						break;
					}
				}
				if (!ok) continue;
				for (const Node w: e)
					if (!candidates.count(w) && c[w] == val)
						YPruneColorDelete(w, val);
			}
		}
	}
	void recolorInsert(unsigned val) {
		for (bool flag = true; flag;) {
			flag = false;
			auto iter = candidates.begin();
			while (iter != candidates.end()) {
				const Node u = *iter;
				unsigned x = 0;
				for (const unsigned eId: h.eList[u]) {
					const Hyperedge &e = h.edgePool[eId];
					bool ok = true;
					for (const Node v: e) { // Check whether hyperedge e can count towards x
						if (c[v] + candidates.count(v) <= val) {
							ok = false;
							break;
						}
					}
					if (ok) ++x;
				}
				if (x <= val) {
					iter = candidates.erase(iter);
					flag = true;
				} else {
					++iter;
				}
			}
		}
	}
	void recolorDelete(unsigned val) {
		for (bool flag = true; flag;) {
			flag = false;
			auto iter = candidates.begin();
			while (iter != candidates.end()) {
				const Node u = *iter;
				unsigned x = 0;
				for (const unsigned eId: h.eList[u]) {
					const Hyperedge &e = h.edgePool[eId];
					bool ok = true;
					for (const Node v: e) {
						if (c[v] + candidates.count(v) <= val) {
							ok = false;
							break;
						}
					}
					if (ok) ++x;
				}
				if (x < val) {
					demotedNodes.insert(u);
					iter = candidates.erase(iter);
					flag = true;
				} else {
					++iter;
				}
			}
		}
	}
};

int main(int argc, char **argv) {
	char *fileName = argv[1];
	FullyDynamic fullyDynamic(fileName);
	time_t t0 = clock();
	fullyDynamic.run();
	FILE *ofp = fopen("StatTimeMemory.txt", "a");
	fprintf(ofp, "%d ms.\n", clock() - t0);
	fprintf(ofp, "%f MB.\n", outputMemory());
	fclose(ofp);
	return 0;
}
