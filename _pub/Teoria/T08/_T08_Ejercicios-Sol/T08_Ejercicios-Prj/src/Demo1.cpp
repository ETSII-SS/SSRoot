#define _CRT_SECURE_NO_WARNINGS
#include <locale.h>
#include <sslib/SSLib.h>

// Tema 8, Demo 1 (configuración x86) y Demo 2 (configuración x64): el mismo programa.
int main(int argc, char *argv[])
{
	SYSTEM_INFO SysInfo;
	double TamMemoria;


	setlocale(LC_ALL, "Spanish"); // Previo

	// Espacio de direcciones disponible para una aplicación
	GetSystemInfo(&SysInfo);
	printf("\nLa memoria de usuario de esta aplicación está en 0x%p - 0x%p \n",
		SysInfo.lpMinimumApplicationAddress, SysInfo.lpMaximumApplicationAddress);
	TamMemoria = (double)((LPBYTE)SysInfo.lpMaximumApplicationAddress - (LPBYTE)SysInfo.lpMinimumApplicationAddress);
	printf("Tamaño máximo (en GBytes): %f\n", (float)TamMemoria / (1024.0*1024.0*1024.0));

	// Tamaño de página y granularidad de reserva (VirtualAlloc)
	printf("Tamaño de página: %lu bytes\n", SysInfo.dwPageSize);
	printf("Granularidad de reserva: %lu bytes\n", SysInfo.dwAllocationGranularity);
	printf("\nPulse una tecla para terminar\n");  //Solo para depurar. Quitar si no es necesario
	(void)getchar();
	return 0;
}