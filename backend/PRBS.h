
#pragma once

#include <stdint.h>

class PRBS {
	static const uint32_t R = 32;      /* Anzahl der Worte im Zustandsregister */
	
	static const uint32_t M1 = 3;
	static const uint32_t M2 = 24;
	static const uint32_t M3 = 10;
	
	static const uint32_t MAT0POS(uint32_t t, uint32_t v) { return v ^ (v >> t);}
	static const uint32_t MAT0NEG(uint32_t t, uint32_t v) { return v ^ (v << t);} 

	/* Der Zustandsvektor muss mit (pseudo-)zufälligen Werten initialisiert werden. */
	uint32_t State[R];
	uint32_t i;
 
public:
	PRBS() {
		i = 0;
		for(uint32_t x=0; x<R; x++)
			State[x] = x*x + 7*x + 15;
	}

	uint32_t get()
	{
		uint32_t z0;
		uint32_t z1;
		uint32_t z2;

		z0 = State[(i+31) % R];
		z1 = State[i] ^ MAT0POS ( +8, State[(i+M1) % R]);
		z2 = MAT0NEG (-19, State[(i+M2) % R]) ^ MAT0NEG (-14, State[(i+M3) % R]);

		State[i] = z1 ^ z2;
		State[(i+31) % R] = MAT0NEG (-11,  z0) ^ MAT0NEG ( -7,  z1) ^ MAT0NEG (-13,  z2);

		i = (i + R - 1) % R;

		return State[i];
	}
};
