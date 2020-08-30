// util.h

#ifndef INCLUDED_UTIL
#define INCLUDED_UTIL

#include <LEDA/graph/graph.h>
#include <LEDA/graph/ugraph.h>
#include <LEDA/core/map.h>

void copyUGraph( leda::ugraph &from, leda::ugraph &to, leda::map<leda::node,leda::node> &original );
leda::node_map<bool> lcsToBfvs( leda::graph &lcs, leda::ugraph &bfvs, leda::map<leda::node, leda::node> &original, leda::map<leda::node, leda::node> *boNode = 0 );

void originalize( const leda::map<leda::node,leda::node> &original, leda::list<leda::node> &from, leda::list<leda::node> &to );

#endif //ndef INCLUDED_UTIL