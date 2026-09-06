...

// Copiar esto al mismo archivo donde está main
#define NRO_BYTES sizeof(i)
void Ej1_Inicial(){
	clock_t inicio = clock();  // inicia medición de tiempo

	//printf("Tamaño de i: %d bytes\n", NRO_BYTES);
	char i;
	for (i = 1; i > 0; i++)	{
		printf("%d:0x%02x -> ", i, i);
		ImprimeBinario(i, NRO_BYTES*8);
		printf("\n");
	}

	clock_t fin = clock();  // finaliza medición de tiempo
	double milisegundos = (fin - inicio) / CLOCKS_PER_SEC * 1000;

	printf(__FUNCTION__ ": Tiempo: %.3f ms\n", milisegundos);
	printf("Tamaño de i: %d bytes\n", NRO_BYTES);
}
...






// y esto a su archivo nusuario-P1a.cpp
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








// Experimento: ¿añadir includes de más modifica el tamaño del código?
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <locale.h>
#include <iso646.h>
#include <wchar.h>
#include <wctype.h>
#include <complex.h>
#include <fenv.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <tgmath.h>
#include <uchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>










void Ej1_LosFloatsNoSonReales() {
	int i = 0;
	for (double j = 0; j != 3.0; j+= 1.0/3.0)
	{
		printf("%f", j, (int)j);
		printf("\n");
	}
}















#define LIMITE 0
typedef char tipo_test_t;

void Ej1_InicialCorregido() {
	clock_t inicio = clock();  // inicia medición de tiempo

	tipo_test_t i = 0;
	// Imprime todos los valores de i
	do {
		printf("%d:0x%02x -> ", i, (unsigned_tipo_test_t)i);
		ImprimeBinario(i, NRO_BYTES * 8);
		printf("\n");
		i += 1;
	} while (i != LIMITE);

	clock_t fin = clock();
	double milisegundos =
		1000.0 * (double)(fin - inicio) / CLOCKS_PER_SEC;

	printf(__FUNCTION__ ": Tiempo de ejecución: %.3f ms\n", milisegundos);
	printf("Tamaño de i: %d bytes\n", NRO_BYTES);
}















void Ej1_InicialMasRapido() {
	clock_t inicio = clock();  // inicia medición de tiempo

	int len = 0;
	char buffer[200];
	tipo_test_t i = 0;
	// Imprime todos los valores de i 
	do {
		len = snprintf(buffer, sizeof(buffer), "%d:0x%02x -> ", i, (unsigned_tipo_test_t)i);
		len += ImprimeBinario(buffer + len, sizeof(buffer) - len, i, NRO_BYTES * 8);
		printf("%s\n", buffer);
		i += 1;
	} while (i != LIMITE);

	clock_t fin = clock();
	double milisegundos =
		1000.0 * (double)(fin - inicio) / CLOCKS_PER_SEC;

	printf(__FUNCTION__ ": Tiempo de ejecución: %.3f ms\n", milisegundos);
	printf("Tamaño de i: %d bytes\n", NRO_BYTES);
}








//Versión de ImprimeBinario con con parámetros distintos
size_t ImprimeBinario(char* buffer, size_t tamBuffer, int valor, int bits)
{
	size_t len = 0;
	for (int i = bits - 1; i >= 0; i--)
	{
		len += snprintf(buffer + len, tamBuffer - len,
			"%d", (valor >> i) & 1);
		// Imprime un espacio para separar cada nibble
		if ((i % 4) == 0)
			snprintf(buffer + len, tamBuffer - len, " ");
		if ((i % 8) == 0)
			snprintf(buffer + len, tamBuffer - len, " ");
	}
	return len;
}



// ------------------------------------------------------------------
// Sesión 2: matrices C y archivos
// ------------------------------------------------------------------

// Lee los primeros bytes de 'nombre' y los deja en 'bloque'.
// Devuelve el número de bytes leídos, o -1 si no se puede abrir el archivo.
int LeeBloqueArchivo(const char nombre[], unsigned char bloque[])
{
	FILE* f;
	if (fopen_s(&f, nombre, "rb") != 0)
		return -1;
	int n = (int)fread(bloque, 1, 32, f);
	fclose(f);
	return n;
}

// Para probarlo, llamar a esta función desde main
// Ejercicio inicial sobre archivos, P1-Sesión 2.
void Eje1_DumpbinAntesDeEmpezar() {
	unsigned char bloque[32], var1= 1;
	int n = LeeBloqueArchivo("prueba.bin", bloque);
	for (int i = 0; i < n; i++)
		printf("%02X ", bloque[i]);
	printf("\n");
}





















// Devuelve el número de bytes leídos, o -1 si no se puede abrir el archivo.
int LeeBloqueArchivoOk(const char nombre[], unsigned char bloque[], size_t tamBloque)
{
	FILE* f;
	if (fopen_s(&f, nombre, "rb") != 0)
		return -1;
	int n = (int)fread(bloque, 1, tamBloque, f);
	fclose(f);
	return n;
}






#define TAM_LINEA 16
// Imprime el valor de un contador y una línea hexadecimal.
// Devuelve el número de caracteres escritos.
size_t ImprimeLineaHex(const unsigned char datos[], size_t nDatos, int* pContador)
{
	char buffer[TAM_LINEA * 3 + 100];
	size_t len = snprintf(buffer, sizeof(buffer), "%08X  ", *pContador);
	for (size_t i = 0; i < TAM_LINEA && i < nDatos; i++) {
		len += snprintf(buffer + len, sizeof(buffer) - len, "%02X ", datos[i]);
		(*pContador)++; // incrementa el valor del contador
	}
	len= printf("%s\n", buffer);
	return len;
}

void Eje1_DumpbinCasi() {
	unsigned char bloque[TAM_LINEA];
	int n = LeeBloqueArchivoOk("prueba.bin", bloque, sizeof(bloque));
	if (n < 0) {
		printf("Error al abrir el archivo.\n");
		return;
	}
	int contador = 0;
	ImprimeLineaHex(bloque, (size_t)n, &contador);
}
