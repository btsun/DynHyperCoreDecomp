/*
The insertion-only algorithm of Zhang et al. ("A Fast Order-Based Approach for Core Maintenance", ICDE 2017)
This program only works for normal graphs.

[To compile]
g++ -std=c++11 -O3 IncrementalExactOrderBasedNormal.cpp GraphScheduler.cpp Hypergraph.cpp HypergraphCoreDecomp.cpp -o IncrementalExactOrderBasedNormal -lpsapi

[To run]
IncrementalExactOrderBasedNormal filename

[Format of input]
The file should contain an update in each line.
Hyperedge insertion: + [node IDs] timestamp
Hyperedge deletion: - [node IDs]
So be careful that the last number of a line beginning with "+" is not an endpoint of the inserted hyperedge.
The timestamps are actually useless (for our purpose). You will need them only when you want to trace the insertion time of hyperedges.
When deleting a hyperedge, make sure that exactly the same sequence appeared in an insertion update before.
Therefore, it is recommended that in each line, the node IDs are sorted in some particular order, e.g. increasing order.

[Remark]
This program is developed on Windows. "-lpsapi" is related to reporting memory usage. You may have to remove it and "OutputMemory.cpp" (which includes a function outputMemory()) and reimplement this part when you want to compile and run the code on a different operating system.
*/

#include <cstdio>
#include <ctime>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <set>
#include "Hypergraph.hpp"
#include "GraphScheduler.hpp"
#include "HypergraphCoreDecomp.hpp"
#include "OutputMemory.cpp"
#include "Splay.cpp"
using namespace std;

