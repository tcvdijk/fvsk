// BeckerGeiger.h

#ifndef INCLUDED_BECKERGEIGER
#define INCLUDED_BECKERGEIGER

#include <LEDA/graph/ugraph.h>
#include <LEDA/graph/node_pq.h>

class BeckerGeiger {
public:

	leda::ugraph &g;
	leda::node_array<int> deg;
	leda::node_array<double> w;
	leda::node_array<bool> inGreedyFVS;
	leda::node_pq<double> pq;
	int k;

	BeckerGeiger( leda::ugraph &g, int k );

	void run( leda::list<leda::node> &fvs, leda::node_array<bool> *inputWeights = 0 );

	void cleanup( leda::node v, double C, leda::node_array<bool> *inputWeights );

};


#endif //ndef INCLUDED_BECKERGEIGER