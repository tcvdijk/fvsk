// HansbKernel.h

#ifndef INCLUDED_HANSBKERNEL
#define INCLUDED_HANSBKERNEL

#include <LEDA/graph/ugraph.h>
#include <Leda/core/h_array.h>

class HansbKernelizer {
public:
	
	ugraph &g;
	
	// flow network for Improvement rule
	void makeFlowNetwork();
	bool flowNetworkIncremental;
	graph flowNetwork;
	node_array<node> flowNodeIn;
	node_array<node> flowNodeOut;
	edge_map<edge> flowEdge1;
	edge_map<edge> flowEdge2;
	edge_map<edge> flowOriginalEdge;
	
	// matching graph for Flower rule
	void makeMatchingGraph( node flowerCenter );
	bool matchingGraphIncremental;
	ugraph matchingGraph;
	node_map<node> flowerWidgetNode1;
	node_map<node> flowerWidgetNode2;
	node_map<edge> flowerWidgetEdge;
	edge_map<node> flowerEdgeNode1;
	edge_map<node> flowerEdgeNode2;
	edge_map<edge> flowerEdgeEdge;
	edge_map<int> flowerWeight;

	int k;

	list<node> inFVS;
	void putInFVS( node v );

	void remove( node v );
	bool removed( node v );
	void removeEdge( edge e );
	void makeEdge( node v, node w );
	void makeDoubleEdge( node v, node w );
	bool hasDoubleEdge( node v, node w );

	bool changed;

	HansbKernelizer( ugraph &g, int k );
	HansbKernelizer( ugraph &g ); // get a valid k from the 2-approx

	void run();
	float runtime();
	float runtime_;
	bool abort_;

	// Rules
	void lowDegreeRules( node v );
	float timeLowDegree;

	void tripleEdgeRule( node v );
	float timeTripleEdge;

	void degreeTwoRule( node v );
	float timeDegreeTwo;

	void largeDoubleDegreeRule( node v );
	float timeLargeDoubleDegree;

	bool improvementRule( node v, node w );
	float timeImprovement;

	void flowerRule( node v );
	float timeFlower;

	float timeApprox;

	void abdicationRules( list<node> &approxFVS );
	h_array<node,bool> inAorB;
	node_array<int> abdiDfsVisited;
	node_array<edge> abdiEdge;
	list<node> abdiBorder;
	list<node> abdiPiece;
	void abdiDFS( node v );
	float timeAbdication;

};



#endif //ndef INCLUDED_HANSBKERNEL