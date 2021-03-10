#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <malloc.h>

// Сколько случайных чисел будет сгенерировано ЛКГ для тестирования хи-квадратом
#define TEST_LIMIT 1000000
// Количество интервалов от 0 до модуля ЛКГ, для измерения количества чисел, попавших в каждый интервал, методом хи-квадрат
#define TEST_INTERVALS_NUMBER 20
// Количество проводимых тестов (при запуске каждого следующего теста ЛКГ вновь инициализируется с другим начальным значением)
#define TESTS 100
// Количество чисел, генерируемых ЛКГ для теста критерием сериальной корреляции
#define SERIAL_CORRELATION_NUMBERS 50000

// Число бит в целом unsigned int и в половине, чтобы проверять случайность как для полных значений, выдаваемых ЛКГ, так и для младших бит
#define MAXIMUM_BITS_NUMBER_IN_UNSIGNED_INT 32
#define HALF_BITS_NUMBER_IN_UNSIGNED_INT 16

#define printDelimeter() printf("----------------------------------------------------------------------------------------------------\n\n")

void testLCGParameters();
unsigned gcd(unsigned long long a, unsigned long long b);
void chiSquaredTest(size_t remainingBitsNumber);
double getChiSquareValue(size_t intervalsNumber, double mid_value, size_t remainingBitsNumber);
void serialCorrelationTest(size_t remainingBitsNumber);
double getSerialCorrelationValue(size_t remainingBitsNumber);

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
	chiSquaredTest(MAXIMUM_BITS_NUMBER_IN_UNSIGNED_INT);
	serialCorrelationTest(MAXIMUM_BITS_NUMBER_IN_UNSIGNED_INT);
	printDelimeter();
	puts("Now let's check the randomness of the least significant bits (half of the total):\n\n");
	serialCorrelationTest(HALF_BITS_NUMBER_IN_UNSIGNED_INT);
}

