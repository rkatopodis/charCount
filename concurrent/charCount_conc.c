#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

#define _FILE_OFFSET_BITS 64 // Ensures that fsize is able to handle files larger than 2GB
#define VERDE 0
#define AMARELO 1
#define VERMELHO 2

void charIncrement(char c);
void writeCount();
void printBuffer();
void* produtor(void* tid);
void* consumidor(void* tid);

FILE *input, *output;
long long int freq[256] = {0};
int bufferSize, bufferCount = 0;
int fileEnded = 0, nthreads_cons;
pthread_cond_t cond_cons, cond_prod; pthread_mutex_t mutex;
char* buffer;

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

	//Initialize character buffer
	bufferSize = 10; //this should be a parameter
	buffer = (char*) malloc(sizeof(char) * bufferSize);

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
	nthreads_cons = atoi(argv[3]);
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
	printf(">> Thread consumidora #%d iniciada\n", id);
	int bloco = bufferSize / nthreads_cons;
	int inicio = id * bloco;
	int fim = inicio + bloco;
	if (id == nthreads_cons - 1)
		fim = bufferSize;
	printf("inicio: %d, fim: %d, bloco: %d, thread: %d\n", inicio, fim, bloco, id + 1);

	while (1) {
		//printf("C1\n");
		pthread_mutex_lock(&mutex);
		while (bufferCount != bufferSize && fileEnded == 0) {
			pthread_cond_wait(&cond_cons, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		//printf("C2\n");

		for (int i = inicio; i < fim && i < bufferCount; i++) {
			charIncrement(buffer[i]);
			buffer[i] = '_';
			printBuffer();
		}
		
		if (fileEnded == 1) break;
		//printf("C3\n");
		pthread_mutex_lock(&mutex);
		bufferCount -= bloco;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond_prod);
		//printf("C4\n");
	}

	printf(">> Thread consumidora #%d encerrada\n", id);
	pthread_exit(NULL);
}

void* produtor(void* tid) {
	char c;
	printf(">> Thread produtora iniciada\n");
	while (1) {
		//printf("P1\n");
		pthread_mutex_lock(&mutex);
		while (bufferCount != 0) {
			pthread_cond_wait(&cond_prod, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		//printf("P2\n");

		for (int i = 0; i < bufferSize; i++) {
			c = getc(input);
			if (c == EOF) {
				printf(">> Input file has been read.\n");
				fileEnded = 1;
				bufferCount = i;
				pthread_cond_broadcast(&cond_cons);
				pthread_exit(NULL); }
			buffer[i] = c;
		}

		//printf("P3\n");
		printBuffer();
		pthread_mutex_lock(&mutex);
		bufferCount = bufferSize;
		pthread_cond_broadcast(&cond_cons);
		pthread_mutex_unlock(&mutex);
		//printf("P4\n");
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

void charIncrement(char c) {
	if((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
		|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')')
		freq[c]++;
}

// Write the character frequencies to the output file
void writeCount() {
	int c; long long int count;

	fprintf(output, "Caractere, Qtde\n");
	for (c = 0; c < 256; c++)
		if ((count = freq[c]) != 0)
			fprintf(output, "%c, %lld\n", c, count);
}