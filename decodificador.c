#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BASE 91

typedef struct {
  uint64_t buffer; // using bit in interger of size 64 bits to store information
  uint32_t size; // how many bit already stored
} Buffer;

// Concat at the end of a buffer, k bits comming from another buffer.
void insert (Buffer* b, uint32_t buff, uint32_t k) {
	for (uint64_t i = b->size; i < b->size+k; ++i) {
		b->buffer |= (buff & (1 << i));
	}
	
	b->size += k;
}

// Extract from the beggining of the buffer n bits
uint64_t extract (Buffer* b, int n) {
  if (n > 64) return 0;
  
	uint64_t x = 0;

	// this will read n bits
	for (uint64_t i = 0; i < n; ++i) {
		x |= (b->buffer & (1 << i));
	}
	
	b->buffer >>= n;
	b->size -= n;

	return x;
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
  uint64_t filesize = getSize(input);
	
	while (true) {
		int k = fread(&aux_in, 1, filesize, input); // read a byte

		insert(&in, aux_in, k);

		if (in.size < 14) continue;

		uint64_t y1 = extract(&in, 7);
		uint64_t y2 = extract(&in, 7);

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
