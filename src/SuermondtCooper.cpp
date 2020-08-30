// SuermondtCooper.cpp

#include "SuermondtCooper.h"
using namespace leda;

bool suermondtCooper( graph &g, list<node> &fvs ) {

	while( !g.empty() ) {
	
		node v;
		bool selected = false;
		
		// any v with degree<=1
		bool found = false;
		forall_nodes( v, g ) {
			if( degree(v)<=1 ) {
				found = true;
				break;
			}		
		}
		
		// else, any v with max degree among vertices with indeg<=1 
		if( !found ) {
		
			node maxNode;
			int maxSeen = -1;
			forall_nodes( v, g ) {
				if( indeg(v)<=1 && degree(v)>maxSeen ) {
					maxSeen = degree(v);
					maxNode = v;
				}
			}
			v = maxNode;
			selected = true;
			if( maxSeen>1 ) found = true;
		}
		
		if( !found ) return false;
		
		if( selected ) fvs.append( v );
		g.del_node( v );
	
	}
	
	return true;
	
}