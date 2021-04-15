#include <stdio.h>

#include <string.h>
#include "my_string.h"
#define debug 1

void strlenAssert(char* str, int *testNumber, int *testsPassed) {
	int given, expected, passed;

	(*testNumber)++;
	expected = strlen(str);
	given = my_strlen(str);

	passed = expected == given;

	if (debug)
		printf("TEST %d: strlen(\"%.100s\") %s", *testNumber, str, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %d, expected: %d\n", given, expected);
	else
		(*testsPassed)++;
}

char *generateLongStr(int length) {
	int i;
	char *longStr = malloc(length);
	for (i = 0; i < length - 1; i++)
		longStr[i] = i % 126 + 1;
	
	longStr[i + 1] = '\0';
	return longStr;
}

void strlenStress(int length, int *testNumber, int *testsPassed) {
	int given, expected, passed;
	char* str = generateLongStr(length);

	(*testNumber)++;
	expected = strlen(str);
	given = my_strlen(str);

	passed = expected == given;

	if (debug)
		printf("STRESS TEST %d: strlen(\"...\") %s", *testNumber, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %d, expected: %d\n", given, expected);
	else
		(*testsPassed)++;
	
	free(str);
}

void strcpyAssert(char* str, int *testNumber, int *testsPassed) {
	char *given, *expected;
	int passed, length;
	length = strlen(str) + 1;

	given = malloc(length);
	expected = malloc(length);
	

	(*testNumber)++;
	strcpy(expected, str);
	my_strcpy(given, str);
	passed = strcmp(given, expected) == 0;

	if (debug)
		printf("TEST %d: strcpy(, \"%.100s\") %s", *testNumber, str, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %s, expected: %s\n", given, expected);
	else
		(*testsPassed)++;
	
	free(given);
	free(expected);
}

void strcpyStress(int length, int *testNumber, int *testsPassed) {
	char *given, *expected;
	char *str = generateLongStr(length);
	int passed;
	given = malloc(length);
	expected = malloc(length);

	(*testNumber)++;
	strcpy(expected, str);
	my_strcpy(given, str);

	passed = strcmp(expected, given) == 0;

	if (debug)
		printf("STRESS TEST %d: strcpy(, \"...\") %s", *testNumber, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %.100s, expected: %.100s\n", given, expected);
	else
		(*testsPassed)++;
	
	free(str);
	free(expected);
	free(given);
}

void strncpyAssert(char *str, int sizeMod, int *testNumber, int *testsPassed) {
	int length, passed;
	char *given, *expected;
	
	length = strlen(str) + sizeMod;
	given = malloc(length);
	expected = malloc(length);

	(*testNumber)++;
	strncpy(expected, str, length);
	my_strncpy(given, str, length);

	passed = memcmp(expected, given, length) == 0;

	if (debug)
		printf("TEST %d: strncpy(, \"%.100s\", %d) %s",
			*testNumber, str, length, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %s, expected: %s\n", given, expected);
	else
		(*testsPassed)++;
	

	free(given);
	free(expected);
}

void strncpyStress(int len, int sizeMod, int *testNumber, int *testsPassed) {
	int length, passed;
	char *given, *expected, *str;
	str = generateLongStr(len);
	
	length = len + sizeMod;
	given = malloc(length);
	expected = malloc(length);

	(*testNumber)++;
	strncpy(expected, str, length);
	my_strncpy(given, str, length);

	passed = memcmp(expected, given, length) == 0;

	if (debug)
		printf("STRESS TEST %d: strncpy(, \"...\", %d) %s",
			*testNumber, sizeMod, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %.100s, expected: %.100s\n", given, expected);
	else
		(*testsPassed)++;
	

	free(given);
	free(expected);
}

void strncpyTests(int *testNumber, int *testsPassed) {

	strncpyAssert("", 0, testNumber, testsPassed);
	strncpyAssert("", 1, testNumber, testsPassed);
	strncpyAssert("", 50, testNumber, testsPassed);
	strncpyAssert("A", 0, testNumber, testsPassed);
	strncpyAssert("A", 1, testNumber, testsPassed);
	strncpyAssert("A", 50, testNumber, testsPassed);
	strncpyAssert("A", -1, testNumber, testsPassed);
	strncpyAssert("Dell", 0, testNumber, testsPassed);
	strncpyAssert("Dell", 1, testNumber, testsPassed);
	strncpyAssert("Dell", 50, testNumber, testsPassed);
	strncpyAssert("Dell", -1, testNumber, testsPassed);
	strncpyAssert("Dell", -3, testNumber, testsPassed);
	strncpyAssert("Dell", -4, testNumber, testsPassed);
	strncpyAssert("Lenovo", 0, testNumber, testsPassed);
	strncpyAssert("Lenovo", 1000, testNumber, testsPassed);
	strncpyAssert("Lenovo", -4, testNumber, testsPassed);
}

void strncpyStressTests(int *testNumber, int *testsPassed) {
	
	strncpyStress(10000, 0, testNumber, testsPassed);
	strncpyStress(10000, -100, testNumber, testsPassed);
	strncpyStress(10000, 100, testNumber, testsPassed);
	strncpyStress(1000000, 0, testNumber, testsPassed);
	strncpyStress(1000000, 1000, testNumber, testsPassed);
	strncpyStress(1000000, -1000, testNumber, testsPassed);
	strncpyStress(100000000, 0, testNumber, testsPassed);
	strncpyStress(100000000, 10000, testNumber, testsPassed);
	strncpyStress(100000000, -10000, testNumber, testsPassed);
}

void strlenTests(int *testNumber, int *testsPassed) {

	// strlenAssert(NULL, testNumber, testsPassed);
	strlenAssert("", testNumber, testsPassed);
	strlenAssert("A", testNumber, testsPassed);
	strlenAssert("Dell", testNumber, testsPassed);
	strlenAssert("Lenovo", testNumber, testsPassed);
	strlenAssert("HP", testNumber, testsPassed);
	strlenAssert("Asus", testNumber, testsPassed);
	strlenAssert("Acer", testNumber, testsPassed);
	strlenAssert("Apple", testNumber, testsPassed);
}


void strlenStressTests(int *testNumber, int *testsPassed) {
	strlenStress(10000, testNumber, testsPassed);
	strlenStress(1000000, testNumber, testsPassed);
	strlenStress(100000000, testNumber, testsPassed);
}

void strcpyTests(int *testNumber, int *testsPassed) {

	strcpyAssert("", testNumber, testsPassed);
	strcpyAssert("A", testNumber, testsPassed);
	strcpyAssert("Dell", testNumber, testsPassed);
	strcpyAssert("Lenovo", testNumber, testsPassed);
	strcpyAssert("HP", testNumber, testsPassed);
	strcpyAssert("Asus", testNumber, testsPassed);
	strcpyAssert("Acer", testNumber, testsPassed);
	strcpyAssert("Apple", testNumber, testsPassed);
}


void strcpyStressTests(int *testNumber, int *testsPassed) {
	strcpyStress(10000, testNumber, testsPassed);
	strcpyStress(1000000, testNumber, testsPassed);
	strcpyStress(100000000, testNumber, testsPassed);
}

void memmoveSplitAssert(int size, int *testNumber, int *testsPassed) {
	/* creates two separate memory allocations for movement */
	int passed;
	void *source, *destinationA, *destinationB;

	source = malloc(size);

	destinationA = memmove(malloc(size), source, size);
	destinationB = my_memmove(malloc(size), source, size);

	passed = memcmp(destinationA, destinationB, size) == 0;
	(*testNumber)++;
	
	if (debug)
		printf("TEST %d: SPLIT memmove(%p, %p, ...) %s",
			*testNumber, destinationB, source, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %.100s, expected: %.100s\n", // go ahead and print data as chars
			(char *) destinationB, (char *) destinationA);    // maybe it'll help with debugging
	else
		(*testsPassed)++;
	
	free(source);
	free(destinationA);
	free(destinationB);
}

void memmoveOverlapAssert(int size, int offset, int *testNumber, int *testsPassed) {
	/* creats one large memory allocation (size + |offset|) for movement */
	int passed;
	void *sourceA, *sourceB, *destinationA, *destinationB;

	if (offset < 0) {
		destinationA = malloc(size - offset);
		destinationB = malloc(size - offset);
		sourceA = destinationA - offset;
		sourceB = destinationB - offset;
		memmove(destinationB, destinationA, size - offset);
	} else {
		sourceA = malloc(size + offset);
		sourceB = malloc(size + offset);
		destinationA = sourceA + offset;
		destinationB = sourceB + offset;
		memmove(sourceB, sourceA, size + offset);
	}

	memmove(destinationA, sourceA, size);
	my_memmove(destinationB, sourceB, size);

	(*testNumber)++;
	if (offset < 0)
		passed = memcmp(destinationA, destinationB, size) == 0;
	else
		passed = memcmp(sourceA, sourceB, size) == 0;

	if (debug)
		printf("TEST %d: OVERLAP memmove(%p, %p, ...) %s",
			*testNumber, destinationB, sourceB, passed ? "PASSED\n" : "\n\t");

	if (!passed)
		printf("FAILED: given: %.100s, expected: %.100s\n", // again, print data as chars
			(char *) destinationB, (char *) destinationA);
	else
		(*testsPassed)++;
	
	if (offset < 0) {
		free(destinationA);
		free(destinationB);
	} else {
		free(sourceA);
		free(sourceB);
	}
}

void memmoveTests(int *testNumber, int *testsPassed) {
	memmoveSplitAssert(1, testNumber, testsPassed);
	memmoveSplitAssert(10, testNumber, testsPassed);
	memmoveSplitAssert(100, testNumber, testsPassed);
	memmoveSplitAssert(10000, testNumber, testsPassed);
	memmoveOverlapAssert(1, 0, testNumber, testsPassed);
	memmoveOverlapAssert(1, 5, testNumber, testsPassed);
	memmoveOverlapAssert(1, -5, testNumber, testsPassed);
	memmoveOverlapAssert(10, 5, testNumber, testsPassed);
	memmoveOverlapAssert(10, 50, testNumber, testsPassed);
	memmoveOverlapAssert(10, -50, testNumber, testsPassed); // 
	memmoveOverlapAssert(1000, 500, testNumber, testsPassed);
	memmoveOverlapAssert(1000, -500, testNumber, testsPassed); //
	memmoveOverlapAssert(100000, 5000, testNumber, testsPassed);
	memmoveOverlapAssert(100000, -5000, testNumber, testsPassed); //
}

void printSummary(int *testNumber, int *testsPassed) {

	float avg = (float) *testsPassed / *testNumber;
	printf("Passed %d out of %d, %.3f\n", *testsPassed, *testNumber, avg);
}

int main(int argc, char** argv)
{
	int testNumber = 0, testsPassed = 0;
	strlenTests(&testNumber, &testsPassed);
	strlenStressTests(&testNumber, &testsPassed);
	strcpyTests(&testNumber, &testsPassed);
	strcpyStressTests(&testNumber, &testsPassed);
	strncpyTests(&testNumber, &testsPassed);
	strncpyStressTests(&testNumber, &testsPassed);
	memmoveTests(&testNumber, &testsPassed);
	printSummary(&testNumber, &testsPassed);
}