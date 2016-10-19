#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void charIncrement(char c, long long int freq[]);
void writeCount(FILE *fp, long long int freq[]);
void* executa(void* tid);

int main(int argc, char *argv[]) {
	// Initialize variables and data structures
	FILE *input, *output;
	long long int freq[256] = {0};
	char c;

	// Validate command-line arguments
	if (argc < 4) {
		printf("Error: too few arguments\n");
		printf("Usage: %s <input file> <output file> <number of threads>\n", argv[0]);
		exit(1);
	}
	
	// Open input file
	input = fopen(argv[1], "r");
	if (input == NULL) {
		printf("Error: unable to open input file\n");
		exit(1);
	}

	//Open and write to output file
	output = fopen(argv[2], "w");
	if(output == NULL) {
		printf("Error: unable to open output file\n");
		exit(1);
	}

	// Initialize threads
	int nthreads = atoi(argv[3]);
	pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * nthreads);
	if (threads == NULL) {
		printf("Error: could not allocate memory for threads\n");
		exit(1);
	}

	int* tid;
	for (int i = 0; i < nthreads; i++) {
		tid = (int*) malloc(sizeof(int));
		*tid = i;

		if (pthread_create(&threads[i], NULL, executa, (void*) tid)) {
			printf("Error: could not create threads\n");
			exit(1);
		}
	}

	for (int i = 0; i < nthreads; i++) {
	     if (pthread_join(threads[i], NULL)) {
	        printf("Error: could not join threads\n");
	        exit(1);
	    }
	}

	// Parse input file contants
	while((c = getc(input)) != EOF)
		charIncrement(c, freq);
	
	writeCount(output, freq);

	// Free memory
	fclose(input);
	fclose(output);
	free(threads);

	//Exit
	pthread_exit(NULL);
}

void* executa(void* tid) {
	int id = * (int*) tid;
	printf("Thread ID: %d\n", id);
	pthread_exit(NULL);
}

void charIncrement(char c, long long int freq[]) {
	if((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
		|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')')
		freq[c]++;
}

// Write the character frequencies to the output file
void writeCount(FILE *fp, long long int freq[]) {
	int c; long long int count;

	fprintf(fp, "Caractere, Qtde\n");
	for(c = 0; c < 256; c++)
		if((count = freq[c]) != 0)
			fprintf(fp, "%c, %lld\n", c, count);
}