class FullyDynamic {
public:
	FullyDynamic(char fileName[]): scheduler(fileName), O(1), A(1) {}
		void run() {
			FILE *ofpTime = fopen("StatIncrementalExactOrderBasedTime.txt", "a");
			FILE *ofpMem = fopen("StatTimeMemory.txt", "a");
			unsigned cnt = 0;
			time_t t0 = clock();
			while (scheduler.hasNext()) {
				EdgeUpdate edgeUpdate = scheduler.nextUpdate();
				assert(edgeUpdate.updType == INS);
				insertEdge(edgeUpdate.e);
				++cnt;
				if (cnt % 100000 == 0) {
					fprintf(stderr, "%d...\t", cnt);
					time_t t1 = clock();
					fprintf(ofpTime, "%d\n", t1 - t0);
					fflush(ofpTime);
					t0 = t1;
				//	fprintf(ofpMem, "%f MB.\n", outputMemory());
					fflush(ofpMem);
					// Verify correctness
				/*	HypergraphCoreDecomp hcd(h);
					hcd.solve();
					for (auto &p: c) {
						Node u = p.first;
						if (hcd.c[u] != c[u])
							cerr << "c[" << u << "], std = " << hcd.c[u] << ", mine = " << c[u] << endl;
						assert(hcd.c[u] == c[u]);
					}*/
				}
			}
			fclose(ofpTime);
			fclose(ofpMem);
		}
		Hypergraph h;
private:
	GraphScheduler scheduler;
	unordered_map<Node, unsigned> c;
	vector<list<Node>> O;
	unordered_map<Node, list<Node>::iterator> iterToO;
	list<Node> VC;
	unordered_map<Node, list<Node>::iterator> iterToVC;
	vector<SplayTree<Node>> A; // Data structure A
	unordered_map<Node, SplayNode<Node> *> pointerToA;
	set<pair<int, Node>> B; // Data structure B
	unordered_map<Node, unsigned> degPlus, degStar;
	vector<pair<Node, Node>> changesInA; // Record how we should modify A to make it consistent with the new order
	void insertEdge(const Hyperedge& e) { // Algorithm 2: OrderInsert
		h.insertEdge(e);
		for (const Node u: e) {
			if (pointerToA[u] == NULL) {
				// Append the new node to O[0], insert it to data structure A[0] and record the pointer to the new node in A[0]
				iterToO[u] = O[0].emplace(O[0].begin(), u);
				pointerToA[u] = new SplayNode<Node>(u);
				A[0].insertAfter(pointerToA[u], A[0].begindummy);
			}
		}
		Node u = e[0], v = e[1];
		unsigned K = min(c[u], c[v]);
		if (c[u] > c[v] || (c[u] == c[v] && A[c[u]].rank(pointerToA[u]) > A[c[v]].rank(pointerToA[v])))
			u = v;
		// Let u be the node with least order
		++degPlus[u];
		if (degPlus[u] <= K) return;
		B.insert(make_pair(A[K].rank(pointerToA[u]), u));
		for (list<Node>::iterator iter = O[K].begin(); iter != O[K].end();) {
			v = *iter;
			if (degStar[v] + degPlus[v] > K) { // Case 1
				iter = O[K].erase(iter);
				iterToVC[v] = VC.emplace(VC.end(), v);
				for (const unsigned eId: h.eList[v]) {
					const Hyperedge& e = h.edgePool[eId];
					const Node w = e[0] ^ e[1] ^ v; // Be careful! Use XOR here.
					if (c[w] == K && A[K].rank(pointerToA[v]) < A[K].rank(pointerToA[w])) {
						if (degStar[w] == 0)
							B.insert(make_pair(A[K].rank(pointerToA[w]), w));
						++degStar[w];
					}
				}
			} else if (degStar[v] == 0) { // Case 2a
				if (B.empty())
					break;
				const Node w = B.begin()->second;
				iter = iterToO[w];
			} else { // Case 2b
				degPlus[v] += degStar[v];
				degStar[v] = 0;
				// Because emplace() adds an element before the given iterator,
				// unlike the pseudocode in the paper, we perform ++iter before removeCandidates().
				SplayNode<Node> *currPosInA = pointerToA[v];
				++iter;
				removeCandidates(iter, v, K, currPosInA);
			}
			if (!B.empty() && A[K].rank(pointerToA[v]) >= B.begin()->first) {
				B.erase(B.begin());
			}
		}
		for (const Node w: VC) {
			degStar[w] = 0;
			++c[w];
		}
		for (list<Node>::reverse_iterator iter = VC.rbegin(); iter != VC.rend(); ++iter) {
			const Node w = *iter;
			if (O.size() <= K + 1) O.push_back(list<Node>());
			iterToO[w] = O[K + 1].emplace(O[K + 1].begin(), w);
			// Modify data structure A
			A[K].del(pointerToA[w]);
			if (A.size() <= K + 1) A.push_back(SplayTree<Node>());
			A[K + 1].insertAfter(pointerToA[w], A[K + 1].begindummy);
		}
		// Modify A to make it consistent with the new order
		for (vector<pair<Node, Node>>::iterator iter = changesInA.begin(); iter != changesInA.end(); ++iter) {
			A[K].del(pointerToA[iter->first]);
			A[K].insertAfter(pointerToA[iter->first], pointerToA[iter->second]);
		}
		changesInA.clear();
		VC.clear();
		iterToVC.clear();
	}
	void removeCandidates(const list<Node>::iterator &iter, const Node w, const unsigned K, SplayNode<Node> *currPosInA) { // Algorithm 3: RomoveCandidates
		queue<Node> Q;
		unordered_set<Node> Qnodes;
		for (const unsigned eId: h.eList[w]) {
			const Hyperedge &e = h.edgePool[eId];
			const Node w2 = e[0] ^ e[1] ^ w; // Be careful! Use XOR here.
			if (iterToVC.count(w2)) {
				--degPlus[w2];
				if (degPlus[w2] + degStar[w2] <= K) {
					Q.push(w2);
					Qnodes.insert(w2);
				}
			}
		}
		while (!Q.empty()) {
			const Node w2 = Q.front();
			Q.pop();
			Qnodes.erase(w2);
			degPlus[w2] += degStar[w2];
			degStar[w2] = 0;
			VC.erase(iterToVC[w2]);
			iterToVC.erase(w2);
			iterToO[w2] = O[K].emplace(iter, w2);
			// Modify data structure A
			changesInA.push_back(make_pair(w2, *prev(iterToO[w2])));
			for (const unsigned eId: h.eList[w2]) {
				const Hyperedge &e = h.edgePool[eId];
				const Node w3 = e[0] ^ e[1] ^ w2; // Be careful! Use XOR here.
				if (c[w3] == K) {
					if (A[K].rank(pointerToA[w]) < A[K].rank(pointerToA[w3])) {
						--degStar[w3];
						if (degStar[w3] == 0) {
							B.erase(make_pair(A[K].rank(pointerToA[w3]), w3));
						}
					} else if (iterToVC.count(w3)) {
						if (A[K].rank(pointerToA[w2]) < A[K].rank(pointerToA[w3]))
							--degStar[w3];
						else
							--degPlus[w3];
						if (degPlus[w3] + degStar[w3] <= K && !Qnodes.count(w3)) {
							Q.push(w3);
							Qnodes.insert(w3);
						}
					}
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
//	FILE *ofp = fopen("StatTimeMemory.txt", "a");
//	fprintf(ofp, "%d ms.\n", clock() - t0);
//	fprintf(ofp, "%f MB.\n", outputMemory());
//	fclose(ofp);
//	return 0;
}
