#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <Windows.h>

/// IMPORTANTE: Compilar este programa en x64
/// Uso: Tema 8, Demo4               -> reserva y asocia toda la secuencia de una vez
///      Tema 8, Demo4 /porsegundos  -> reserva toda la secuencia, pero asocia solo el segundo que procesa
/// Para ver la diferencia: columna "Tamaño de confirmación" (memoria asociada) del Administrador de tareas.
const int	MaxX = 1024, // Cada fila ocupa una página de 4 K (1024*4)
			MaxY = 4*1024, // Cada imagen son 16 Mbytes 
			FramesXSec= 25, // Cada segundo son 400 MBytes
			Duracion=1*10; // 4000 MBytes (unos 3,9 GB) cada 10 segundos

typedef RGBQUAD frame_t[MaxY][MaxX];
//frame_t secuencia[FramesXSec*Duracion];  // problema: el compilador solo admite matrices
										 // estáticas de menos de 2 G

const size_t tamSecuencia = sizeof(frame_t)*FramesXSec*Duracion;
const size_t tamSegundo = sizeof(frame_t)*FramesXSec;

// Procesa los frames de un segundo. Escribe el primer pixel de cada fila: como cada fila
// ocupa una página, cada escritura usa una página nueva (fallo de página).
static void ProcesaSegundo(frame_t * frames)
		{
			for (int nroFrame = 0; nroFrame < FramesXSec; nroFrame++) // Proceso cada frame del segundo
			{
				//...
				for (int filaImagen = 0; filaImagen < MaxY; filaImagen++) // Proceso cada fila de la imagen
				{
					//...
			frames[nroFrame][filaImagen][0].rgbRed = 0xff;
		}
	}
				}

// Versión simple: reserva y asocia toda la secuencia de una vez.
// La memoria asociada crece con la duración: 20, 30 o 60 segundos pueden no caber
// en memoria física + archivo de paginación, y VirtualAlloc falla.
static bool ProcesaTodo()
{
	frame_t * secuencia = (frame_t *) VirtualAlloc(NULL, tamSecuencia, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (secuencia == NULL) {
		printf("\nERROR %lu: No se pudo reservar y asociar la memoria (%zu MBytes)\n",
			GetLastError(), tamSecuencia / (1024 * 1024));
		return false;
			}
	for (int seg = 0; seg < Duracion; seg++) // Proceso cada segundo
	{
		ProcesaSegundo(&secuencia[seg*FramesXSec]);
		printf("\rProcesado el segundo %d de %d", seg + 1, Duracion);
		}
		VirtualFree(secuencia, 0, MEM_RELEASE);
	return true;
}

// Versión por segundos: reserva direcciones para toda la secuencia, pero en cada momento
// solo tiene asociado el segundo que procesa (400 MBytes), sea cual sea la duración.
static bool ProcesaPorSegundos()
{
	frame_t * secuencia = (frame_t *) VirtualAlloc(NULL, tamSecuencia, MEM_RESERVE, PAGE_NOACCESS);
	if (secuencia == NULL) {
		printf("\nERROR %lu: No se pudo reservar la memoria (%zu MBytes)\n",
			GetLastError(), tamSecuencia / (1024 * 1024));
		return false;
	}
	bool ok = true;
	for (int seg = 0; seg < Duracion && ok; seg++) // Proceso cada segundo
	{
		frame_t * segundo = &secuencia[seg*FramesXSec];
		if (NULL == VirtualAlloc(segundo, tamSegundo, MEM_COMMIT, PAGE_READWRITE)) {
			printf("\nERROR %lu: No se pudo asociar el segundo %d\n", GetLastError(), seg + 1);
			ok = false;
		}
		else {
			ProcesaSegundo(segundo);
			VirtualFree(segundo, tamSegundo, MEM_DECOMMIT); // desasocia: deja de contar como memoria asociada
			printf("\rProcesado el segundo %d de %d", seg + 1, Duracion);
		}
	}
	VirtualFree(secuencia, 0, MEM_RELEASE); // libera la reserva completa (tamaño 0)
	return ok;
	}
int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Spanish"); // Previo

	printf("Secuencia de %d segundos: %zu MBytes\n", Duracion, tamSecuencia / (1024 * 1024));
	if (argc > 1 && _stricmp(argv[1], "/porsegundos") == 0)
		ProcesaPorSegundos();
	else
		ProcesaTodo();
    printf("\nPulse una tecla para terminar\n");  //Solo para depurar. Quitar si no es necesario
    (void)getchar();
    return 0;
}