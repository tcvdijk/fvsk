// Bruteforcer.h

#ifndef INCLUDED_BRUTEFORCER
#define INCLUDED_BRUTEFORCER

#include <LEDA/graph/ugraph.h>

class Bruteforcer {
public:

	leda::ugraph g;
	bool haveFVS;
	leda::list<leda::node> fvs;
	leda::map<leda::node,leda::node> original;
	leda::node_map<bool> blackedOut;
	bool ownBlackedOut;
	leda::node_array<bool> selected;

	int checkCount;

	Bruteforcer( leda::ugraph &gr, leda::node_map<bool> *blackedOut ); // feedback vertex set
	Bruteforcer( leda::graph &gr );  // loop cutset

	inline leda::node nodeForBlackOut( leda::node v ) {
		if( ownBlackedOut ) return v;
		else return original[v];
	}

	double effectiveBranchingFactor;

	float startTime, timeLimit;

	void promiseBound( int k );
	int promisedBound;

	void run();
	void recurse( leda::list<leda::node> &F, leda::node v );
	
	bool checkFVS();
	bool dfsIsCyclic( leda::node_array<bool> &visited, leda::node v, leda::node parent );

	void startTests();
	void test( leda::list<leda::node> &F );
	void endTests();

};

#endif //ndef INCLUDED_BRUTEFORCER