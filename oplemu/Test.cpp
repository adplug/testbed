// ________
// Test.cpp
//
// A small console program that generates
// a single (hopefully Adlib-like) sound.
//
// Writes 16-bit PCM data to opl-test.pcm (44.1kHz)

#include "OPL.hpp"

#include <stdio.h>

int main()
{
	// Main frequency
	Phasor ps;
	ps.omega = 1 << 22;
	ps.phi = 0;

	// Modulator waveform
	Waveform wave1;
	wave1.type = 1;

	// Carrier waveform
	Waveform wave2;

	// Modulator envelope
	ADSR env1;
	env1.aRate = 0;
	env1.dRate = 16;
	env1.rRate = 14;

	// Carrier envelope
	ADSR env2;
	env2.aRate = 0;
	env2.dRate = 18;
	env2.rRate = 12;

	FILE *out = fopen("opl-test.pcm", "wb+");

	for (int j=0;j<2;++j) {
		if (j) {
			env1.KeyOff(); 
			env2.KeyOff();
		} else {
			env1.KeyOn();
			env2.KeyOn();
		}
		for (int i=0;i<44100;++i) {	
			long mod = (wave1[ps.Get()] * (env1.Get() >> 16));
			short car = (wave2[ps.Get()*2 + mod] * (env2.Get() >> 16)) >> 16;
			fwrite(&car, 2, 1, out);
		}
	}

	fclose(out);

	return 0;
}