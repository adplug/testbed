// _______
// OPL.hpp
//
// Very experimental set of classes to learn about the secrets
// of OPL2/3 calculus. 
// Right now we have a waveform and envelope generator in
// (what I think is) microprocessor style.
//
// See also: test.cpp
//
// Written in 2002-12 by Volker Gietz <talphir@web.de>

typedef unsigned long ulong;

// ______
// Phasor
// 
// ABSTRACT: Phase accumulating saw wave generator

struct Phasor {

	long Get() {
		return phi += omega;
	}

	long phi;
	long omega;
};

#include <cmath>

// _________
// SineTable
//
// ABSTRACT: A table of the first quarter of a sine wave
//   Supports normalised phase readout
// 
// 512 real entries, 2048 effectively - to my knowledge what OPL2 has also

struct SineTable {

	SineTable() {
		for (ulong i=0;i<512;++i)
			sine[i] = short(32767.0*sin(i/1024.0 * 3.141592653));
	}

	short operator [] (long phi) {
		if (phi & 0x40000000) 
			phi ^= 0x3FFFFFFF;				// symmetry to 90 and 270 dgs

		return sine[(phi >> 21) & 511];		// map to 512 entries
	}

	short sine[512];
} stab; // one global instance

// ________
// Waveform
//
// ABSTRACT: OPL2 waveform generator
//   Four waveforms are selectable:
//     __        __        __  __    _   _
//    /  \      /  \___   /  \/  \  / |_/ |_
//        \__/      
//
//     0:sine   1:sine>0  2:|sine|  3:chopped

struct Waveform {
	
	short operator [] (long phi) {
		short y = stab[phi];  // use sine table

		// use upper two bits to mangle waveform
		if (type == 3 && phi & 0x40000000) y = 0;
		return (phi & 0x80000000) ? ((type & 1) ? 0 : ((type & 2) ? y : -y)) : y;
	}

	ulong type;			// 0, 1, 2, 3
};

// _____
// Decay
//
// ABSTRACT: Simple exponential decay envelope
//
// USAGE: Set the 'shift' member to change the half-time period
//   0=fast .. 24=slow

struct Decay {

	Decay() : level(0), shift(0) {
	}

	ulong Get() {
		ulong out = level;
		level -= (level >> shift);	// level *= 1 - 1/(2^shift)
		return out;
	}

	bool Finished() {
		return !(level >> 8);	// OPL has only 24(?) bits so "quite zero" is OK
	}

	ulong	level;				// current level
	ulong	shift;				// 0..15 = half-life time
};

// ____
// ADSR
//
// ABSTRACT: Attack-Decay-Sustain-Release envelope

struct ADSR {

	// set these - see Decay for ranges
	ulong aRate, dRate, rRate;

	// normalised sustain level
	ulong susLevel;

	ADSR() : attack(false), aRate(0), dRate(0), rRate(0), susLevel(0), bias(0) {
	}

	void KeyOn() {
		if (attack) return;					// am already in attack
		attack = true;	
		env.level = ~(env.level + bias);	// take over at current level
		env.shift = aRate;					// use attack rate
	}

	void KeyOff() {
		if (attack) {
			env.level = ~env.level;			// cancel inversion
			attack = false;
		} else {
			env.level += bias;				// take over at actual level
			bias = 0;						// and let go to zero
		}

		env.shift = rRate;					// use release rate
	}

	ulong Get() {
		ulong level = env.Get();			// get new level
		if (attack) {
			if (env.Finished()) {			// time to go from attack to decay
				attack = false;
				bias = susLevel;			// bias is now sustain level
				env.level = ~susLevel;		// leave room for the bias
				env.shift = dRate;			// use decay rate
			}
			return ~level;					// if attack: level goes up
		}
		return level + bias;				// no attack: level goes down
	}	

	bool attack;
	Decay env;
	ulong bias;
};

