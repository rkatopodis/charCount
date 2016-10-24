#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include "timer.h"

#define TAMANHO_BUFFER 200000
#define true 1
#define false 0

off_t fsize(const char *fp);
void charIncrement(char c, int id);
void escreveFrequencia();
void* produtor(void* tid);
void* consumidor(void* tid);

FILE *input, *output;
int caracteresLidos;
int nthreads_cons, cons_restantes = 0;
int arquivoEncerrado = false;
int* blocoCheio;

pthread_cond_t cond_cons, cond_prod;
pthread_mutex_t mutex;
char* buffer;

long long int** freq_parcial;
long long int freq_total[256] = {0};

int main(int argc, char *argv[]) {
	// Inicialização de variáveis da main
	double timeStart, timeEnd; //variáveis para mensuração de tempo
	long long int tamanhoArquivo; //tamanho do arquivo de entrada
	pthread_t thread_prod; //thread produtora
	pthread_t* threads_cons; //threads consumidoras
	int* tid; //vetor de array de thread IDs
	int i; //contador genérico

	GET_TIME(timeStart);

	// Validação da entrada do programa
	if (argc < 4) {
		printf("Erro: número de argumentos incorreto.\n");
		printf("Uso: %s <arquivo de entrada> <arquivo de saída> <número de threads consumidoras>\n", argv[0]);
		exit(1); }
	
	// Validação e inicialização do arquivo de entrada
	input = fopen(argv[1], "r");
	if (input == NULL) {
		printf("Erro: incapaz de abrir arquivo de entrada.\n");
		exit(1); }
	tamanhoArquivo = (long long int) fsize(argv[1]);

	// Validação e inicialização do arquivo de saída
	output = fopen(argv[2], "w");
	if (output == NULL) {
		printf("Erro: incapaz de abrir arquivo de saída.\n");
		exit(1); }

	// Validação da entrada do número de threads
	nthreads_cons = atoi(argv[3]);
	blocoCheio = (int*) malloc(sizeof(int) * nthreads_cons);
	for (i = 0; i < nthreads_cons; i++)
		blocoCheio[i] = false;

	// Inicialização do vetor de frequências parciais
	freq_parcial = (long long int**) malloc(sizeof(long long int*) * nthreads_cons);
	for (i = 0; i < nthreads_cons; i++) {
		freq_parcial[i] = (long long int*) malloc(sizeof(long long int) * 256);
		for (int c = 0; c < 256; c++)
			freq_parcial[i][c] = 0;
	}

	// Inicialização do tamanho do buffer, baseado no tamanho do arquivo de entrada
	buffer = (char*) malloc(sizeof(char) * TAMANHO_BUFFER);

	GET_TIME(timeEnd);
	printf(">> Tempo de inicialização: %.8lf\n", timeEnd - timeStart);
	GET_TIME(timeStart);

	// Inicialização das variáveis de condição e exclusão mútua
	pthread_cond_init(&cond_cons, NULL);
	pthread_cond_init(&cond_prod, NULL);
	pthread_mutex_init(&mutex, NULL);

	// Inicialização das thread produtora
	if (pthread_create(&thread_prod, NULL, produtor, NULL)) {
		printf("Erro: incapaz de criar thread produtora\n");
		exit(1);
	}

	// Inicialização das threads consumidoras
	threads_cons = (pthread_t*) malloc(sizeof(pthread_t) * nthreads_cons);
	if (threads_cons == NULL) {
		printf("Erro: incapaz de alocar memória para thread produtora\n");
		exit(1); }

	for (i = 0; i < nthreads_cons; i++) {
		tid = (int*) malloc(sizeof(int));
		*tid = i;

		if (pthread_create(&threads_cons[i], NULL, consumidor, (void*) tid)) {
			printf("Erro: incapaz de criar threads consumidoras\n");
			exit(1); }
	}

	// Encerramento da thread produtora
	if (pthread_join(thread_prod, NULL)) {
		printf("Erro: incapaz de encerrar thread produtora\n");
		exit(1); }

	// Encerramento das threads consumidoras
	for (i = 0; i < nthreads_cons; i++) {
	     if (pthread_join(threads_cons[i], NULL)) {
	        printf("Erro: incapaz de encerrar threads consumidoras\n");
	        exit(1); }
	}

	GET_TIME(timeEnd);
	printf(">> Tempo de processamento: %.8lf\n", timeEnd - timeStart);
	GET_TIME(timeStart);
	
	escreveFrequencia();

	// Desalocação de memória alocada ao longo da execução
	fclose(input);
	fclose(output);
	free(threads_cons);
	pthread_cond_destroy(&cond_cons);
	pthread_cond_destroy(&cond_prod);
	pthread_mutex_destroy(&mutex);

	for (i = 0; i < nthreads_cons; i++)
		free(freq_parcial[i]);
	free(freq_parcial);

	GET_TIME(timeEnd);
	printf(">> Tempo de finalização: %.8lf\n", timeEnd - timeStart);
	
	// Encerramento do programa
	pthread_exit(NULL);
}

