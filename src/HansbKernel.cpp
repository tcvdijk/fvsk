// HansbKernel.cpp

#include <LEDA/core/partition.h>
#include <LEDA/core/map.h> 
#include <LEDA/graph/ugraph.h>
#include <LEDA/graph/max_flow.h>
#include <LEDA/graph/mw_matching.h>
using namespace leda;

#include "log.h"
using Log::out;

#include "HansbKernel.h"
#include "BeckerGeiger.h"
#include "Timer.h"

namespace {

	const Log::Level debug = Log::LOG;

	// High level
	const Log::Level logSteps = Log::VOID;
	const Log::Level logTimeStats = Log::VOID;
	const Log::Level logResult = Log::VOID;

	const bool showTwoApprox = false;

	// Individual rule hits
	const Log::Level logIslet = Log::VOID;
	const Log::Level logTwig = Log::VOID;
	const Log::Level logTripleEdge = Log::VOID;
	const Log::Level logLargeDoubleDegree = Log::VOID;
	const Log::Level logDegreeTwo = Log::VOID;	
	const Log::Level logImprovement = Log::VOID;
	const Log::Level logFlower = Log::VOID;
	const Log::Level logAbdication = Log::VOID;

	// Really small steps
	const Log::Level logAbdicationPieces = Log::VOID;
	const Log::Level logAbdicationSteps = Log::VOID;

}

HansbKernelizer::HansbKernelizer( ugraph &g, int k ) : 
		g(g),
		k(k),
		flowNodeIn(g),
		flowNodeOut(g),
		flowEdge1(g),
		flowEdge2(g),
		flowOriginalEdge(flowNetwork) {

	timeLowDegree = 0;
	timeTripleEdge = 0;
	timeDegreeTwo = 0;
	timeLargeDoubleDegree = 0;
	timeImprovement = 0;
	timeFlower = 0;
	timeAbdication = 0;
	timeApprox = 0;
	makeFlowNetwork();

};

HansbKernelizer::HansbKernelizer( ugraph &g ) :
		g(g),
		flowNodeIn(g),
		flowNodeOut(g),
		flowEdge1(g),
		flowEdge2(g),
		flowOriginalEdge(flowNetwork) {

	list<node> approxFVS;
	BeckerGeiger approx( g, g.number_of_nodes() );
	approx.run( approxFVS );
	k = approxFVS.size();

	timeLowDegree = 0;
	timeTripleEdge = 0;
	timeDegreeTwo = 0;
	timeLargeDoubleDegree = 0;
	timeImprovement = 0;
	timeFlower = 0;
	timeAbdication = 0;
	timeApprox = 0;
	makeFlowNetwork();

};

void HansbKernelizer::makeFlowNetwork() {
	node v;
	forall_nodes( v, g ) {
		flowNodeIn[v] = flowNetwork.new_node( );
		flowNodeOut[v] = flowNetwork.new_node();
		flowNetwork.new_edge( flowNodeIn[v], flowNodeOut[v] );
	}

	edge e;
	forall_edges( e, g ) {
		node a = e->terminal(0);
		node b = e->terminal(1);
		flowEdge1[e] = flowNetwork.new_edge( flowNodeOut[a], flowNodeIn[b] );
		flowEdge2[e] = flowNetwork.new_edge( flowNodeOut[b], flowNodeIn[a] );
		flowOriginalEdge[ flowEdge1[e] ] = e;
		flowOriginalEdge[ flowEdge2[e] ] = e;
	}
}

