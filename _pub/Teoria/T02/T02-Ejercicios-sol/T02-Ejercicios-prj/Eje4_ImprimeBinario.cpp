// Ejemplo basado en la primera sesión de la práctica 1.
#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <locale.h>
#include <time.h>

size_t ImprimeBinario(int valor, int bits);
void Ej1_Inicial();


int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Spanish");  

	Ej1_Inicial();

	printf("\nPulse tecla RETORNO para terminar\n");
	(void)getchar();
	return 0;
}


#define NRO_BYTES sizeof(i)
void Ej1_Inicial() {
	clock_t inicio = clock();  // inicia medición de tiempo

	//printf("Tamaño de i: %d bytes\n", NRO_BYTES);
	short int i;
	for (i = 1; i < 5000; i++) {
		printf("%d:0x%02x -> ", i, i);
		ImprimeBinario(i, NRO_BYTES * 8);
		printf("\n");
	}

	clock_t fin = clock();  // finaliza medición de tiempo
	double milisegundos = (double)(fin - inicio) / CLOCKS_PER_SEC * 1000;

	printf(__FUNCTION__ ": Tiempo: %.3f ms\n", milisegundos);
	printf("Tamaño de i: %d bytes\n", NRO_BYTES);
}


size_t ImprimeBinario(int valor, int bits)
{
	for (int i = bits - 1; i >= 0; i--)
	{
		printf("%d", (valor >> i) & 1);
		// Imprime un espacio para separar cada nibble
		if (i % 4 == 0)
			printf(" ");
		// Imprime un espacio adicional para separara cada byte
		if (i % 8 == 0)
			printf(" ");
	}
	return bits;
}
