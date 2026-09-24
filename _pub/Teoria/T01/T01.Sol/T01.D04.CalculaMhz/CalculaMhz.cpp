#include <stdio.h>
#include <Windows.h>

// Funciona cambiando ESPERA_MS a 2000 ms?
#define ESPERA_MS (500)
#define _1MHzEnHerzios (1000*1000)

void ImprimeGhz() {
	long int antes, ahora;
	double VelocidadCPUGhz;

	antes = __rdtsc(); // Leo el valor del contador RDTSC y lo guardo
	Sleep(ESPERA_MS); // Espero un tiempo determinado (en milisegundos)
	ahora = __rdtsc();// Leo el valor del contador ahora

	VelocidadCPUGhz = (double)(ahora - antes) / (ESPERA_MS * _1MHzEnHerzios);
	printf("\nLa frecuencia en Hz es: %f", VelocidadCPUGhz);
}


#include <stdio.h>
#include <locale>

int main(int argc, char* argv[], char* envp[])
{
	// Necesario para imprimir caracteres específicos del español
	setlocale(LC_ALL, "Spanish"); // Necesita #include <locale.h>

	ImprimeGhz();

	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();  // printf y getchar=> necesitan #include <stdio.h>
	return 0;
}







