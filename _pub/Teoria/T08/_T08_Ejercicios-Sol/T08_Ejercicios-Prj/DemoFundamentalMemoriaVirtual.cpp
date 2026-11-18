#define _CRT_SECURE_NO_WARNINGS
#include <conio.h>
#include <sslib/SSLib.h>

DWORD contador = 0;
int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Spanish"); // Previo
	printf("\Pulse la tecla ESC para terminar\n\n");  //Solo para depurar. Quitar si no es necesario
	while (true) {
		printf("Dir: 0x%p, contenido: %d \n", &contador, contador++);
		Sleep(1000);
		if (_kbhit()){
			if (_getch() == 27)  // Tecla ESC
				break;
		}
	}
	return 0;
}