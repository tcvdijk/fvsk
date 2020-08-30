// BeckerGeiger.cpp

#include "BeckerGeiger.h"
#include <LEDA/core/set.h> 
#include <LEDA/core/partition.h>
using namespace leda;

#include <sstream>
using std::ostringstream;

#include "log.h"
using Log::out;
using Log::toString;

namespace {
	const Log::Level debug = Log::VOID;
	const Log::Level logSelection = Log::VOID;
	const Log::Level logProcess = Log::VOID;
	const Log::Level logGreedyFVS = Log::VOID;
	const Log::Level logMinimalizeSteps = Log::VOID;
}


BeckerGeiger::BeckerGeiger( ugraph &g, int k ) : g(g), k(k), pq(g), deg(g), w(g), inGreedyFVS(g,false) {

}

void BeckerGeiger::run( list<node> &fvs, node_array<bool> *blackedOut ) {

	list<node> greedyFVS;

	node v;
	// initial weights
	forall_nodes( v, g ) {
		deg[v] = degree(v);
		//w[v] = inputWeights? (*inputWeights)[v] : 1.0;
		//if( blackedOut && !(*blackedOut)[v] ) {
		if( blackedOut==0 || !(*blackedOut)[v] ) {
			w[v] = 1.0;
			pq.insert( v, w[v]/deg[v] );
			//out<logProcess>() << "An allowed vertex: " << v;
		} else {
			w[v] = 0;
		}
	}
	// initial cleanup
	forall_nodes( v, g ) {
		cleanup( v, 0, blackedOut );
	}

	// PHASE 1: greedily construct a FVS
	while( !pq.empty() ) {

		//double prio = w.prio( w.find_min() );
		v = pq.del_min();
		if( deg[v]==0 ) {
			out<logSelection>() << "Already deleted: " << v;
			continue; // ignore if already deleted
		}

		// this vertex is selected
		out<logSelection>() << "Select: " << v;
		greedyFVS.append( v );
		inGreedyFVS[v] = true;

		double C = w[v]/deg[v];

		deg[v] = 0; // `delete' the vertex
		//gw->set_label( v, toString(deg[v]).c_str() );
		
		node n;
		forall_adj_nodes( n, v ) {
			if( deg[n]>0 ) {
				out<logProcess>() << "Neighbor of " << v << ": " << n << ". Decrease prio.";
				w[n] -= C;
				--deg[n];
				if( blackedOut==0 || !((*blackedOut)[n]) ) {
					// update prio, but only if n is in the pq
					pq.decrease_p( n, w[n]/deg[n] );
				}
				cleanup( n, C, blackedOut );
			} else {
				out<logProcess>() << "Not a neighbor of " << v << ": " << n << ". Ignore.";
			}
			//gw->set_label( n, toString(deg[n]).c_str() );
		}
			
	}

	if( logGreedyFVS == Log::PAUSE ) {
		gw->set_color( g.all_nodes(), color(white) );
		gw->set_color( greedyFVS, color(yellow) );
		out<Log::PAUSE>() << "This is the greedy FVS.";
	}


	// PHASE 2: make the FVS minimal

	// Partition the vertices of g\fvs into connected components
	node_array< partition_item > component( g );
	partition p;
	forall_nodes( v, g ) {
		component[v] = p.make_block();
	}
	edge e;
	forall_edges( e, g ) {
		node n1 = e->terminal(0), n2 = e->terminal(1);
		if( !inGreedyFVS[n1] && !inGreedyFVS[n2] ) {
			p.union_blocks( component[n1], component[n2] );
		}
	}

	forall_rev( v, greedyFVS ) {

		bool required = false;

		map< partition_item, partition_item > comps;
		node n;
		forall_adj_nodes( n, v ) {

			partition_item comp = p.find( component[n] );
			if( comps.defined(comp) ) {
				// two edges into the same conn component: we need this vertex
				required = true;
				break;
			} else {
				// another neighboring component
				comps[ comp ] = comp;
			}

		}

		if( required ) {
			fvs.append( v );
			out<logMinimalizeSteps>() << "We need " << v;

		} else {
			// apparently we didn't need this vertex.
			out<logMinimalizeSteps>() << "We don't need " << v;
	
			// but now that we don't use it, it joins conn components
			partition_item comp;
			forall( comp, comps ) {
				partition_item comp2;
				forall( comp2, comps ) {
					if( comp<comp2 ) p.union_blocks( comp, comp2 );
				}
			}
		}
		
	}

}

void BeckerGeiger::cleanup( node v, double C, node_array<bool> *blackedOut ) {

	if( deg[v]==1 ) {
	
		deg[v]=0;
		
		node n;
		forall_adj_nodes( n, v ) {
			if( deg[n]>0 ) {
				--deg[n];
				switch( deg[n] ) {
				case 0:
					// we `deleted' n: just ignore it.
					break;
				case 1:
					// n is now also a leaf.
					// don't mind updating the pq, but do recurse.
					cleanup( n, C, blackedOut );
				default:
					// did not `delete' n: update it in the pq.
					// unless n is blacked out, then it isn't in the pq.
					if( blackedOut==0 || !((*blackedOut)[n]) ) {
						w[n] -= C;
						pq.decrease_p( n, w[n]/deg[n] );
					}
				}
			}
		}

	}

}