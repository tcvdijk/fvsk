// log.cpp

#ifndef STUPID_COMPILER

#include "log.h"
std::ofstream Log::logfile( "log.txt" );

std::string Log::colorName[] = {
	"red vertex",
	"green vertex",
	"blue vertex", 
	"yellow vertex",
	"orange vertex",
	"pink vertex"
};
leda::color Log::colorValue[] = {
	leda::color(leda::red),
	leda::color(leda::green),
	leda::color(leda::blue2),
	leda::color(leda::yellow),
	leda::color(leda::orange),
	leda::color(leda::pink)
};

#endif