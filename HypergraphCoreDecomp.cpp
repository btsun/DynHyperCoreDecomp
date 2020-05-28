#include "HypergraphCoreDecomp.hpp"
#include <set>
#include <algorithm>
#include <cassert>
using namespace std;

HypergraphCoreDecomp::HypergraphCoreDecomp(const Hypergraph& h): h(h) {}

void HypergraphCoreDecomp::solve() {
	set<pair<unsigned, Node>> S;
	unsigned ans = 0;
	for (auto& p: h.eList) {
		deg[p.first] = p.second.size();
		S.insert(make_pair(deg[p.first], p.first));
	}
	while (!S.empty()) {
		pair<unsigned, Node> p = *S.begin();
		S.erase(S.begin());
		ans = max(ans, p.first);
		assert(p.first == deg[p.second]);
		c[p.second] = ans;
		for (const unsigned eId: h.eList.at(p.second)) {
			if (!erasedEdgeIds.count(eId)) {
				erasedEdgeIds.insert(eId);
				const Hyperedge& e = h.edgePool[eId];
				for (const Node v: e) {
					if (S.erase(make_pair(deg[v], v))) {
						--deg[v];
						S.insert(make_pair(deg[v], v));
					}
				}
			}
		}
	}
}
