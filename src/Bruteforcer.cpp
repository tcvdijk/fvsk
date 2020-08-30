// Bruteforcer.cpp

#include "Bruteforcer.h"
#include "util.h"
using namespace leda;

#include "log.h"
using Log::out;

#include <iostream>
#include <limits>
using std::cout;
using std::endl;

namespace {
	double effectiveBF( double target, int n ) {
		// calculate effective branching factor
		double lo = 1.0;
		double hi = 2.0;
		while( hi-lo > 1e-4 ) {

			double mid = (lo + hi)/2;
			double val = pow( mid, n );
			if( val < target ) lo = mid;
			else hi = mid;

		}
		return lo;
	}
}


Bruteforcer::Bruteforcer( ugraph &gr, node_map<bool> *blackedOutIn ) {
	promisedBound = std::numeric_limits<int>::max();
	checkCount = 0;
	g.clear();
	copyUGraph( gr, g, original );
	if( blackedOutIn ) {
		blackedOut = *blackedOutIn;
		ownBlackedOut = false;
	} else {
		blackedOut = node_map<bool>( g, false );
		ownBlackedOut = true;
	}
	timeLimit = std::numeric_limits<float>::infinity();
}

Bruteforcer::Bruteforcer( graph &gr ) {
	promisedBound = std::numeric_limits<int>::max();
	checkCount = 0;
	g.clear();
	blackedOut = lcsToBfvs( gr, g, original );
	ownBlackedOut = true;
	timeLimit = std::numeric_limits<float>::infinity();
}

void Bruteforcer::run() {
	startTime = used_time();
	//cout << "=============================================================================================\n";
	startTests();
	recurse( list<node>(), g.first_node() );
	endTests();
	//out<Log::LOG>() << "\nBnB checked: " << checkCount << " out of " << (pow(2.0,g.number_of_nodes())) << " possible subsets.";
	//out<Log::LOG>() << "Which is an effective branching factor of " << effectiveBF( checkCount, g.number_of_nodes() );
	effectiveBranchingFactor = effectiveBF( checkCount, g.number_of_nodes() );
}
void Bruteforcer::recurse( list<node> &F, node v ) {

	if( used_time()-startTime > timeLimit ) return;

	// last vertex?
	if( v == nil ) {
		if( checkFVS() ) test( F );
		return;
	}

	// branch on not taking this vertex
	recurse( F, g.succ_node(v) );

	if( used_time()-startTime > timeLimit ) return;

	// if we're not too big already,
	// and this vertex is allowed,
	// branch on taking this vertex
	bool tooBig = F.size()>=promisedBound || (haveFVS && F.size()>=fvs.size());
	//bool lowDegree = degree(v)<2;
	if( !tooBig && !blackedOut[nodeForBlackOut(v)] ) {
		F.append( v );
		selected[v] = true;
		recurse( F, g.succ_node(v) );
		selected[v] = false;
		F.pop_back();
	}
}
bool Bruteforcer::checkFVS() {
	++checkCount;
	//if( checkCount%500000 == 0 ) cout << ".";
	node_array<bool> visited( g, false );
	node v;
	//cout << "Check: ";
	//forall_nodes( v, g ) {
	//	if( selected[v] ) cout << "1"; else cout << "0";
	//}
	//cout << endl;
	forall_nodes( v, g ) {
		if( !visited[v] && !selected[v] ) {
			//cout << endl << "|";
			if( dfsIsCyclic(visited,v,nil) ) {
				// a cycle! not a fvs.
				//cout << " NOT" << endl;
				return false;
			}
		}
	}
	// no cycles found: an fvs.
	//cout << " FVS!" << endl;
	return true;
}
bool Bruteforcer::dfsIsCyclic( node_array<bool> &visited, node v, node parent ) {
	bool seenParent = false;
	//cout << ".";
	node n;
	forall_adj_nodes( n, v ) {
		if( selected[n] ) {
			// this is okay.
			// don't mark it as visited.
			// don't recurse.
		} else {
			if( n==parent ) {
				if( seenParent ) {
					// a double edge to the parent!
					return true;
				} else {
					// this is the edge we've just come over.
					seenParent = true;
					continue;
				}
			}
			if( visited[n] ) {
				// seen this vertex before: cycle!
				return true;
			} else {
				// mark as visited and recurse.
				visited[n] = true;
				if( dfsIsCyclic( visited, n, v ) ) {
					// found a cycle in recursion!
					return true;
				}
			}
		}
	}
	// didn't find a cycle.
	return false;
}

void Bruteforcer::promiseBound( int k ) {
	promisedBound = k;
}

namespace {
	int myDegree( const node &v ) { return degree(v); }
}
void Bruteforcer::startTests() {
	haveFVS = false;
	fvs.clear();
	g.bucket_sort_nodes( 0, g.number_of_edges(), myDegree );
	selected = node_array<bool>( g, false );
}
void Bruteforcer::test( list<node> &F ) {
	//cout << "F " << F.size() << ", fvs " << fvs.size();
	if( !haveFVS || F.size() < fvs.size() ) {
		haveFVS = true;
		//cout << "Accept " << fvs.size() << " -> " << F.size() << endl;
		fvs = F;
		//cout << " " << fvs.size() << " ";
	} else {
		//cout << "Reject, have " << fvs.size() << endl;
	}
}
void Bruteforcer::endTests() {
	list<node> F = fvs;
	fvs.clear();
	originalize( original, F, fvs );
}