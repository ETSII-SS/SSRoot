#define _CRT_SECURE_NO_WARNINGS
#include <sslib/SSLib.h>

int main(int argc, char *argv[])
{
	SYSTEM_INFO SysInfo;
	double TamMemoria;


	setlocale(LC_ALL, "Spanish"); // Previo

	// Espacio de direcciones disponible para una aplicación
	GetSystemInfo(&SysInfo);
	printf("\nLa memoria de usuario de esta aplicación está en 0x%p - 0x%p \n",
		SysInfo.lpMinimumApplicationAddress, SysInfo.lpMaximumApplicationAddress);
	TamMemoria = (LPBYTE)SysInfo.lpMaximumApplicationAddress - (LPBYTE)SysInfo.lpMinimumApplicationAddress;
	printf("Tamaño máximo (en GBytes): %f\n", (float)TamMemoria / (1024.0*1024.0*1024.0));

	printf("\nPulse una tecla para terminar\n");  //Solo para depurar. Quitar si no es necesario
	(void)getchar();
	return 0;
}