void testLCGParameters() {
	printf("1) Are c and m a coprime numbers?\nc=%u, m=%llu\nCheck gcd(c, m): %u\n\n", summand, module, gcd(summand, module));
	printf("2) Is a-1 multiple of p for every p prime divisor of m?\na - 1 = %u, single prime divisor of m = 2\n(a - 1) %% 2 = %u\n\n",
		multiplier - 1, (multiplier - 1) % 2);
	printf("3) Is a-1 is a multiple of 4 if m is a multiple of 4?\nm %% 4 = %llu, (a - 1) %% 4 = %u\n\n", module % 4, (multiplier - 1) % 4);
	printDelimeter();
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

/* remainingBitsNumber - количество бит, которые требуется оставить. Если мы смотрим полное число (unsigned int) - то 32,
* если только младшие биты - то 16 */
void serialCorrelationTest(size_t remainingBitsNumber) {
	/* Критерии для проверки значения значения сериальной корреляции: μ и σ (мю и сигма).
	* Формула для μ: μ = (-1)/(n - 1)
	* Формула для σ: σ^2 = (n^2)/((n - 2)*((n - 1)^2))
	* В данном случае n - количество интервалов */
	double mju = -1.0 / (SERIAL_CORRELATION_NUMBERS - 1);
	double sigma = sqrt(pow(SERIAL_CORRELATION_NUMBERS, 2) / (((double) SERIAL_CORRELATION_NUMBERS - 2) * pow((SERIAL_CORRELATION_NUMBERS - 1), 2)));

	/* Количество "хороших" или "плохих" значений: сколько раз за определенное число тестов критерий сериальной корреляции
	* вернул значение, попадающее в рамки случайной последовательности, а сколько раз вернул значение, показывающее,
	* что ЛКГ недостаточно случайно */
	size_t goodValueCounter = 0, badValueCounter = 0;
	double currentValue = 0.0;

	// Верхняя и нижняя границы "хорошего" значения критерия сериальной корреляции
	double lowerLimit = mju - 2 * sigma, higherLimit = mju + 2 * sigma;
	for (size_t i = 0; i < TESTS; i++) {
		initLCG();
		currentValue = getSerialCorrelationValue(remainingBitsNumber);
		if (currentValue > lowerLimit && currentValue < higherLimit) {
			goodValueCounter++;
		}
		else badValueCounter++;
	}

	printf("\nThe percentage of \"good\" values of the serial correlation criterion indicating that the sequence is random was %g%%\n\n",
		(double) goodValueCounter / TESTS * 100);
	printf("The percentage of \"bad\" values of the serial correlation criterion indicating that the sequence isn`t random was %g%%\n\n",
		(double) badValueCounter / TESTS * 100);
}

/*Функция считает значение критерия сериальной корреляции для ЛКГ по этой формуле: https://prnt.sc/109nzd9
Значением Uj в данном случае является сумма всех элементов на интервале
*/
double getSerialCorrelationValue(size_t remainingBitsNumber) {
	size_t i;
	double* pseudorandomNumbers = (double*) calloc((SERIAL_CORRELATION_NUMBERS + 1), sizeof(double));

	for (i = 0; i < SERIAL_CORRELATION_NUMBERS; i++) {
		// Делим текущее значение ЛКГ на модуль, чтобы не было переполнения
		pseudorandomNumbers[i] = (double) (nextStep() % (unsigned long long) pow(2, remainingBitsNumber)) / module;
	}

	double finalCriterionValue;
	// Делимое и делитель формулы для получения итогового значения (скрин формулы в описании функции)
	double dividend, divisor, dividendMinuend = 0, dividendSubtrahend = 0, divisorMinuend = 0, divisorSubtrahend = 0;

	for (i = 0; i < SERIAL_CORRELATION_NUMBERS - 1; i++) {
		dividendMinuend += pseudorandomNumbers[i] * pseudorandomNumbers[i + 1];
	}
	dividendMinuend += pseudorandomNumbers[i] * pseudorandomNumbers[0];
	dividendMinuend *= SERIAL_CORRELATION_NUMBERS;

	for (i = 0; i < SERIAL_CORRELATION_NUMBERS; i++) dividendSubtrahend += pseudorandomNumbers[i];
	dividendSubtrahend = dividendSubtrahend * dividendSubtrahend;
	dividend = dividendMinuend - dividendSubtrahend;

	for (i = 0; i < SERIAL_CORRELATION_NUMBERS; i++) {
		divisorMinuend += pow(pseudorandomNumbers[i], 2);
	}
	divisorMinuend *= SERIAL_CORRELATION_NUMBERS;
	
	divisorSubtrahend = dividendSubtrahend;
	divisor = divisorMinuend - divisorSubtrahend;

	return (double)dividend / divisor;
}

double getChiSquareValue(size_t intervalsNumber, double mid_value, size_t remainingBitsNumber) {
	// Текущее псевдорандомное число, выданное ЛКГ
	unsigned currentValue = 0;
	size_t currentInterval, i;

	/* Массив, в котором сохраненяется количество чисел, попавший в каждый интервал. Нулевой элемент - первый интервал,
	* первый элемент - второй интервал, и так далее. Интервалы значений таковы: 
	* (максимальное число, которое может быть выдано ЛКГ, то есть module - 1) / (кол-во интервалов).
	* То есть первый интервал от 0 до module / intervalsNumber, второй - от конца первого до (module / intervalsNumber) * 2 и так далее. */
	size_t* measurementsNumberInInterval = (size_t*)calloc((intervalsNumber + 1), sizeof(size_t));

	for (i = 0; i < TEST_LIMIT; i++) {
		// Получаем следующее значение ЛКГ
		currentValue = nextStep() % (unsigned long long) pow(2, remainingBitsNumber);
		// Определяем, в какой интервал оно попадает
		currentInterval = ((float) currentValue / (unsigned long long) pow(2, remainingBitsNumber)) * intervalsNumber;
		measurementsNumberInInterval[currentInterval]++;
	}

	double xi2 = 0;
	for (i = 0; i < intervalsNumber; i++) {
		xi2 += pow(measurementsNumberInInterval[i] - mid_value, 2) / mid_value;
	}

	free(measurementsNumberInInterval);

	return xi2;
}

void chiSquaredTest(size_t remainingBitsNumber) {
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
		currentChiSquare = getChiSquareValue(TEST_INTERVALS_NUMBER, TEST_LIMIT / TEST_INTERVALS_NUMBER, remainingBitsNumber);
		if (currentChiSquare > fivePercentChanceLowerLimit && currentChiSquare < fivePercentChanceUpperLimit) 
			numbersAmountInNinetyPercentInterval++;
		if (currentChiSquare > twentyFivePercentChanceLowerLimit && currentChiSquare < twentyFivePercentChanceUpperLimit)
			numbersAmountInFiftyPercentInterval++;
	}

	printf("Typically 50%% of the chi-square values at %u degrees of freedom are split between the 25%% = %g limit and the 75%% = %g limit.\n\
In the current case, for %u tests %g%% values fell into this interval\n\n", TEST_INTERVALS_NUMBER, twentyFivePercentChanceLowerLimit,
		twentyFivePercentChanceUpperLimit, TESTS, numbersAmountInFiftyPercentInterval / (double) TESTS * 100);
	printf("Typically 90%% of the chi-square values at %u degrees of freedom are split between the 5%% = %g limit and the 95%% = %g limit.\n\
In the current case, for %u tests %g%% values fell into this interval\n\n", TEST_INTERVALS_NUMBER, fivePercentChanceLowerLimit,
		fivePercentChanceUpperLimit, TESTS,  numbersAmountInNinetyPercentInterval / (double) TESTS * 100);
	printDelimeter();
}