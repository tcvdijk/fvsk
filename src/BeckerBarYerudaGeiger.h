// BBYG.h

#ifndef INCLUDED_BECKERBARYERUDAGEIGER
#define INCLUDED_BECKERBARYERUDAGEIGER

#include <LEDA/graph/ugraph.h>
#include <LEDA/core/random_variate.h>

class BBYG {
public:

	leda::ugraph g;
	leda::map<leda::node,leda::node> original;
	leda::node_map<bool> blackedOut; // was array!
	bool isBFVS;

	float timeLimit;

	leda::list<leda::node> fvs;
	double c;

	BBYG( leda::ugraph &gr ); // feedback vertex set
	BBYG( leda::graph &gr, bool kernelize );
	BBYG( leda::graph &gr );  // loop cutset

	void run();

	void setResult( leda::list<leda::node> &F );

	leda::list<leda::node> singleGuess( int k );
	double numRuns( int size );

	leda::random_source rand; 

};



#endif //ndef INCLUDED_BECKERBARYERUDAGEIGER