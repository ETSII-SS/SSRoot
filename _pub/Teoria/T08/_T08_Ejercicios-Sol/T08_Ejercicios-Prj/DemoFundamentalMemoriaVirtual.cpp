#define _CRT_SECURE_NO_WARNINGS
#include <conio.h>
#include <locale.h>
#include <sslib/SSLib.h>

// Demo fundamental de memoria virtual.
// Ejecutar dos instancias a la vez (Ctrl+F5 dos veces, 
//con varios segundos de separación: las dos imprimen la misma
// dirección para contador, pero cada una tiene su propio valor.
// La dirección coincide porque Windows (ASLR) elige la dirección de carga de cada
// ejecutable una vez por arranque: dos instancias del mismo .exe lo cargan en el mismo sitio.
DWORD contador = 0;
int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Spanish"); // Previo
	printf("\nPulse la tecla ESC para terminar\n\n");
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