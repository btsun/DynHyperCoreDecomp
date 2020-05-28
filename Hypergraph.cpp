#include "Hypergraph.hpp"
#include <iostream>
using namespace std;

Hypergraph::Hypergraph() {
	nNodes = nEdges = edgeIdCounter = 0;
}

unsigned Hypergraph::insertEdge(const Hyperedge& e) {
	// Insert an edge to the hypergraph
	edge2id.insert(pair<const Hyperedge, unsigned>(e, edgeIdCounter));
	for (const Node u: e)
		eList[u].insert(edgeIdCounter);
	edgePool.push_back(e);
	// ++edgeIdCounter;
	++nEdges;
	return edgeIdCounter++;
}

void Hypergraph::deleteEdge(const Hyperedge& e) {
	// Delete an edge from the hypergraph
	auto iter = edge2id.find(e);
	for (const Node u: e)
		eList[u].erase(iter->second);
	edgePool[iter->second].clear();
	edge2id.erase(iter);
	--nEdges;
}
