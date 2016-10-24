#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define TAMANHO_BUFFER 100000
#define false 0
#define true 1

void charIncrement(char c, long long int freq[]);
void escreveFrequencia(FILE *fp, long long int freq[]);

int main(int argc, char *argv[]) {
	// Inicialização de variáveis da main
	FILE *input, *output;
	long long int freq[256] = {0};
	double inicio, fim;
	char buffer[TAMANHO_BUFFER] = {0};
	int arquivoEncerrado = false, charRead, i;
	
	// Validação da entrada do programa
	if (argc < 3) {
		printf("Erro: número de argumentos insuficiente.\n");
		printf("Uso: %s <arquivo de entrada> <arquivo de saída>\n", argv[0]);
		exit(1); }
	
	GET_TIME(inicio);

	// Validação e inicialização do arquivo de entrada
	input = fopen(argv[1], "r");
	if (input == NULL) {
		printf("Erro: incapaz de abrir arquivo de entrada.\n");
		exit(1); }

	// Validação e inicialização do arquivo de saída
	output = fopen(argv[2], "w");
	if (output == NULL) {
		printf("Erro: incapaz de abrir arquivo de saída.\n");
		exit(1); }

	GET_TIME(fim);
	printf(">> Tempo de inicialização: %.8lf\n", fim - inicio);
	GET_TIME(inicio);

	// Processamento do arquivo de entrada
	while (arquivoEncerrado == false) {
		charRead = fread(buffer, sizeof(char), TAMANHO_BUFFER, input);

		if (charRead != TAMANHO_BUFFER) arquivoEncerrado = true;
		for (i = 0; i < charRead; i++)
			charIncrement(buffer[i], freq);
	}

	// Escreve no arquivo de saída
	escreveFrequencia(output, freq);
	
	GET_TIME(fim);
	printf(">> Tempo de processamento: %.8lf\n", fim - inicio);
	GET_TIME(inicio);

	// Desalocação de memória
	fclose(input);
	fclose(output);

	GET_TIME(fim);
	printf(">> Tempo de finalização: %.8lf\n", fim - inicio);
}

void charIncrement(char c, long long int freq[]) {
	if ((c >= '?' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= ';') 
	|| (c >= '#' && c <= '&') || c == '!' || c == '.' || c == '_' || c == '-' || c == '(' || c == ')')
		freq[c]++;
}

// Write the character frequencies to the output file
void escreveFrequencia(FILE *fp, long long int freq[]) {
	long long int count;
	int c;

	fprintf(fp, "Caractere, Qtde\n");
	for (c = 0; c < 256; c++)
		if ((count = freq[c]) != 0)
			fprintf(fp, "%c, %lld\n", c, count);
}