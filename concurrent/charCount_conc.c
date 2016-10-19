#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

#define _FILE_OFFSET_BITS 64 // Ensures that fsize is able to handle files larger than 2GB
#define VERDE 0
#define AMARELO 1
#define VERMELHO 2
#define true 1
#define false 0

void charIncrement(char c, int id);
void writeCount();
void printBuffer();
void* produtor(void* tid);
void* consumidor(void* tid);

FILE *input, *output;
int bufferSize, bufferCount = 0, lastCharacter;
int fileEnded = 0, nthreads_cons, threads_cons_left = nthreads_cons;
int* isBlocoFull;
pthread_cond_t cond_cons, cond_prod; pthread_mutex_t mutex;
char* buffer;
long long int** freq;
long long int freq_total[256] = {0};

int main(int argc, char *argv[]) {
	// Initialize variables and data structures
	double inicio, fim;

	GET_TIME(inicio);
	// Validate command-line arguments
	if (argc < 4) {
		printf("Error: too few arguments\n");
		printf("Usage: %s <input file> <output file> <number of threads consumidoras>\n", argv[0]);
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

	nthreads_cons = atoi(argv[3]);
	//Initialize character buffer
	bufferSize = 10; //this should be a parameter
	buffer = (char*) malloc(sizeof(char) * bufferSize);
	isBlocoFull = (int*) malloc(sizeof(char) * nthreads_cons);
	for (int i = 0; i < nthreads_cons; i++)
		isBlocoFull[i] = false;

	freq = (long long int**) malloc(sizeof(long long int*) * nthreads_cons);
	for (int i = 0; i < nthreads_cons; i++) {
		freq[i] = (long long int*) malloc(sizeof(long long int) * 256);
		for (int c = 0; c < 256; c++) {
			freq[i][c] = 0;
		}
	}

	GET_TIME(fim);
	printf(">> Tempo de inicialização: %.8lf\n", fim - inicio);
	GET_TIME(inicio);

	//Initialize thread produtora
	pthread_t thread_prod;
	pthread_cond_init(&cond_cons, NULL);
	pthread_cond_init(&cond_prod, NULL);
	pthread_mutex_init(&mutex, NULL);
	if (pthread_create(&thread_prod, NULL, produtor, NULL)) {
		printf("Error: could not create thread produtora\n");
		exit(1);
	}

	//Initialize threads consumidoras
	pthread_t* threads_cons = (pthread_t*) malloc(sizeof(pthread_t) * nthreads_cons);
	if (threads_cons == NULL) {
		printf("Error: could not allocate memory for threads consumidoras\n");
		exit(1);
	}

	int* tid;
	for (int i = 0; i < nthreads_cons; i++) {
		tid = (int*) malloc(sizeof(int));
		*tid = i;

		if (pthread_create(&threads_cons[i], NULL, consumidor, (void*) tid)) {
			printf("Error: could not create threads consumidoras\n");
			exit(1);
		}
	}

	//Join thread produtora
	if (pthread_join(thread_prod, NULL)) {
		printf("Error: could not join thread produtora\n");
		exit(1);
	}

	//Join threads consumidoras
	for (int i = 0; i < nthreads_cons; i++) {
	     if (pthread_join(threads_cons[i], NULL)) {
	        printf("Error: could not join threads consumidoras\n");
	        exit(1);
	    }
	}

	GET_TIME(fim);
	printf(">> Tempo de processamento: %.8lf\n", fim - inicio);
	GET_TIME(inicio);
	
	// Parse input file contants
	//while((c = getc(input)) != EOF)
	//	charIncrement(c, freq);
	
	writeCount();

	// Free memory
	fclose(input);
	fclose(output);
	free(threads_cons);
	free(freq);
	pthread_cond_destroy(&cond_cons);
	pthread_cond_destroy(&cond_prod);
	pthread_mutex_destroy(&mutex);

	GET_TIME(fim);
	printf(">> Tempo de finalização: %.8lf\n", fim - inicio);
	
	//Exit
	pthread_exit(NULL);
}

void* consumidor(void* tid) {
	int id = * (int*) tid;
	printf(">> Thread consumidora #%d iniciada\n", id + 1);
	int bloco = bufferSize / nthreads_cons;
	int inicio = id * bloco;
	int fim = inicio + bloco;
	if (id == nthreads_cons - 1)
		fim = bufferSize;
	printf("inicio: %d, fim: %d, bloco: %d, thread: %d\n", inicio, fim, bloco, id + 1);

	while (1) {
		printf("T%d - C1\n", id + 1);
		pthread_mutex_lock(&mutex);
		while (isBlocoFull[id] == false && fileEnded == 0) {
			pthread_cond_wait(&cond_cons, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		printf("T%d - C2\n", id + 1);

		for (int i = inicio; i < fim && i < lastCharacter; i++) {
			charIncrement(buffer[i], id);
			buffer[i] = '_';
			//printf("T%d: ", id + 1); printBuffer();
		}
		
		if (fileEnded == 1) break;
		printf("T%d - C3\n", id + 1);
		pthread_mutex_lock(&mutex);
		isBlocoFull[id] = false;
		bufferCount -= bloco;
		if (bufferCount == 0) pthread_cond_signal(&cond_prod);
		printf("bufferCount: %d\n", bufferCount);
		pthread_mutex_unlock(&mutex);
		printf("T%d - C4\n", id + 1);
	}

	printf(">> Thread consumidora #%d encerrada\n", id + 1);
	pthread_exit(NULL);
}

void* produtor(void* tid) {
	char c;
	printf(">> Thread produtora iniciada\n");
	while (1) {
		printf("P1\n");
		pthread_mutex_lock(&mutex);
		while (bufferCount != 0) {
			pthread_cond_wait(&cond_prod, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		printf("P2\n");

		for (int i = 0; i < bufferSize; i++) {
			c = getc(input);
			if (c == EOF) {
				printf(">> Input file has been read.\n");
				fileEnded = 1;
				lastCharacter = i;
				pthread_cond_broadcast(&cond_cons);
				pthread_exit(NULL); }
			buffer[i] = c;
		}

		printf("P3\n");
		//printBuffer();
		pthread_mutex_lock(&mutex);
		lastCharacter = bufferSize;
		bufferCount = bufferSize;
		for (int i = 0; i < nthreads_cons; i++)
			isBlocoFull[i] = true;
		printf("bufferCount: %d\n", bufferCount);
		pthread_cond_broadcast(&cond_cons);
		pthread_mutex_unlock(&mutex);
		printf("P4\n");
	}
}

void printBuffer() {
	printf("[");
	for (int i = 0; i < bufferSize; i++) {
		printf("%c", buffer[i]);
		if (i != bufferSize - 1)
			printf(", ");
	}
	printf("]\n");
}

void charIncrement(char c, int id) {
	if((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
		|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')') {
		//printf("freq[%d][%c] = %lld\n", id, c, freq[id][c]);
		freq[id][c]++;
	}
}

// Write the character frequencies to the output file
void writeCount() {
	int c; long long int count;

	for (int i = 0; i < nthreads_cons; i++) {
		printf("Thread #%d - Resultados:\n", i + 1);
		for (int d = 0; d < 256; d++) {
			if (freq[i][d] != 0) printf("%c, %lld\n", d, freq[i][d]);
			freq_total[d] += freq[i][d];
		}
	}

	fprintf(output, "Caractere, Qtde\n");
	for (c = 0; c < 256; c++)
		if ((count = freq_total[c]) != 0)
			fprintf(output, "%c, %lld\n", c, count);
}