// util.cpp

#include "Log.h"
using Log::out;
#include "util.h"
using namespace leda;

void copyUGraph( leda::ugraph &from, leda::ugraph &to, leda::map<node,node> &original ) {

	map<node,node> replacement;

	node v;
	forall_nodes( v, from ) {
		node n = to.new_node();
		original[n] = v;
		replacement[v] = n;
	}

	edge e;
	forall_edges( e, from ) {
		to.new_edge( replacement[e->terminal(0)], replacement[e->terminal(1)] );
	}

}

node_map<bool> lcsToBfvs( graph &lcs, ugraph &bfvs, map<node,node> &original, map<node,node> *boNode ) {
	
	map<node,node> in, out;
	node_map<bool> blackedOut( bfvs );

	node v;
	forall_nodes( v, lcs ) {

		node nout = bfvs.new_node();
		original[nout] = v;
		out[v] = nout;
		blackedOut[nout] = false;

		node nin = bfvs.new_node();
		original[nin] = v; // nin will not be selected, it is blacked out. WTF?!
		in[v] = nin;
		blackedOut[nin] = true;
		if( boNode ) (*boNode)[nin] = v;

		bfvs.new_edge( nout, nin );
	}

	edge e;
	forall_edges( e, lcs ) {
		bfvs.new_edge( out[e->terminal(0)], in[e->terminal(1)] );
	}

	return blackedOut;

}


void originalize( const map<node,node> &original, list<node> &from, list<node> &to ) {
	node v;
	forall( v, from ) {
		//out<Log::LOG>() << "From "<<v<<" to "<< original[v];
		node o = original[v];
		//out<Log::LOG>() << "Converted from " << v << " to " << o;
		to.append( o );
	}
}