void HansbKernelizer::makeMatchingGraph( node flowerCenter = 0 ) {
	
	int vertexUsePenalty = 1;
	int vertexDisusePenalty = 0;

	matchingGraph = ugraph();

	flowerWidgetNode1 = node_map<node>( g );
	flowerWidgetNode2 = node_map<node>( g );
	flowerWidgetEdge = node_map<edge>( g );
	flowerEdgeNode1 = edge_map<node>( g );
	flowerEdgeNode2 = edge_map<node>( g );
	flowerEdgeEdge = edge_map<edge>( g );
	flowerWeight = edge_map<int>( matchingGraph );

	list<node> centerNodes;

	if( flowerCenter ) {
		for( int i=0; i<(2*k+2); ++i ) {
			centerNodes.append( matchingGraph.new_node() );
		}
	}

	node v;
	forall_nodes( v, g ) {
		if( v==flowerCenter ) continue;
		flowerWidgetNode1[v] = matchingGraph.new_node();
		flowerWidgetNode2[v] = matchingGraph.new_node();
		flowerWidgetEdge[v] = matchingGraph.new_edge( flowerWidgetNode1[v], flowerWidgetNode2[v] );
		flowerWeight[flowerWidgetEdge[v]] = 8 * vertexDisusePenalty;
	}

	edge e;
	forall_edges( e, g ) {
		// make frob
		flowerEdgeNode1[e] = matchingGraph.new_node();
		flowerEdgeNode2[e] = matchingGraph.new_node();
		flowerEdgeEdge[e] = matchingGraph.new_edge( flowerEdgeNode1[e], flowerEdgeNode2[e] );
		flowerWeight[flowerEdgeEdge[e]] = 0;
		// connect to terminal one
		node n = e->terminal(0);
		if( n==flowerCenter ) {
			node c;
			forall( c, centerNodes ) flowerWeight[ matchingGraph.new_edge( c, flowerEdgeNode1[e] ) ] = 0;
		} else {
			flowerWeight[ matchingGraph.new_edge( flowerWidgetNode1[n], flowerEdgeNode1[e] ) ] = 4*vertexUsePenalty;
			flowerWeight[ matchingGraph.new_edge( flowerWidgetNode2[n], flowerEdgeNode1[e] ) ] = 4*vertexUsePenalty;
		}
		// connect to terminal two
		n = e->terminal(1);
		if( n==flowerCenter ) {
			node c;
			forall( c, centerNodes ) flowerWeight[ matchingGraph.new_edge( c, flowerEdgeNode2[e] ) ] = 0;
		} else {
			flowerWeight[ matchingGraph.new_edge( flowerWidgetNode1[n], flowerEdgeNode2[e] ) ] = 4*vertexUsePenalty;
			flowerWeight[ matchingGraph.new_edge( flowerWidgetNode2[n], flowerEdgeNode2[e] ) ] = 4*vertexUsePenalty;
		}
	}
}


// Islet and Twig rule. 
void HansbKernelizer::lowDegreeRules( node v ) {
	//float t = used_time();

	if( degree(v)==0 ) {
		Log::out<logIslet>() << "Islet: " << v;
		remove( v );
		changed = true;
	}
	if( degree(v)==1 ) {
		Log::out<logTwig>() << "Twig: " << v;
		list<node> N = g.adj_nodes( v );
		remove( v );
		node n;
		forall( n, N ) {
			lowDegreeRules( n );
		}
		changed = true;
	}

	//timeLowDegree += used_time(t);
}

// Triple Edge rule
void HansbKernelizer::tripleEdgeRule( node v ) {
	//float t = used_time();

	node_map< int > numEdges( g );
	list<edge> toDelete;
	edge e;
	forall_adj_edges( e, v ) {
		node n = g.opposite( v, e );
		++numEdges[n];
		if( numEdges[n]>2 ) {
			removeEdge( e );
			changed = true;
			Log::out<logTripleEdge>() << "Triple Edge for " << v << " and " << n;
		}
	}

	//timeTripleEdge += used_time(t);
}

// Degree Two rule
void HansbKernelizer::degreeTwoRule( node v ) {
	//float t = used_time();
	
	if( degree(v)==2 ) {

		node n1 = g.opposite( v, g.first_adj_edge(v) );
		node n2 = g.opposite( v, g.last_adj_edge(v) );

		if( n1==n2 ) {
			// bypass v by degree-two rule
			// this creates a self-loop for n1==n2
			// self-loop rule removes n1==n2
			Log::out<logDegreeTwo>() << "Degree two: " << v << ", and will create Self-Loop for " << n1;
			remove( v );
			putInFVS( n1 );
			remove( n1 ); // == remove(n2)
		} else {
			// bypass
			Log::out<logDegreeTwo>() << "Degree two: " << v;
			remove( v );
			//g.new_edge( n1, n2 );
			if( !hasDoubleEdge(n1,n2) ) makeEdge( n1, n2 );
		}

		changed = true;

	}

	//timeDegreeTwo += used_time(t);
}

