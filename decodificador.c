#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BASE 91

uint32_t size = 0;
uint64_t buff = 0;

void bufferInsert (int aux, int k) {
	for (uint64_t i = size; i < size+k; ++i) {
		buff |= (aux & (1 << i));
	}
}

void bufferReduce (int k) {
	buff >>= k;
}

bool bufferIsEnd (int y1, int y2) {
	return (y1 == 90) && (y2 == 90);
}

uint64_t bufferGetY (int k) {
	uint64_t y = 0;

	// this will read 7 bits
	for (uint64_t i = k; i < k+7; ++i) {
		y |= (buff & (1 << i));
	}

	return y;
}

uint64_t getX (uint64_t y1, uint64_t y2) {
	return (y1 * BASE) + y2;
}

void decode (FILE * input, FILE * output) {
	char aux;
	printf("Starting... ");

	while (true) {
		int k = fread(&aux, sizeof aux, 1, input);

		bufferInsert(aux, k);

		if (size < 14) continue;

		uint64_t y1 = bufferGetY(0);
		uint64_t y2 = bufferGetY(7);
		bufferReduce(14);

		if (bufferIsEnd(y1, y2)) break;

		uint64_t x = getX(y1, y2);

		printf("%llu\n", x);

		//write(output, x);
	}

	printf("Done!\n");
}

int main () {
	char filename_input[100];
	char filename_output[100];

	printf("Nome do arquivo: ");
	scanf("%100s", filename_input);

	FILE * input = fopen(filename_input, "rb");
	FILE * output = fopen(filename_output, "wb");

	decode(input, output);	

	return 0;
}