/*
A fully dynamic hypergraph approximate k-core maintenance algorithm, which is equivalent to our (round-indexing) fully dynamic algorithm but uses the threshold-indexing approach.

[To compile]
g++ -std=c++11 -O3 FullyDynamicThresholdIndexing.cpp GraphScheduler.cpp Hypergraph.cpp -o FullyDynamicThresholdIndexing -lpsapi

[To run]
FullyDynamicThresholdIndexing epsilon lambda alpha filename

[Format of input]
The file should contain an update in each line.
Hyperedge insertion: + [node IDs] timestamp
Hyperedge deletion: - [node IDs]
So be careful that the last number of a line beginning with "+" is not an endpoint of the inserted hyperedge.
The timestamps are actually useless (for our purpose). You will need them only when you want to trace the insertion time of hyperedges.
When deleting a hyperedge, make sure that exactly the same sequence appeared in an insertion update before.
Therefore, it is recommended that in each line, the node IDs are sorted in some particular order, e.g., increasing order.

[Remark]
This program is developed on Windows. "-lpsapi" is related to reporting memory usage. You may have to remove it and "OutputMemory.cpp" (which includes a function outputMemory()) and reimplement this part when you want to compile and run the code on a different operating system.
*/

#include <cmath>
#include <ctime>
#include <iostream>
#include <unordered_set>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "OutputMemory.cpp"
using namespace std;