void HansbKernelizer::largeDoubleDegreeRule( node v ) {
	//float t = used_time();

	int doubleNeighbors = 0;
	node_map< int > numEdges( g );
	list<edge> toDelete;
	edge e;
	forall_adj_edges( e, v ) {
		node n = g.opposite( v, e );
		++numEdges[n];
		if( numEdges[n]==2 ) {
			++doubleNeighbors;
			if( doubleNeighbors>k ) break;
		}
	}

	if( doubleNeighbors>k ) {
		Log::out<logLargeDoubleDegree>() << "Large Double Degree rule: " << v;
		putInFVS( v );
		remove( v );
		changed = true;
	}

	//timeLargeDoubleDegree += used_time(t);
}

bool HansbKernelizer::improvementRule( node v, node w ) {
	// returns true if it sees (or makes) a double edge between v and w
	//float t = used_time();

	if( v==w ) return false; // no improvement; not a double edge

	// do the vertices even have enough edges for k+2 paths?
	if( degree(v)<k+2 ) return false; // no improvement; maybe a double edge
	if( degree(w)<k+2 ) return false; // no improvement; maybe a double edge

	// improvement rule doesn't apply if there already is a double edge
	if( hasDoubleEdge( v, w ) ) return true;

	// set up the flow network and calculate max flow
	edge_array<int> cap( flowNetwork );
	edge e;
	forall_edges( e, flowNetwork ) cap[e] = 1;
	edge_array<int> f( flowNetwork, 0 );
	int flow = MAX_FLOW( flowNetwork, flowNodeOut[v], flowNodeIn[w], cap, f );

	// do we have improvement?
	if( flow > k+1 ) {
		// show the flow
		if( logImprovement==Log::PAUSE ) {
			edge e;
			forall_edges( e, flowNetwork ) {
				if( f[e]==1 && flowOriginalEdge[e] ) {
					gw->set_color( flowOriginalEdge[e], color(red) );
					gw->set_width( flowOriginalEdge[e], 4 );
				}
			}
		}
		Log::out<logImprovement>() << "Improvement between " << v << " and " << w << " with flow " << flow << ".";
		
		// do the rule
		makeDoubleEdge( v, w );
		
		// restore graphics
		if( logImprovement==Log::PAUSE ) {
			gw->set_color( g.all_edges(), color(blue) );
			gw->set_width( g.all_edges(), 2 );
		}

		//timeImprovement += used_time(t);
		// we made a double edge.
		return true;
		
	}

	//timeImprovement += used_time(t);

	// there was no double edge, and we didn't make one
	return false;

}

void HansbKernelizer::flowerRule( node v ) {
	//float t = used_time();

	if( degree(v)<(2*k+2) ) return;

	makeMatchingGraph( v );

	list<edge> matching = MIN_WEIGHT_PERFECT_MATCHING( matchingGraph, flowerWeight );

	if( !matching.empty() ) {
		
		Log::out<logFlower>() << "Flower: " << v << ".";
		putInFVS( v );
		remove( v );

	}

	//timeFlower += used_time(t);
}

