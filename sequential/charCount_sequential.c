#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define _FILE_OFFSET_BITS 64 // Ensures that fsize is able to handle files larger than 2GB

void charIncrement(char c, long long int freq[]);
void writeCount(FILE *fp, long long int freq[]);
off_t fsize(const char *fp);
void printProgress(long long int currentCharCount, long long int size);

int main(int argc, char *argv[]) {
	// Initialize variables and data structures
	FILE *input, *output;
	long long int freq[256] = {0}, size, currentCharCount = 1;
	char c;

	// Validate command-line arguments
	if(argc < 3) {
		printf("Error: Too few arguments\n");
		printf("Usage: %s <input file> <output file>\n", argv[0]);

		exit(1);
	}
	
	// Open input file
	input = fopen(argv[1], "r");
	if(input == NULL) {
		printf("Error: enable to open input file\n");
		exit(1);
	}
	// Determine file size
	size = (long long int) fsize(argv[1]);
	if(size == -1) {
		printf("Enable to determine file size\n");
		exit(1);
	}
	// Parse input file contants
	while((c = getc(input)) != EOF) {
		// Print progress
		printProgress(currentCharCount++, size);

		charIncrement(c, freq);
	}
	printf("\n");

	//Open and write to output file
	output = fopen(argv[2], "w");
	if(output == NULL) {
		printf("Error: enable to open output file\n");
		exit(1);
	}
	
	writeCount(output, freq);

	// Free memory
	fclose(input);
	fclose(output);
}

void charIncrement(char c, long long int freq[]) {
	if((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
		|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')')
		freq[c]++;
}

// Write the character frequencies to the output file
void writeCount(FILE *fp, long long int freq[]) {
	long long int count;
	int c;

	fprintf(fp, "Caractere, Qtde\n");
	for(c = 0; c < 256; c++)
		if((count = freq[c]) != 0)
			fprintf(fp, "%c, %lld\n", c, count);
}

// Determine file size
off_t fsize(const char *fp) {
	struct stat st;

	if(stat(fp, &st) == 0)
		return st.st_size;

	return -1;
} 
void printProgress(long long int currentCharCount, long long int size) {
	printf("\r[%3d%%]", currentCharCount*100/size);
}

// Maps valid characters to the index of its frequency count (Indirect adressing option)
/*int indexof(char c) {
	switch(c) {
		case 'a': return 0;
		case 'b': return 1;
		case 'c': return 2;
		case 'd': return 3;
		case 'e': return 4;
		case 'f': return 5;
		case 'g': return 6;
		case 'h': return 7;
		case 'i': return 8;
		case 'j': return 9;
		case 'k': return 10;
		case 'l': return 11;
		case 'm': return 12;
		case 'n': return 13;
		case 'o': return 14;
		case 'p': return 15;
		case 'q': return 16;
		case 'r': return 17;
		case 's': return 18;
		case 't': return 19;
		case 'u': return 20;
		case 'v': return 21;
		case 'w': return 22;
		case 'x': return 23;
		case 'y': return 24;
		case 'z': return 25;
		case 'A': return 26;
		case 'B': return 27;
		case 'C': return 28;
		case 'D': return 29;
		case 'E': return 30;
		case 'F': return 31;
		case 'G': return 32;
		case 'H': return 33;
		case 'I': return 34;
		case 'J': return 35;
		case 'K': return 36;
		case 'L': return 37;
		case 'M': return 38;
		case 'N': return 39;
		case 'O': return 40;
		case 'P': return 41;
		case 'Q': return 42;
		case 'R': return 43;
		case 'S': return 44;
		case 'T': return 45;
		case 'U': return 46;
		case 'V': return 47;
		case 'W': return 48;
		case 'X': return 49;
		case 'Y': return 50;
		case 'Z': return 51;
		case '0': return 52;
		case '1': return 53;
		case '2': return 54;
		case '3': return 55;
		case '4': return 56;
		case '5': return 57;
		case '6': return 58;
		case '7': return 59;
		case '8': return 60;
		case '9': return 61;
		case '?': return 62;
		case '!': return 63;
		case '.': return 64;
		case ':': return 65;
		case ';': return 66;
		case '_': return 67;
		case '-': return 68;
		case '(': return 69;
		case ')': return 70;
		case '@': return 71;
		case '%': return 72;
		case '&': return 73;
		case '$': return 74;
		case '#': return 75;
	}
}*/