#ifndef INCLUDED_LOG
#define INCLUDED_LOG

#ifndef STUPID_COMPILER

#include <memory>
#include <sstream>
#include <fstream>
#include <vector>
#include <sstream>

//#define LEDA_MEMORY_MT
#include <LEDA/graphics/color.h>
#include <LEDA/graphics/graphwin.h>
#include <LEDA/graph/graph.h>

// forward declarations and externs
class leda::GraphWin;
extern std::auto_ptr<leda::GraphWin> gw;

namespace Log {

	extern std::ofstream logfile;
	extern std::string colorName[6];
	extern leda::color colorValue[6];

	enum Level {
		VOID,
		FILE,
		LOG,
		SHOW,
		PAUSE
	};

	struct Nothing {
		Nothing() {}
		Nothing( bool b ) {}
		operator bool() const{ return false; }
		template< typename T >
		Nothing operator<<( const T &x ) const { return *this; }
		template< typename T >
		void reset( T ) const {}
		std::string str() const { return ""; }
		void push_back( leda::node v ) {}
		typedef int **iterator;
		iterator begin() const { return 0; }
		iterator end() const { return 0; }
	};


	template< typename T, bool C >
	struct Maybe {};
	template< typename T >
	struct Maybe< T, false > {
		typedef Nothing T;
	};
	template< typename T >
	struct Maybe< T, true > {
		typedef T T;
	};

	template< bool Fout, bool Stdout, bool Gwout, bool Pause >
	class Connector {
	public:

		typedef Connector<Fout,Stdout,Gwout,Pause> This;
		typedef std::vector<leda::node> Nodes;

		static const bool nonVoid = Fout||Stdout||Gwout||Pause;
		typename Maybe< std::stringstream, Gwout >::T str;
		typename Maybe< Nodes, Pause >::T nodes;
		typename Maybe< bool, nonVoid >::T used;

		int colorCount;

		Connector() : colorCount(0) {}

		template< typename T >
		This &operator<<( const T &x ) {
			if( nonVoid ) used = true;
			if( Fout ) logfile << x;
			if( Stdout ) std::cout << x;
			if( Gwout ) str << x;
			return *this;
		}

		This &operator<<( leda::node v ) {
			if( nonVoid ) used = true;
			if( Fout ) logfile << v;
			if( Stdout ) std::cout << v;
			if( Gwout ) {
				if( Pause ) {
					str << colorName[colorCount];
					gw->set_color( v, colorValue[colorCount++] );
					if( colorCount>5 ) colorCount=0;
					nodes.push_back( v );
				} else str << v;
			}
			return *this;
		}

		~Connector() {
			if( used ) {
				if( Fout ) logfile << std::endl;
				if( Stdout ) std::cout << std::endl;
				if( Gwout ) {
					gw->update_graph();
					gw->redraw();
					gw->message( str.str().c_str() );
				}
				if( Pause ) {
					gw->wait();
					gw->del_message();
					Maybe<Nodes,Pause>::T::iterator i, end = nodes.end();
					for( i=nodes.begin(); i!=end; ++i )
						gw->set_color( reinterpret_cast<leda::node>(*i), color(white) );
				}
			}
		}

	};


	template< Level L >
	struct StreamType {};
	template<>
	struct StreamType< VOID > {
		typedef Connector<false,false,false,false> T;
	};
	template<>
	struct StreamType< FILE > {
		typedef Connector<true,false,false,false> T;
	};
	template<>
	struct StreamType< LOG > {
		typedef Connector<true,true,false,false> T;
	};
	template<>
	struct StreamType< SHOW > {
		typedef Connector<true,true,true,false> T;
	};
	template<>
	struct StreamType< PAUSE > {
		typedef Connector<true,true,true,true> T;
	};

	template< Level L >
	typename StreamType<L>::T out() {
		return StreamType<L>::T();
	}





	template< typename T >
	std::string toString( T x ) {
		std::ostringstream str;
		str << x;
		return str.str();
	}

						
}

#else

#include <iostream>
#include <fstream>

namespace Log {

	enum Level {
		VOID,
		FILE,
		LOG,
		SHOW,
		PAUSE
	};

	class Connector {
	public:
		template< typename T >
		Connector &operator<<( const T &x ) {
			return *this;
		}
	};

	template< Level L >
	Connector out() {
		return Connector();
	}

}

#endif //ndef STUPID_COMPILER

#endif //ndef INCLUDED_LOG