void HansbKernelizer::abdicationRules( list<node> &approxFVS ) {
	//float t = used_time();

	// construct inAorB
	inAorB.clear();
	inAorB.set_default_value( false );
	node v;
	forall( v, approxFVS ) {
		inAorB[ v ] = true; // v is in B
		node n;
		forall_adj_nodes( n, v ) {
			if( hasDoubleEdge(v,n) ) { // horrible performance; do this properly some time.
				inAorB[ n ] = true;
			}
		}
	}

	// clear the DFS visited info
	abdiDfsVisited = node_array<int>(g,0);
	abdiEdge = node_array<edge>(g,0);
	
	// DFS
	forall_nodes( v, g ) {
		abdiBorder.clear();
		abdiPiece.clear();

		if( abdiDfsVisited[v]==0 && !inAorB.defined(v) ) {
			// not yet visited, and not in A or B: a new piece

			abdiDFS( v );

		}
		if( !abdiBorder.empty() ) {
			// We have found a border
			
			// Show the piece if in extreme logging mode
			if( logAbdicationPieces == Log::PAUSE ) {
				gw->set_color( g.all_nodes(), color(white) );
				gw->set_border_width( approxFVS, 4 );

				gw->set_color( abdiPiece, color(blue2) );
				gw->set_color( abdiBorder, color(red) );
				gw->set_label( g.all_nodes(), "" );
				node n;
				forall( n, abdiBorder ) {
					gw->set_label( n, Log::toString(abdiDfsVisited[n]).c_str() );
					gw->set_width( abdiEdge[n], 4 );
				}
				out<Log::PAUSE>() << "A piece and its border.";

				gw->set_width( g.all_edges(), 1 );
				gw->set_color( g.all_nodes(), color(white) );
				gw->set_border_width( g.all_nodes(), 1 );
				gw->set_label( g.all_nodes(), "" );
			}

			// Check for governance
			node bestGovernor = 0;
			int bestDegree = 0;
			node b1, b2;
			forall( b1, abdiBorder ) {
				out<logAbdicationSteps>() << "Checking "<<b1<<" for governance.";
				bool governor = true;
				forall( b2, abdiBorder ) {
					if( b1!=b2 && !hasDoubleEdge(b1,b2) ) {
						out<logAbdicationSteps>() <<b1<<" is not a governor: has no double edge to " << b2;
						governor = false;
						break;
					}
				}
				if( governor ) {
					out<logAbdicationSteps>() << b1 << "is a governor!";
					if( degree(b1)>bestDegree ) {
						bestGovernor = b1;
						if( abdiDfsVisited[b1]>1 ) bestDegree = degree(b1);
					}
				}
			}
			if( bestGovernor!=0 ) {
				if( abdiDfsVisited[bestGovernor]==1 ) {
					// First abdication rule
					if( logAbdication == Log::PAUSE ) {
						gw->set_color( g.all_nodes(), color(white) );
						gw->set_border_width( approxFVS, 4 );

						gw->set_color( abdiPiece, color(blue2) );
						gw->set_color( abdiBorder, color(yellow) );
						gw->set_label( g.all_nodes(), "" );
						node n;
						forall( n, abdiBorder ) {
							gw->set_label( n, Log::toString(abdiDfsVisited[n]).c_str() );
							gw->set_width( abdiEdge[bestGovernor], 4 );
						}
						out<Log::PAUSE>() << "Abdication 1 on " << bestGovernor;

						gw->set_width( g.all_edges(), 1 );
						gw->set_color( g.all_nodes(), color(white) );
						gw->set_border_width( g.all_nodes(), 1 );
						gw->set_label( g.all_nodes(), "" );
					} else out<logAbdication>() << "Abdication 1 on " << bestGovernor;
					removeEdge( abdiEdge[bestGovernor] );
					changed = true;
					return;
				} else {
					// Second abdication rule
					if( logAbdication == Log::PAUSE ) {
						gw->set_color( g.all_nodes(), color(white) );
						gw->set_border_width( approxFVS, 4 );

						gw->set_color( abdiPiece, color(blue2) );
						gw->set_color( abdiBorder, color(yellow) );
						gw->set_label( g.all_nodes(), "" );
						node n;
						forall( n, abdiBorder ) {
							gw->set_label( n, Log::toString(abdiDfsVisited[n]).c_str() );
						}
						out<Log::PAUSE>() << "Abdication 2 on " << bestGovernor;

						gw->set_width( g.all_edges(), 1 );
						gw->set_color( g.all_nodes(), color(white) );
						gw->set_border_width( g.all_nodes(), 1 );
						gw->set_label( g.all_nodes(), "" );
					} else out<logAbdication>() << "Abdication 2 on " << bestGovernor;
					remove( bestGovernor );
					putInFVS( bestGovernor );
					changed = true;
					return;
				}
			}

			// clear the border
			node n;
			forall( n, abdiBorder ) {
				abdiDfsVisited[n] = 0;
			}
		}

	}

	//timeAbdication += used_time(t);
}
void HansbKernelizer::abdiDFS( node v ) {

	if( inAorB.defined(v) ) {
		// a node of the border
		++abdiDfsVisited[v]; // count an edge into the the piece for this border-vertex
		if( abdiDfsVisited[v]==1 ) {
			// this is the first time we see this border vertex; remember it
			abdiBorder.append( v );
		}

	} else {
		// a node of the piece.
		if( abdiDfsVisited[v]>0 ) {
			// been here before; backtrack. cannot actually happen: the piece is a tree.
		} else {
			++abdiDfsVisited[v]; // mark as visited
			if( abdiDfsVisited[v]==1 ) {
				// this is the first time we see this piece vertex; remember it
				abdiPiece.append( v );
			}
			// DFS
			edge e;
			forall_adj_edges( e, v ) {
				node n = g.opposite(v,e);
				abdiEdge[n] = e;
				abdiDFS( n );
			}
		}
	}

}



