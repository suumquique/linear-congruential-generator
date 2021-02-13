#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <malloc.h>

static unsigned long long nextValue = 1;
const unsigned multiplier = 1103515245;
const unsigned summand = 12345;
const unsigned long long module = (unsigned long long) UINT_MAX;

void initLCG() {
	nextValue = (unsigned long long) time(NULL);
}

unsigned int nextStep() {
	nextValue = nextValue * multiplier + summand;
	return (unsigned) (nextValue % module);
}

void main(void) {
	initLCG();
	
}