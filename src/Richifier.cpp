// Richifier.cpp

#include "Richifier.h"
using namespace leda;

#include "log.h"
using Log::out;

Richifier::Richifier( ugraph &gr, node_array<bool> *blackedOut, map<node,node> *original ) : g(gr), blackedOut(blackedOut), original(original) {
}

void Richifier::run() {
	changed = true;
	while( changed ) {
		changed = false;
		list<node> nodes;

		node v;
		nodes = g.all_nodes();
		forall( v, nodes ) {
			if( !removed(v) ) {
				lowDegreeRules( v );
			}
		}
		nodes = g.all_nodes();
		forall( v, nodes ) {
			if( !removed(v) ) {
				degreeTwoRule( v );
			}
		}

	}
}

// Islet and Twig rule. 
void Richifier::lowDegreeRules( node v ) {

	if( degree(v)==0 ) {
		remove( v );
		changed = true;
	}
	if( degree(v)==1 ) {
		list<node> N = g.adj_nodes( v );
		remove( v );
		node n;
		forall( n, N ) {
			lowDegreeRules( n );
		}
		changed = true;
	}
}
// Degree Two rule
void Richifier::degreeTwoRule( node v ) {

	if( degree(v)==2 ) {

		node n1 = g.opposite( v, g.first_adj_edge(v) );
		node n2 = g.opposite( v, g.last_adj_edge(v) );

		// maybe do not bypass because of blackedoutness:
		if( blackedOut ) {
			// if we're in the blackout-fvs case
			if( !(*blackedOut)[(*original)[v]] ) {
				// if v itself isn't blacked out
				if( (*blackedOut)[(*original)[n1]] && (*blackedOut)[(*original)[n2]] ) {
					// yet both neighbors _are_ blacked out
					return; // don't bypass.
				}
				
			}
		}

		if( n1==n2 ) {
			// bypass v by degree-two rule
			// this creates a self-loop for n1==n2
			// self-loop rule removes n1==n2
			remove( v );
			added.append( n1 );
			//if( (*blackedOut)[(*original)[n1]] ) out<Log::LOG>() << "RAMP!"; else out<Log::LOG>() << "RIght.";
			remove( n1 ); // == remove(n2)
		} else {
			remove( v );
			if( !hasDoubleEdge(n1,n2) ) makeEdge( n1, n2 );
		}

		changed = true;

	}
}

void Richifier::remove( node v ) {
	g.del_node( v );
	v->set_owner( 0 );
	changed = true;
}
bool Richifier::removed( node v ) {
	return v->owner()==0;
}
void Richifier::makeEdge( node v, node w ) {
	edge e = g.new_edge( v, w );
	if( gw.get() ) gw->update_edges();
	changed = true;
}
bool Richifier::hasDoubleEdge( node v, node w ) {
	bool seen = false;
	node n;
	forall_adj_nodes( n, v ) {
		if( n==w ) {
			if( seen ) return true;
			else seen = true;
		}
	}
	return false;
}