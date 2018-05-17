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

void to_b (Buffer* b_in, Buffer* b_out) {
	uint32_t y1, y2;
	uint64_t x;
	while(b_in->size >= 14) {
		y1 = extract(b_in, 7);
		y2 = extract(b_in, 7);
		x = (y1 * BASE) + y2;
		insert(b_out, x, 13);
	}	
}

void to_b91 (Buffer* b_in, Buffer* b_out) {
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
	uint8_t byte;
	uint32_t y1, y2, n;
	uint64_t y;

	in.buffer  = 0;
	in.size    = 0;
	out.buffer = 0;
	out.size   = 0;

	while (!feof(input)) {
		read(&in, 5, input);
		to_b(&in, &out);
		dump(&out, output);
	}

	printf("Done!\n");
}

void encode (FILE * input, FILE * output) {
	printf("Starting... ");

	Buffer in, out;
	uint8_t byte;
	uint32_t y1, y2, n;
	uint64_t y;

	// Initialize buffers
	in.buffer  = 0;
	in.size    = 0;
	out.buffer = 0;
	out.size   = 0;


	// Read bytes from input file to buffer, convert them to B91,
	// store them in output buffer and dump output buffer to output file
	while (!feof(input)) {
		read(&in, 5, input);
		to_b91(&in, &out);
		dump(&out, output);
	}

	if (in.size) { // Input file size is not multiple of 13 bits
		n = 13 - in.size;
		insert(&in, 0, n); // Pad input buffer with zeros so it has 13 bits
		insert(&in, 8191 + n, 13); // Indicates how many bits were used as padding
		to_b91(&in, &out);
		dump(&out, output);
	}


	// This sequence indicates the end of B91 bitstream
	insert(&out, 90, 7);
	insert(&out, 90, 7);
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
	
	printf("Aperte [1] para codificar para base91\n");
	printf("Aperte [2] para decodificar de base91\n");

	int op = 0;
	scanf("%d", op);

	switch (op) {
		case 1:
			encode(input, output);
			break;
		case 2:
			decode(input, output);			
			break;
		default:
			printf("Tente novamente!\n");
	}

	fclose(input);
	fclose(output);

	return 0;
}
