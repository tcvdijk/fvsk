// Timer.h

#ifndef INCLUDED_TIMER
#define INCLUDED_TIMER

class Timer {
public:

	Timer( float &ref ) : ref(ref) {
		start = used_time();
	}
	~Timer() {
		ref += used_time(start);
	}
	float &ref;
	float start;

private:
	Timer( const Timer& t ) : ref(t.ref) {}
	void operator=( const Timer& ) {}
};


#endif //ndef INCLUDED_TIMER