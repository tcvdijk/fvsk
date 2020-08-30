// BeckerBarYerudaGeiger.cpp

#include "BeckerBarYerudaGeiger.h"

#include <LEDA/graph/ugraph.h>
using namespace leda;

#include "TcdijkKernel.h"

#include "log.h"
using Log::out;

#include "util.h"

#include "Richifier.h"

#include <iostream>

BBYG::BBYG( ugraph &gr ) {

	g.clear();
	copyUGraph( gr, g, original );
	//blackedOut = node_array<bool>( g, false );
	blackedOut = node_map<bool>(g,false);
	isBFVS = false;

	c = 1.0;

	timeLimit = std::numeric_limits<float>::infinity();

}

BBYG::BBYG( graph &gr, bool kernelize ) {

	g.clear();
	blackedOut = lcsToBfvs( gr, g, original );
	if( kernelize ) {
		TcdijkKernelizer k( g, blackedOut );
		k.run();
	}
	isBFVS = true;

	c = 1.0;

	timeLimit = std::numeric_limits<float>::infinity();

}

BBYG::BBYG( graph &gr ) {

	g.clear();
	blackedOut = lcsToBfvs( gr, g, original );
	isBFVS = true;

	c = 1.0;

	timeLimit = std::numeric_limits<float>::infinity();

}

void BBYG::run() {

	float startTime = used_time();

	// initial guess
	list<node> F = singleGuess( g.number_of_nodes() );

	// initial number runs needed
	double M = numRuns( F.size() );
	out<Log::VOID>() << "Runs todo: " << M;

	double i=1;
	while( i < M ) {

		if( used_time()-startTime > timeLimit ) break;
		
		// another guess
		list<node> F2 = singleGuess( g.number_of_nodes() );

		// is it better?
		if( F2.size() < F.size() ) {
			F = F2;
			M = numRuns( F.size() ); // update max number of runs
			//out<Log::SHOW>() << "Revision, runs todo: " << M;
		}

		if( long(i)%10==0 ) out<Log::VOID>() << i << " / " << M;

		++i;
	}
	out<Log::VOID>() << "Done " << i << " runs.";

	setResult( F );

}

void BBYG::setResult( list<node> &F ) {
	fvs.clear();
	//original.statistics();
	originalize( original, F, fvs );
}



list<node> BBYG::singleGuess( int k ) {

	list<node> F;

	map<node,node> original;
	ugraph gr;
	copyUGraph( g, gr, original );

	while( !gr.empty() && F.size()<=k ) {
		
		// richify the graph
		node_array<bool> *blackedOutPtr = isBFVS? &blackedOut : 0;
		Richifier r( gr, blackedOutPtr, &original );
		r.run();
		F.conc( r.added );

		// if still not empty, guess a vertex
		if( gr.empty() ) break;

		node v;
		if( isBFVS ) {
			array<int> w( gr.number_of_nodes() );
			array<node> V( gr.number_of_nodes() );
			int i = 0;
			forall_nodes( v, gr ) {
				V[i] = v;
				if( blackedOut[original[v]] ) w[i] = 0;
				else                          w[i] = degree(v);
				++i;
			}
			random_variate selector( w );
			i = selector.generate();
			v = V[i];
			if( blackedOut[original[v]] ) out<Log::PAUSE>() << "RAMP!";
		} else {
			edge e = gr.choose_edge();
			bool b;
			rand >> b;
			if( b ) v = e->terminal(0);
			else	v = e->terminal(1);
		}

		F.append( v );
		gr.del_node( v );

	}

	list<node> result;
	originalize( original, F, result );
	return result;

}

double BBYG::numRuns( int size ) {
	return double( c * pow(4.0, size) );
}