void* consumidor(void* tid) {
	int id = * (int*) tid;
	int i;
	
	printf(">> Thread consumidora #%d iniciada.\n", id + 1);
	while (1) {
		// Thread consumidora entra em espera enquanto não houver nada para ela consumir
		// ou o arquivo não tiver encerrado
		pthread_mutex_lock(&mutex);
		while (blocoCheio[id] == false && arquivoEncerrado == false)
			pthread_cond_wait(&cond_cons, &mutex);
		pthread_mutex_unlock(&mutex);

		// Divisão do buffer em blocos e atribuição do próprio bloco
		int bloco = caracteresLidos / nthreads_cons;
		int inicio = id * bloco;
		int fim = inicio + bloco;
		if (id == nthreads_cons - 1)
			fim = caracteresLidos;
		bloco = fim - inicio;

		// Incremento dos caracteres
		for (i = inicio; i < fim; i++)
			charIncrement(buffer[i], id);
		
		// Encerra o funcionamento caso o arquivo tenha sido completamente lido
		if (arquivoEncerrado == true) break;

		// Prepara para se bloquear e desbloqueia a thread produtora caso
		// seja a última thread consumidora
		pthread_mutex_lock(&mutex);
		blocoCheio[id] = false;
		cons_restantes--;
		if (cons_restantes == 0) pthread_cond_signal(&cond_prod);
		pthread_mutex_unlock(&mutex);
	}

	printf(">> Thread consumidora #%d encerrada.\n", id + 1);
	pthread_exit(NULL);
}

void* produtor(void* tid) {
	printf(">> Thread produtora iniciada.\n");
	while (1) {
		// Thread produtora se bloqueia caso haja alguma thread consumidora 
		// restante em execução		
		pthread_mutex_lock(&mutex);
		while (cons_restantes != 0)
			pthread_cond_wait(&cond_prod, &mutex);
		pthread_mutex_unlock(&mutex);

		// Preenche o buffer com TAMANHO_BUFFER caracteres do arquivo de entrada.
		// Caso o arquivo de entrada tenha sido inteiramente lido, inicia o
		// encerramento da aplicação.
		int aux = fread (buffer, sizeof(char), TAMANHO_BUFFER, input);
		if (aux != TAMANHO_BUFFER) {
			printf(">> Thread produtora encerrada.\n");
			caracteresLidos = aux;
			arquivoEncerrado = true;
			pthread_cond_broadcast(&cond_cons);
			pthread_exit(NULL);
		}

		// Reinicia as variáveis para o funcionamento das threads consumidoras
		pthread_mutex_lock(&mutex);
		caracteresLidos = TAMANHO_BUFFER;
		cons_restantes = nthreads_cons;
		for (int i = 0; i < nthreads_cons; i++)
			blocoCheio[i] = true;
		pthread_cond_broadcast(&cond_cons);
		pthread_mutex_unlock(&mutex);
	}
}

// Rotina de incremento de frequência de caracteres
void charIncrement(char c, int id) {
	if((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
	|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')')
		freq_parcial[id][c]++;
}

// Escreve a frequência dos caracteres no arquivo de saída
void escreveFrequencia() {
	int i, d; long long int count;

	// Soma das frequências parciais
	for (i = 0; i < nthreads_cons; i++)
		for (d = 0; d < 256; d++)
			freq_total[d] += freq_parcial[i][d];

	fprintf(output, "Caractere, Qtde\n");
	for (d = 0; d < 256; d++)
		if ((count = freq_total[d]) != 0)
			fprintf(output, "%c, %lld\n", d, count);
}

// Determina o tamanho do arquivo de entrada
off_t fsize(const char *fp) {
	struct stat st;

	if(stat(fp, &st) == 0)
		return st.st_size;

	return -1;
} 