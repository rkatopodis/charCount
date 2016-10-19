#include <stdio.h>
#include <stdlib.h>

void charIncrement(char c, long long int freq[]);
void writeCount(FILE *fp, long long int freq[]);

int main(int argc, char *argv[]) {
	// Initialize variables and data structures
	FILE *input, *output;
	long long int freq[256] = {0};
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
	// Parse input file contants
	while((c = getc(input)) != EOF)
		charIncrement(c, freq);

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
	int c, count;

	fprintf(fp, "Caractere, Qtde\n");
	for(c = 0; c < 256; c++)
		if((count = freq[c]) != 0)
			fprintf(fp, "%c, %lld\n", c, count);
}