class FullyDynamic {
public:
	FullyDynamic(double epsilon, double lambda, double alpha, char fileName[]):
		epsilon(epsilon), lambda(lambda), alpha(alpha), scheduler(fileName) {
			initialize();
	}
	void run() {
		int cnt = 0;
		while (scheduler.hasNext()) {
			EdgeUpdate edgeUpdate = scheduler.nextUpdate();
			if (edgeUpdate.updType == INS)
				insertEdge(edgeUpdate.e);
			else
				deleteEdge(edgeUpdate.e);
			++cnt;
			if (cnt % 100000 == 0) {
				cerr << cnt << "...\t";
			}
		}
		cerr << thresholds.size() << "thresholds. Max threshold = " << thresholds.back() << "." << endl;
	}
	unsigned getApproxCoreVal(Node u) {
		int p = 0, q = thresholds.size();
		while (p + 1 < q) {
			int m = (p + q) >> 1;
			if (l[m][u] < tau)
				q = m;
			else
				p = m;
		}
		return thresholds[p];
	}
	void debug() {
		for (int i = 1; i <= 100; ++i)
			cerr << getApproxCoreVal(i) << ' ';
		cerr << endl;
	}
private:
	double epsilon, lambda, alpha;
	GraphScheduler scheduler;
	Hypergraph h;
	int tau;
	vector<unordered_map<Node, int>> l, b, a;
	vector<unsigned> thresholds;
	const bool DYNAMIC_THRESHOLDS = true;
	void initialize() {
		tau = ceil(0.15 * log(scheduler.numberOfNodes) / log(1.0 + epsilon));
		thresholds.push_back(0);
		if (!DYNAMIC_THRESHOLDS)
			while (thresholds.back() < scheduler.maxDegree)
				thresholds.push_back(max((unsigned)(thresholds.back() * (1.0 + lambda)), thresholds.back() + 1));
		l.resize(thresholds.size());
		b.resize(thresholds.size());
		a.resize(thresholds.size());
	}
	void insertEdge(const Hyperedge& e) {
		h.insertEdge(e);
		unordered_set<Node> bad;
		int originalSize = thresholds.size();
		for (int i = 0; i < thresholds.size(); ++i) {
			unsigned beta = thresholds[i];
			if (i < originalSize) {
				int l_e = INT_MAX;
				for (const Node u: e)
					l_e = min(l_e, l[i][u]);
				for (const Node u: e) {
					if (l_e >= l[i][u]) ++b[i][u], bad.insert(u);
					if (l_e >= l[i][u] - 1) ++a[i][u];
				}
			}
			else {
				for (auto& p: h.eList) {
					const Node u = p.first;
					b[i][u] = p.second.size();
					bad.insert(u);
				}
			}
			while (!bad.empty()) {
				const Node u = *bad.begin();
				if (l[i][u] < tau && b[i][u] >= (unsigned)(alpha * beta)) {
					promote(i, u, bad);
					if (DYNAMIC_THRESHOLDS)
						if (i == thresholds.size() - 1 && l[i][u] == tau) {
							thresholds.push_back(max((unsigned)(thresholds.back() * (1.0 + lambda)), thresholds.back() + 1));
							l.resize(thresholds.size());
							b.resize(thresholds.size());
							a.resize(thresholds.size());
						}
				}
				else
					bad.erase(u);
			}
		}
	}
	void promote(const unsigned i, const Node u, unordered_set<Node>& bad) {
		int old_l_u = l[i][u];
		++l[i][u];
		updateBAndA(i, u);
		for (const unsigned eId: h.eList[u]) {
			const Hyperedge& e = h.edgePool[eId];
			int old_l_e = INT_MAX;
			for (const Node v: e)
				if (v != u)
					old_l_e = min(old_l_e, l[i][v]);
			int new_l_e = min(old_l_e, l[i][u]);
			old_l_e = min(old_l_e, old_l_u);
			if (new_l_e == old_l_e) continue;
			for (const Node v: e) {
				if (old_l_e < l[i][v] && l[i][v] <= new_l_e)
					++b[i][v], bad.insert(v);
				if (old_l_e < l[i][v] - 1 && l[i][v] - 1 <= new_l_e)
					++a[i][v];
			}
		}
	}
	void deleteEdge(const Hyperedge& e) {
		h.deleteEdge(e);
		unordered_set<Node> bad;
		for (int i = 0; i < thresholds.size(); ++i) {
			unsigned beta = thresholds[i];
			int l_e = INT_MAX;
			for (const Node u: e)
				l_e = min(l_e, l[i][u]);
			for (const Node u: e) {
				if (l_e >= l[i][u]) --b[i][u];
				if (l_e >= l[i][u] - 1) --a[i][u], bad.insert(u);
			}
			while (!bad.empty()) {
				const Node u = *bad.begin();
				if (l[i][u] > 0 && a[i][u] < beta)
					demote(i, u, bad);
				else
					bad.erase(u);
			}
		}
	}
	void demote(const unsigned i, const Node u, unordered_set<Node>& bad) {
		int old_l_u = l[i][u];
		--l[i][u];
		updateBAndA(i, u);
		for (const unsigned eId: h.eList[u]) {
			const Hyperedge& e = h.edgePool[eId];
			int old_l_e = INT_MAX;
			for (const Node v: e)
				if (v != u)
					old_l_e = min(old_l_e, l[i][v]);
			int new_l_e = min(old_l_e, l[i][u]);
			old_l_e = min(old_l_e, old_l_u);
			if (new_l_e == old_l_e) continue;
			for (const Node v: e) {
				if (new_l_e < l[i][v] && l[i][v] <= old_l_e)
					--b[i][v];
				if (new_l_e < l[i][v] - 1 && l[i][v] - 1 <= old_l_e)
					--a[i][v], bad.insert(v);
			}
		}
	}
	void updateBAndA(unsigned i, Node u) {
		b[i][u] = a[i][u] = 0;
		for (const unsigned eId: h.eList[u]) {
			const Hyperedge& e = h.edgePool[eId];
			int l_e = INT_MAX;
			for (const Node v: e)
				l_e = min(l_e, l[i][v]);
			if (l_e >= l[i][u]) ++b[i][u];
			if (l_e >= l[i][u] - 1) ++a[i][u];
		}
	}
};

int main(int argc, char** argv) {
	double epsilon = atof(argv[1]);
	double lambda = atof(argv[2]);
	double alpha = atof(argv[3]);
	char* fileName = argv[4];
	FullyDynamic fullyDynamic(epsilon, lambda, alpha, fileName);
	time_t t = clock();
	fullyDynamic.run();
	t = clock() - t;
	FILE *ofp = fopen("StatTimeMemory.txt", "a");
	fprintf(ofp, "Thres\t%.2f\t%f\t", epsilon, t / 1000.0);
	cerr << t << " ms." << endl;
	fullyDynamic.debug();
	double mem = outputMemory();
	fprintf(ofp, "%f\n", mem);
	fclose(ofp);
	return 0;
}
