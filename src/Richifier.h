// Richifier.h

#ifndef INCLUDED_RICHIFIER
#define INCLUDED_RICHIFIER

#include <LEDA/graph/ugraph.h>

class Richifier {
public:

	leda::ugraph &g;
	leda::node_array<bool> *blackedOut;
	leda::map<leda::node,leda::node> *original;

	leda::list<leda::node> added;
	
	Richifier( leda::ugraph &gr, leda::node_array<bool> *blackedOut = 0, leda::map<leda::node,leda::node> *original = 0 );

	void run();

	bool changed;
	void remove( leda::node v );
	bool removed( leda::node v );
	void lowDegreeRules( leda::node v );
	void degreeTwoRule( leda::node v );
	void makeEdge( leda::node v, leda::node w );
	bool hasDoubleEdge( leda::node v, leda::node w );

};



#endif //ndef INCLUDED_BECKERBARYERUDAGEIGER