void HansbKernelizer::run() {
	float t = used_time();

	list<node> approxFVS;
	int approxRunForK = k+1; // just so we will start by running the 2-approx

	abort_ = false;
	changed = true;
	bool doneTripleEdgeRules = false;
	

	while( changed ) {
		changed = false;
		if( g.empty() ) break;

		node v;
		list<node> nodes;

		if( k!=approxRunForK ) {
			Timer timer(timeApprox);

			// k has changed; run the 2-approx
			float t = used_time();
			approxFVS.clear();
			BeckerGeiger approx( g, k );
			approx.run( approxFVS );
			timeApprox += used_time(t);

			if( showTwoApprox && gw.get() ) {
				gw->set_color( g.all_nodes(), color(white) );
				gw->set_color( approxFVS, color(yellow) );
				out<Log::PAUSE>() << "This is a 2-approx fvs.";
				gw->set_color( g.all_nodes(), color(white) );
			}

			if( approxFVS.size() > 2*k ) {
				abort_ = true;
				break;
			}

		}

		{	Timer timer( timeAbdication );
			abdicationRules( approxFVS );
		}

		// only do triple edge rule at start of algo: no rule ever makes triple edges.
		if( !doneTripleEdgeRules ) {
			Timer timer( timeTripleEdge );
			doneTripleEdgeRules = true;
			Log::out<logSteps>() << "Going to do triple edge rule...";
			//node v;
			forall_nodes( v, g ) {
				tripleEdgeRule( v );
			}
		}
		
		{	Timer timer( timeLowDegree );
			Log::out<logSteps>() << "Going to do low degree rules...";
			nodes = g.all_nodes();
			forall( v, nodes ) {
				if( !removed(v) ) {
					lowDegreeRules( v );
				}
			}
		}
		
		{	Timer timer( timeDegreeTwo );
			Log::out<logSteps>() << "Going to do degree two rule...";
			nodes = g.all_nodes();
			forall( v, nodes ) {
				if( !removed(v) ) {
					degreeTwoRule( v );
					if( abort_ ) {
						break;
					}
				}
			}
			if( abort_ ) break;
		}

		//Log::out<logSteps>() << "Going to do large double degree rule...";
		{	Timer timer(timeLargeDoubleDegree);
			nodes = g.all_nodes();
			forall( v, nodes ) {
				if( !removed(v) ) {
					largeDoubleDegreeRule( v );
					if( abort_ ) break;
				}
			}
			if( abort_ ) break;
		}

		Log::out<logSteps>() << "Going to do improvement rule...";
	
		nodes = g.all_nodes();
		forall( v, nodes ) {
			if( removed(v) ) continue;

			float t = used_time();
			// Try improvement rule with all other vertices.
			int numDoubleEdges = 0;
			node w;
			forall_nodes( w, g ) {
				if( v<w ) {
					if( improvementRule( v, w ) ) ++numDoubleEdges;
					if( numDoubleEdges>k ) {
						// v has k double edges: it is already going
						// to be deleted by large double degree rule!
						// so don't mind about trying more improvements.
						break;
					}
				}
			}
			timeImprovement += used_time(t);
			
			// Hopefully made lots of double edges; try large double degree.
			t = used_time();
			largeDoubleDegreeRule( v );
			timeLargeDoubleDegree += used_time(t);
			if( abort_ ) break;

			// Okay, so not so simple as large double degree; try flower rule.
			if( removed(v) ) continue;
			t = used_time();
			flowerRule( v );
			timeFlower += used_time(t);
			if( abort_ ) break;

		}
		if( abort_ ) break;
		// */

	}

	if( abort_ ) {
		out<logResult>() << "\nIMPOSSIBLE!\n";
	} else {
		out<logResult>() << "Already have " << inFVS.size() << " vertices in the FVS.";
	}

	runtime_ = used_time(t);

	Log::out<logTimeStats>() << "Timing.";
	Log::out<logTimeStats>() << "Approx              : " << timeApprox;
	Log::out<logTimeStats>() << "Abdication          : " << timeAbdication;
	Log::out<logTimeStats>() << "Low degree          : " << timeLowDegree;
	Log::out<logTimeStats>() << "Triple edge         : " << timeTripleEdge;
	Log::out<logTimeStats>() << "Degree two          : " << timeDegreeTwo;
	Log::out<logTimeStats>() << "Large double degree : " << timeLargeDoubleDegree;
	Log::out<logTimeStats>() << "Improvement         : " << timeImprovement;
	Log::out<logTimeStats>() << "Flower              : " << timeFlower;

}
float HansbKernelizer::runtime() {
	return runtime_;
}

