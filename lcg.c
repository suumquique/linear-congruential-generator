#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <malloc.h>

#define TEST_LIMIT 1000000
#define TEST_INTERVALS_NUMBER 20

void testLCGParameters();
unsigned gcd(unsigned long long a, unsigned long long b);
double chiSquaredTest(size_t intervalsNumber, double mid_value);

static unsigned long long nextValue = 1;
const unsigned multiplier = 1103515245;
const unsigned summand = 12345;
const unsigned long long module = (unsigned long long) UINT_MAX + 1;

void initLCG() {
	nextValue = (unsigned long long) time(NULL);
}

unsigned nextStep() {
	nextValue = nextValue * multiplier + summand;
	return (unsigned) (nextValue % module);
}

void main(void) {
	initLCG();
	testLCGParameters();
	chiSquaredTest(TEST_INTERVALS_NUMBER, TEST_LIMIT / TEST_INTERVALS_NUMBER);
}

void testLCGParameters() {
	printf("1) Are c and m a coprime numbers?\nc=%u, m=%llu\nCheck gcd(c, m): %u\n\n", summand, module, gcd(summand, module));
	printf("2) Is a-1 multiple of p for every p prime divisor of m?\na - 1 = %u, single prime divisor of m = 2\n(a - 1) %% 2 = %u\n\n",
		multiplier - 1, (multiplier - 1) % 2);
	printf("3) Is a-1 is a multiple of 4 if m is a multiple of 4?\nm %% 4 = %llu, (a - 1) %% 4 = %u\n\n", module % 4, (multiplier - 1) % 4);
}

unsigned gcd(unsigned long long a, unsigned long long b){
	unsigned long long temp;
	while (b != 0)
	{
		temp = a % b;

		a = b;
		b = temp;
	}
	return a;
}

double chiSquaredTest(size_t intervalsNumber, double mid_value) {
	unsigned currentValue = 0;
	size_t currentInterval, i;
	size_t* measurementsNumberInInterval = (size_t*)malloc(intervalsNumber * sizeof(size_t));
	for (i = 0; i < intervalsNumber; i++) measurementsNumberInInterval[i] = 0;
	for (i = 0; i < TEST_LIMIT; i++) {
		currentValue = nextStep();
		currentInterval = ((float) currentValue / module) * intervalsNumber;
		measurementsNumberInInterval[currentInterval]++;
	}

	double xi2 = 0;
	for (i = 0; i < intervalsNumber; i++) {
		xi2 += pow(measurementsNumberInInterval[i] - mid_value, 2) / mid_value;
	}

	printf("chi-squared value: %g\n\n", xi2);
}