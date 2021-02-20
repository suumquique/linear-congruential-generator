#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <malloc.h>

// Сколько случайных чисел будет сгенерировано ЛКГ для тестирования
#define TEST_LIMIT 1000000
// Количество интервалов от 0 до модуля ЛКГ, для измерения количества чисел, попавших в каждый интервал, методом хи-квадрат
#define TEST_INTERVALS_NUMBER 20
// Количество проводимых тестов (каждый при новой инициализации генератора)
#define TESTS 100

void testLCGParameters();
unsigned gcd(unsigned long long a, unsigned long long b);
void chiSquaredTest();
double getChiSquareValue(size_t intervalsNumber, double mid_value);

// Формула - nextValue = (previousValue * multiplier + summand) % module
static unsigned long long nextValue = 1;
const unsigned multiplier = 1103515245;
const unsigned summand = 12345;
const unsigned long long module = (unsigned long long) UINT_MAX + 1;

void initLCG() {
	nextValue = (unsigned long long) time(NULL) + (unsigned long long) clock();
}

unsigned nextStep() {
	nextValue = nextValue * multiplier + summand;
	return (unsigned) (nextValue % module);
}

void main(void) {
	testLCGParameters();
	chiSquaredTest();
}

void testLCGParameters() {
	printf("1) Are c and m a coprime numbers?\nc=%u, m=%llu\nCheck gcd(c, m): %u\n\n", summand, module, gcd(summand, module));
	printf("2) Is a-1 multiple of p for every p prime divisor of m?\na - 1 = %u, single prime divisor of m = 2\n(a - 1) %% 2 = %u\n\n",
		multiplier - 1, (multiplier - 1) % 2);
	printf("3) Is a-1 is a multiple of 4 if m is a multiple of 4?\nm %% 4 = %llu, (a - 1) %% 4 = %u\n\n", module % 4, (multiplier - 1) % 4);
}

// Наибольший общий делитель
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

double getChiSquareValue(size_t intervalsNumber, double mid_value) {
	// Текущее псевдорандомное число, выданное ЛКГ
	unsigned currentValue = 0;
	size_t currentInterval, i;

	/* Массив, в котором сохраненяется количество чисел, попавший в каждый интервал. Нулевой элемент - первый интервал,
	* первый элемент - второй интервал, и так далее. Интервалы значений таковы: 
	* (максимальное число, которое может быть выдано ЛКГ, то есть module - 1) / (кол-во интервалов).
	* То есть первый интервал от 0 до module / intervalsNumber, второй - от конца первого до (module / intervalsNumber) * 2 и так далее. */
	size_t* measurementsNumberInInterval = (size_t*)malloc(intervalsNumber * sizeof(size_t));
	// До запуска генератора во всех интервалах ноль чисел
	for (i = 0; i < intervalsNumber; i++) measurementsNumberInInterval[i] = 0;
	for (i = 0; i < TEST_LIMIT; i++) {
		// Получаем следующее значение ЛКГ
		currentValue = nextStep();
		// Определяем, в какой интервал оно попадает
		currentInterval = ((float) currentValue / module) * intervalsNumber;
		measurementsNumberInInterval[currentInterval]++;
	}

	double xi2 = 0;
	for (i = 0; i < intervalsNumber; i++) {
		xi2 += pow(measurementsNumberInInterval[i] - mid_value, 2) / mid_value;
	}

	return xi2;
}

void chiSquaredTest() {
	double currentChiSquare;

	/* Шанс значения, возвращенного функцией getChiSquareValue, попасть в определенный интервал.
	Все шансы рассчитаны для TEST_INTERVALS_NUMBER степеней свободы. Например, шанс того, что значение будет меньше 10.85 или больше 31.41,
	меньше 10%. Процентные точки распределения взяты из книги M. Abramowitz and I. A. Stegun, Handbook of Mathematical Functions*/
	double fivePercentChanceLowerLimit = 10.85, twentyFivePercentChanceLowerLimit = 15.45,
		twentyFivePercentChanceUpperLimit = 23.83, fivePercentChanceUpperLimit = 31.41;

	// Количество чисел, попавших в определенный интервал. Между 5% и 95% - 90-процентный интервал.
	size_t numbersAmountInFiftyPercentInterval = 0, numbersAmountInNinetyPercentInterval = 0;
	for (size_t i = 0; i < TESTS; i++) {
		initLCG();
		currentChiSquare = getChiSquareValue(TEST_INTERVALS_NUMBER, TEST_LIMIT / TEST_INTERVALS_NUMBER);
		if (currentChiSquare > fivePercentChanceLowerLimit && currentChiSquare < fivePercentChanceUpperLimit) 
			numbersAmountInNinetyPercentInterval++;
		if (currentChiSquare > twentyFivePercentChanceLowerLimit && currentChiSquare < twentyFivePercentChanceUpperLimit)
			numbersAmountInFiftyPercentInterval++;
	}

	printf("Typically 50%% of the chi-square values at %u degrees of freedom are split between the 25%% = %g limit and the 75%% = %g limit.\n\
In the current case, for %u tests %u%% values fell into this interval\n\n", TEST_INTERVALS_NUMBER, twentyFivePercentChanceLowerLimit,
		twentyFivePercentChanceUpperLimit, TESTS, numbersAmountInFiftyPercentInterval / TESTS * 100);
	printf("Typically 90%% of the chi-square values at %u degrees of freedom are split between the 5%% = %g limit and the 95%% = %g limit.\n\
In the current case, for %u tests %u%% values fell into this interval\n\n", TEST_INTERVALS_NUMBER, fivePercentChanceLowerLimit,
		fivePercentChanceUpperLimit, TESTS, numbersAmountInNinetyPercentInterval / TESTS * 100);
}