#ifndef __HYPERGRAPHCOREDECOMP__
#define __HYPERGRAPHCOREDECOMP__

#include "Hypergraph.hpp"
#include <unordered_map>

class HypergraphCoreDecomp {
public:
	HypergraphCoreDecomp(const Hypergraph&);
	void solve();
	std::unordered_map<Node, unsigned> c;
private:
	const Hypergraph& h;
	std::unordered_map<Node, unsigned> deg;
	std::unordered_set<unsigned> erasedEdgeIds;
};

#endif // __HYPERGRAPHCOREDECOMP__
