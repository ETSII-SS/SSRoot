#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#include <conio.h>
#include <Windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

/// IMPORTANTE: Compilar este programa en x64
/// ADVERTENCIA: guardar todos los archivos modificados antes de ejecutarlo.
///
/// Tema 8, Demo 3: hiperpaginación (thrashing), de forma controlada.
///  1. Reserva bloques de 256 MBytes con malloc hasta sumar 1,5 veces la memoria física del equipo,
///     sin acercarse al límite de memoria asociable (memoria física + archivo de paginación).
///  2. Los rellena con datos pseudoaleatorios, que la compresión de memoria de Windows no puede reducir.
///  3. Los recorre una y otra vez, leyendo un byte de cada página, e imprime lo que tarda cada pasada
///     y cuántos fallos de página produce.
/// Uso: Demo3 [factor]   factor = memoria reservada / memoria física (por defecto, 1.5).
///      Con 0.5 todo cabe en memoria física y sirve de comparación.
/// Mientras se ejecuta: Monitor de recursos, pestaña Memoria, columna "Errores graves/s".
/// Para terminar antes: pulsar una tecla (se comprueba después de cada bloque) o Ctrl+C.
/// Al terminar, el SO recupera toda la memoria del proceso de golpe.

#define TAM_BLOQUE ((size_t)256*1024*1024)  // 256 MBytes
#define TAM_PAGINA 4096
#define MAX_BLOQUES 1024                    // como mucho, 256 GBytes
#define MAX_PASADAS 10

static double GB(ULONGLONG bytes)
{
	return (double)bytes / (1024.0 * 1024.0 * 1024.0);
}

// Rellena un bloque con datos pseudoaleatorios (generador xorshift): no se pueden comprimir
static void RellenaAleatorio(char * bloque, size_t tam, uint64_t * semilla)
{
	uint64_t x = *semilla;
	uint64_t * p = (uint64_t *) bloque;
	for (size_t i = 0; i < tam / sizeof(uint64_t); i++) {
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		p[i] = x;
	}
	*semilla = x;
}

// Devuelve false si la memoria asociable libre baja del margen. Antes espera un poco, porque
// Windows amplía el archivo de paginación cuando la memoria asociada se acerca al límite.
static bool QuedaMemoriaAsociable(ULONGLONG margen)
{
	MEMORYSTATUSEX estado;
	estado.dwLength = sizeof(estado);
	for (int intento = 0; intento < 5; intento++) {
		GlobalMemoryStatusEx(&estado);
		if (estado.ullAvailPageFile > margen + TAM_BLOQUE)
			return true;
		Sleep(1000);
	}
	return false;
}

int main(int argc, char *argv[])
{
	MEMORYSTATUSEX estado;
	PROCESS_MEMORY_COUNTERS contadores;
	static char * bloques[MAX_BLOQUES];
	int nBloques = 0;
	uint64_t semilla = 88172645463325252ULL;
	uint64_t suma = 0;
	bool parar = false;

	setlocale(LC_ALL, "Spanish"); // Previo

	double factor = (argc > 1) ? atof(argv[1]) : 1.5;  // memoria reservada / memoria física
	if (factor <= 0 || factor > 3) {
		printf("Uso: %s [factor], con 0 < factor <= 3\n", argv[0]);
		return 1;
	}
	estado.dwLength = sizeof(estado);
	GlobalMemoryStatusEx(&estado);
	ULONGLONG objetivo = (ULONGLONG)(factor * (double)estado.ullTotalPhys);
	ULONGLONG margen = estado.ullTotalPageFile / 20;  // deja libre el 5 % del límite de memoria asociable
	printf("Memoria física: %.1f GB. Límite de memoria asociable: %.1f GB. Se van a reservar %.1f GB.\n",
		GB(estado.ullTotalPhys), GB(estado.ullTotalPageFile), GB(objetivo));

	// 1. Reserva y rellena bloques hasta el objetivo, sin acercarse al límite de memoria asociable
	while (nBloques < MAX_BLOQUES && (ULONGLONG)nBloques * TAM_BLOQUE < objetivo
		&& QuedaMemoriaAsociable(margen)) {
		bloques[nBloques] = (char *) malloc(TAM_BLOQUE);
		if (NULL == bloques[nBloques])
			break;
		RellenaAleatorio(bloques[nBloques], TAM_BLOQUE, &semilla);  // datos que no se pueden comprimir
		nBloques++;
		printf("\rReservados %.1f GB", GB((ULONGLONG)nBloques * TAM_BLOQUE));
	}
	if ((ULONGLONG)nBloques * TAM_BLOQUE < objetivo)
		printf(" (se para antes: no queda memoria asociable o malloc ha fallado)");
	printf("\n\n");

	// 2. Recorre todos los bloques una y otra vez, leyendo un byte de cada página
	for (int pasada = 1; pasada <= MAX_PASADAS && !parar; pasada++) {
		GetProcessMemoryInfo(GetCurrentProcess(), &contadores, sizeof(contadores));
		DWORD fallosAntes = contadores.PageFaultCount;
		ULONGLONG inicio = GetTickCount64();
		for (int b = 0; b < nBloques && !parar; b++) {
			for (size_t i = 0; i < TAM_BLOQUE; i += TAM_PAGINA)
				suma += bloques[b][i];
			parar = _kbhit();
		}
		ULONGLONG ms = GetTickCount64() - inicio;
		GetProcessMemoryInfo(GetCurrentProcess(), &contadores, sizeof(contadores));
		printf("Pasada %2d: %7.2f s, %9lu fallos de página, memoria física del proceso: %.1f GB\n",
			pasada, (double)ms / 1000.0, contadores.PageFaultCount - fallosAntes, GB(contadores.WorkingSetSize));
	}
	if (parar)
		(void)_getch();  // descarta la tecla pulsada

	for (int b = 0; b < nBloques; b++)
		free(bloques[b]);
	// La suma solo sirve para que el compilador no elimine el bucle de lectura al optimizar
	printf("\nMemoria liberada (suma de comprobación: %llu)\n", (unsigned long long)suma);

	printf("\nPulse una tecla para terminar\n");  //Solo para depurar. Quitar si no es necesario
	(void)_getch();
	return 0;
}
