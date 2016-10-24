#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

#define BUFFER_SIZE 50000
#define FALSE 0
#define TRUE 1

FILE *input;
int endOfFile = FALSE;
long long int** freq_partials;
long long int freq_total[256] = {0};

pthread_mutex_t mutex;

void charIncrement(char c, long long int freq[]);
void *thread_counter(void *id);
void writeCount(int nthreads, long long int freq_total[], long long int **freq_partials, FILE *output);


int main(int argc, char *argv[]) {
	// Initialize variables and data structures
	int i, nthreads;
	double inicio, fim;
	pthread_t* threads;
	FILE *output;

	GET_TIME(inicio);
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

	nthreads = atoi(argv[3]);

	// Initialize partial frequencies data structure
	freq_partials = (long long int**) malloc(sizeof(long long int*) * nthreads);
	for (i = 0; i < nthreads; i++) {
		freq_partials[i] = (long long int*) malloc(sizeof(long long int) * 256);
		for (int c = 0; c < 256; c++) {
			freq_partials[i][c] = 0;
		}
	}

	GET_TIME(fim);
	printf(">> Tempo de inicialização: %.8lf\n", fim - inicio);
	GET_TIME(inicio);

	pthread_mutex_init(&mutex, NULL);

	//Initialize threads
	threads = (pthread_t*) malloc(sizeof(pthread_t) * nthreads);
	if (threads == NULL) {
		printf("Error: could not allocate memory for threads\n");
		exit(1);
	}

	int* tid;
	for (i = 0; i < nthreads; i++) {
		tid = (int*) malloc(sizeof(int));
		*tid = i;

		if (pthread_create(&threads[i], NULL, thread_counter, (void*) tid)) {
			printf("Error: could not create threads\n");
			exit(1);
		}
	}

	//Join threads
	for (i = 0; i < nthreads; i++) {
	     if (pthread_join(threads[i], NULL)) {
	        printf("Error: could not join threads\n");
	        exit(1);
	    }
	}

	GET_TIME(fim);
	printf(">> Tempo de processamento: %.8lf\n", fim - inicio);
	GET_TIME(inicio);
	
	writeCount(nthreads, freq_total, freq_partials, output);

	// Free memory
	fclose(input);
	fclose(output);
	free(threads);
	free(freq_partials);
	
	pthread_mutex_destroy(&mutex);

	GET_TIME(fim);
	printf(">> Tempo de finalização: %.8lf\n", fim - inicio);
	
	//Exit
	pthread_exit(NULL);
}

void charIncrement(char c, long long int freq[]) {
	if((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
		|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')')
		freq[c]++;
}

void *thread_counter(void *id) {
	long long int buffer[BUFFER_SIZE];
	int i, tid, charRead;
	
	id = *(int *) id;

	while(endOfFile == FALSE) {
		pthread_mutex_lock(&mutex);
		charRead = fread(buffer, sizeof(char), BUFFER_SIZE, input);
		if(charRead != BUFFER_SIZE) endOfFile = TRUE;

		pthread_mutex_unlock(&mutex);
		for(i = 0; i < charRead; i++)
			charIncrement(buffer[i], freq_partials[tid]);

	}
	pthread_mutex_unlock(&mutex);
}

// Write the character frequencies to the output file
void writeCount(int nthreads, long long int freq_total[], long long int **freq_partials, FILE *output) {
	int c, i, d; long long int count;

	for (i = 0; i < nthreads; i++) {
		printf("Thread #%d - Resultados:\n", i + 1);
		for (d = 0; d < 256; d++) {
			if (freq_partials[i][d] != 0) printf("%c, %lld\n", d, freq_partials[i][d]);
			freq_total[d] += freq_partials[i][d];
		}
	}

	fprintf(output, "Caractere, Qtde\n");
	for (c = 0; c < 256; c++)
		if ((count = freq_total[c]) != 0)
			fprintf(output, "%c, %lld\n", c, count);
}