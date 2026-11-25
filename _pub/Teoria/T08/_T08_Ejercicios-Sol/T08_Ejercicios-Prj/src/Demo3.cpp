/// Tema 8, Demo 3: hiperpaginación (thrashing).
/// IMPORTANTE: Compilar este programa en x64.
/// ADVERTENCIA: guardar todos los archivos modificados antes de ejecutarlo.
///
/// Reserva bloques de 1 GB con malloc, sin liberarlos, hasta pasar en GB_DE_MAS GB la memoria
/// física del equipo. Cada vez que reserva un bloque, recorre toda la memoria reservada
/// (modifica un byte de cada página) y muestra el conjunto de trabajo del proceso, cuánto tarda
/// el recorrido y cuántos fallos de página produce:
///  - mientras todo cabe en memoria física, el conjunto de trabajo es toda la memoria reservada
///    y el recorrido tarda menos de un segundo, sin fallos de página;
///  - cuando deja de caber, Windows quita páginas al proceso: el conjunto de trabajo baja, se
///    disparan los fallos de página y el recorrido tarda segundos o minutos.
/// Para verlo desde fuera: Monitor de recursos, pestaña Memoria, columna "Errores graves/s".
/// Para terminar antes: Ctrl+C. Al terminar, el SO recupera toda la memoria del proceso.
/// Si malloc falla la primera vez, volver a ejecutarlo: Windows amplía el archivo de paginación
/// sobre la marcha y a veces no le da tiempo.

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#include <conio.h>
#include <Windows.h>
#include <psapi.h>  // GetProcessMemoryInfo
#include <sslib/SSLib.h>
#pragma comment(lib, "psapi.lib")

#define GB ((size_t)1024 * 1024 * 1024)
#define GB_DE_MAS 2       // GB que se reservan por encima de la memoria física
#define MAX_BLOQUES 1024
#define TAM_PAGINA 4096

int  CalculaBloques();
void RellenaAleatorio(char* bloque, size_t tam);
void RecorreYMide(char* bloques[], int nBloques);

int main(int argc, char* argv[])
{
	char* bloques[MAX_BLOQUES];

	setlocale(LC_ALL, "Spanish"); // Previo

	int nBloques = CalculaBloques();       // imprime la memoria física y la que se va a reservar
	for (int n = 0; n < nBloques; n++) {
		bloques[n] = (char*)malloc(GB);  // un bloque más, sin liberar los anteriores
		CheckError(NULL == bloques[n], 1, "No se pudo reservar más memoria");
		RellenaAleatorio(bloques[n], GB);  // datos que no se pueden comprimir
		RecorreYMide(bloques, n + 1);      // recorre todo lo reservado: tiempo y fallos de página
	}
	printf("\nPulse una tecla para terminar: el SO recuperará toda la memoria del proceso\n");
	(void)_getch();
	return 0;  // sin free: al terminar el proceso, el SO recupera toda su memoria
}

// Devuelve cuántos bloques de 1 GB hay que reservar para pasar en GB_DE_MAS GB la memoria
// física del equipo, e imprime la memoria física y la que se va a reservar.
int CalculaBloques()
{
	MEMORYSTATUSEX estado;

	estado.dwLength = sizeof(estado);
	GlobalMemoryStatusEx(&estado);
	int nBloques = (int)(estado.ullTotalPhys / GB) + GB_DE_MAS;
	if (nBloques > MAX_BLOQUES)
		nBloques = MAX_BLOQUES;
	printf("Memoria física: %.1f GB. Se van a reservar %d GB.\n\n",
		(double)estado.ullTotalPhys / GB, nBloques);
	return nBloques;
}

// Rellena el bloque con datos pseudoaleatorios (generador xorshift). Windows comprime la memoria
// que saca de memoria física: con datos repetidos, todo cabría comprimido en memoria física y no
// habría hiperpaginación.
void RellenaAleatorio(char* bloque, size_t tam)
{
	static uint64_t x = 88172645463325252ULL;  // static: cada bloque continúa la secuencia
	uint64_t* p = (uint64_t*)bloque;
	for (size_t i = 0; i < tam / sizeof(uint64_t); i++) {
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		p[i] = x;
	}
}

// Recorre los bloques modificando un byte de cada página e imprime una fila de la tabla: memoria
// reservada, conjunto de trabajo del proceso, tiempo del recorrido y fallos de página.
// El conjunto de trabajo son las páginas que el proceso tiene asignadas ahora en memoria física.
// Cuando falta memoria, Windows le quita páginas: pasan a la lista de páginas modificadas (o a la
// de espera, una vez escritas en el archivo de paginación) y siguen en memoria física hasta que
// se reutilizan. Tocar una página que no está en el conjunto de trabajo produce un fallo de
// página: blando si la página sigue en memoria física; grave si hay que leerla del disco.
void RecorreYMide(char* bloques[], int nBloques)
{
	PROCESS_MEMORY_COUNTERS antes, despues;

	GetProcessMemoryInfo(GetCurrentProcess(), &antes, sizeof(antes));
	ULONGLONG inicio = GetTickCount64();
	for (int b = 0; b < nBloques; b++)
		for (size_t i = 0; i < GB; i += TAM_PAGINA)
			bloques[b][i]++;
	ULONGLONG ms = GetTickCount64() - inicio;
	GetProcessMemoryInfo(GetCurrentProcess(), &despues, sizeof(despues));

	if (nBloques == 1)  // primera fila: antes, la cabecera de la tabla
		printf("Reservada   Conjunto de trabajo   Tiempo   Fallos de página\n");
	printf("%6d GB%19.1f GB%7.2f s%19lu\n", nBloques, (double)despues.WorkingSetSize / GB,
		ms / 1000.0, despues.PageFaultCount - antes.PageFaultCount);
}
