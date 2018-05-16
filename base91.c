#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define BASE 91

typedef struct {
	uint64_t buffer; // using bit in interger of size 64 bits to store information
	uint32_t size; // how many bit already stored
} Buffer;

// Insert k LSB from buff into b
void insert (Buffer* b, uint32_t buff, uint32_t k) {
	b->buffer <<= k;
	b->buffer |= buff & ((1 << k) - 1);
	b->size += k;
}

// Extract n MSB from b
uint64_t extract (Buffer* b, int n) {
	if (n > b->size) return 0;
	
	uint64_t shift = b->size - n;
	// this will read n bits
	uint64_t x = (b->buffer >> shift) & ((1UL << n) - 1);
	
	b->buffer &= (1UL << shift) - 1;
	b->size -= n;

	return x;
}

void read (Buffer* b, int nbytes, FILE* fp) {
	uint8_t byte;
	while (nbytes--) {
		if(!fread(&byte, 1, 1, fp)) return;
		insert(b, byte, 8);
	}
}

void dump (Buffer* b, FILE* fp) {
	uint8_t byte;
	while(b->size >= 8) {
		byte = extract(b, 8);
		fwrite(&byte, 1, 1, fp);
	}
}

void to_b91(Buffer* b_in, Buffer* b_out) {
	uint32_t y1, y2;
	uint64_t y;
	while(b_in->size >= 13) {
		y = extract(b_in, 13);
		y1 = y / 91;
		y2 = y % 91;
		insert(b_out, y1, 7);
		insert(b_out, y2, 7);
	}
}

// Get the file size
uint64_t getSize (FILE * fp) {
	uint64_t size;

	fseek(fp , 0 , SEEK_END);
	size = ftell (fp);
	rewind(fp);

	return size;
}

void decode (FILE * input, FILE * output) {
	printf("Starting... ");

	Buffer in, out;
	uint16_t aux_in, aux_out;
	uint32_t k, x, y1, y2;
	uint64_t filesize = getSize(input);
	
	while (true) {
		k = fread(&aux_in, 1, filesize, input); // read a byte

		insert(&in, aux_in, k);

		if (in.size < 14) continue;

		y1 = extract(&in, 7);
		y2 = extract(&in, 7);

		if (y1 == 90 && y2 == 90) break;

		aux_out = (y1 * BASE) + y2;
		insert(&out, aux_out, 13);
		
		while (out.size >= 8) {
			aux_out = extract(&out, 8);
			fwrite(&aux_out, 1, filesize, output); // print a byte
		}
	}

	printf("Done!\n");
}

void encode (FILE * input, FILE * output) {
	printf("Starting... ");

	Buffer in, out;
	uint8_t byte;
	uint32_t y1, y2, n;
	uint64_t y;

	in.buffer  = 0;
	in.size    = 0;
	out.buffer = 0;
	out.size   = 0;

	while (!feof(input)) {
		read(&in, 5, input);
		to_b91(&in, &out);
		dump(&out, output);
	}

	if (in.size) {
		n = 13 - in.size;
		insert(&in, 0, n);
		insert(&in, 8191 + n, 13);
		to_b91(&in, &out);
		dump(&out, output);
	}

	insert(&out, '=', 7);
	insert(&out, '=', 7);
	dump(&out, output);

	// File must have a whole number of bytes.
	if (out.size) {
		insert(&out, 0, 8 - out.size);
		dump(&out, output);
	}

	printf("Done!\n");
}

int main () {
	char filename_input[100];
	char filename_output[100];

	printf("Nome do arquivo a ser codificado: ");
	// scanf("%100s", filename_input);
	strcpy(filename_input, "entrada.txt");

	printf("\nNome do arquivo de saída: ");
	// scanf("%100s", filename_output);
	strcpy(filename_output, "saida.txt");

	FILE * input = fopen(filename_input, "rb");
	FILE * output = fopen(filename_output, "wb");
	
	encode(input, output);
	//decode(input, output);

	fclose(input);
	fclose(output);

	return 0;
}
