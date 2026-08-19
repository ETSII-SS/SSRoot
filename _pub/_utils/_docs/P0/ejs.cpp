

...

// Sustituir el contenido del archivo .cpp por esto:
#include <stdio.h>
#include <locale.h>
 
int main(int argc, char* argv[], char* envp[])
{
	// Necesario para imprimir caracteres específicos del español
	setlocale(LC_ALL, "Spanish"); // Necesita #include <locale.h>
 
	HolaMundo_ComoC();
 
	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();  // printf y getchar=> necesitan #include <stdio.h>
	return 0;
}
static void HolaMundo_ComoC() {
	printf("\nHola mundo, desde C/C++\n");
}


...



#define ESPERA_MS      (500)
#define _1MHzEnHerzios (1000000.0)

void ImprimeGhz() {
    int    antes, ahora;
    double VelocidadCPUGhz;

    antes = __rdtsc();          // leo el contador de ciclos del procesador
    Sleep(ESPERA_MS);           // espero ESPERA_MS milisegundos
    ahora = __rdtsc();          // lo vuelvo a leer

    VelocidadCPUGhz = (ahora - antes) / (ESPERA_MS * _1MHzEnHerzios);
    printf("\nLa frecuencia de la CPU es de %.2f GHz", VelocidadCPUGhz);
}