void HansbKernelizer::remove( node v ) {
	flowNetwork.del_node( flowNodeIn[v] );
	flowNetwork.del_node( flowNodeOut[v] );
	g.del_node( v );
	v->set_owner( 0 );
	changed = true;
}
bool HansbKernelizer::removed( node v ) {
	return v->owner()==0;
}

void HansbKernelizer::removeEdge( edge e ) {
	flowNetwork.del_edge( flowEdge1[e] );
	flowNetwork.del_edge( flowEdge2[e] );
	g.del_edge( e );
	changed = true;
}
void HansbKernelizer::makeDoubleEdge( node v, node w ) {
	int numEdges = 0;
	node n;
	forall_adj_nodes( n, v ) {
		if( n==w ) { numEdges = 1; break; }
	}
	while( numEdges<2 ) {
		makeEdge( v, w );
		++numEdges;
	}
}
void HansbKernelizer::makeEdge( node v, node w ) {
	edge e = g.new_edge( v, w );
	if( gw.get() ) gw->update_edges();
	//gw->set_color( e, color(pink) );
	flowEdge1[e] = flowNetwork.new_edge( flowNodeOut[v], flowNodeIn[w] );
	flowEdge2[e] = flowNetwork.new_edge( flowNodeOut[w], flowNodeIn[v] );
	flowOriginalEdge[flowEdge1[e]] = e;
	flowOriginalEdge[flowEdge2[e]] = e;
	changed = true;
}
bool HansbKernelizer::hasDoubleEdge( node v, node w ) {
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

void HansbKernelizer::putInFVS( node v ) {
	inFVS.append( v );
	--k;
	changed = true;
	//if( inFVS.size() > k ) {
	if( k<0 ) {
		Log::out<Log::VOID>() << "Oh my, we have " << inFVS.size() << " vertices which need to be in the fvs, but k=" << k << ". NO!";
		abort_ = true;
	}
}