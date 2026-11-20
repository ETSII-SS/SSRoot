#define _CRT_SECURE_NO_WARNINGS
#include <locale.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sslib/SSLib.h>

// Tema 8, Demo 5: archivos mapeados en memoria, con los archivos de prueba de la práctica 4 (sesión 2).
//
// Esos archivos (C:\tmp\P4S2\prueba*.dat, que crea el argumento /prueba del programa de la P4)
// son de texto. Todas sus líneas miden 74 bytes y son iguales salvo el número de línea, que empieza en 0:
//   0000000000;0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
//   0000000001;0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
//   ...
// La última línea puede estar incompleta: prueba1000000.dat tiene 13 513 líneas completas y 38 bytes más.
//
// Con el archivo mapeado, el archivo entero se usa como una matriz de estructuras Linea:
// la línea k es lineas[k]. No hay ReadFile, ni WriteFile, ni hay que mover el puntero de archivo.
//
// Uso: Demo5 [ruta] [k] [/cambia | /readfile]
//   ruta       por defecto, C:\tmp\P4S2\prueba1000000.dat
//   k          línea que se imprime (por defecto, la del medio)
//   /cambia    cambia mayúsculas por minúsculas en el texto de la línea k, escribiendo en la vista.
//              Si se ejecuta otra vez con los mismos argumentos, el archivo vuelve a quedar como estaba.
//              (El argumento /prueba de la P4 también lo vuelve a crear.)
//   /readfile  imprime la línea k sin mapa, con SetFilePointerEx y ReadFile, para comparar.

#define RUTA_POR_DEFECTO "C:\\tmp\\P4S2\\prueba1000000.dat"
#define TEXTO_LINEA "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

// Una línea del archivo de prueba. Todos los campos son char: no hay relleno (práctica 2) y ocupa 74 bytes
struct Linea {
	char numero[10];   // número de línea en decimal, con ceros a la izquierda (sin '\0')
	char separador;    // ';'
	char texto[62];    // TEXTO_LINEA (sin '\0')
	char fin;          // '\n'
};
static_assert(sizeof(Linea) == 74, "Linea debe ocupar 74 bytes");

// Imprime una línea. No termina en '\0': se indica cuántos caracteres hay que imprimir
static void ImprimeLinea(const Linea * pLinea, int64_t k)
{
	printf("Línea %lld: %.*s\n", (long long)k, (int)(sizeof(Linea) - 1), (const char *)pLinea);
}

// true si la línea k es como la escribió el programa de la P4
static bool LineaCorrecta(const Linea * pLinea, int64_t k)
{
	char numero[12];
	snprintf(numero, sizeof(numero), "%010lld", (long long)k);
	return memcmp(pLinea->numero, numero, sizeof(pLinea->numero)) == 0 && pLinea->separador == ';'
		&& memcmp(pLinea->texto, TEXTO_LINEA, sizeof(pLinea->texto)) == 0 && pLinea->fin == '\n';
}

// Versión con el archivo mapeado en memoria
static void DemoConMapa(const char * ruta, int64_t k, bool cambia)
{
	HANDLE hArchivo;
	HANDLE hMapa;
	LARGE_INTEGER tam;
	Linea * lineas;

	hArchivo = CreateFileA(ruta, cambia ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CheckError(hArchivo == INVALID_HANDLE_VALUE, 1, "ERROR abriendo %s", ruta);
	CheckError(!GetFileSizeEx(hArchivo, &tam), 2, "ERROR obteniendo el tamaño de %s", ruta);
	CheckError(tam.QuadPart == 0, 3, "ERROR: %s está vacío, y un archivo vacío no se puede mapear", ruta);

	//> Crea el mapa (tamaño 0, 0: el del archivo). Solo para leer, salvo que haya que cambiar algo
	hMapa = CreateFileMappingA(hArchivo, NULL, cambia ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL);
	CheckError(NULL == hMapa, 4, "ERROR ejecutando CreateFileMapping");
	//> Crea una vista de todo el archivo (= reserva direcciones; cada página se lee del disco al usarla)
	lineas = (Linea *)MapViewOfFile(hMapa, cambia ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	CheckError(NULL == lineas, 5, "ERROR ejecutando MapViewOfFile");
	// A partir de aquí, el archivo es una matriz de estructuras Linea

	int64_t nroLineas = tam.QuadPart / (int64_t)sizeof(Linea);  // líneas completas
	int64_t resto = tam.QuadPart % (int64_t)sizeof(Linea);       // bytes de la última línea, incompleta
	printf("%s: %lld %s, %lld líneas completas", ruta, (long long)tam.QuadPart,
		tam.QuadPart == 1 ? "byte" : "bytes", (long long)nroLineas);
	if (resto > 0)
		printf(" y una incompleta de %lld %s", (long long)resto, resto == 1 ? "byte" : "bytes");
	printf("\n");

	// 1. Recorre todas las líneas completas, como si fuera una matriz en memoria
	int64_t incorrectas = 0, primeraIncorrecta = -1;
	for (int64_t i = 0; i < nroLineas; i++) {
		if (!LineaCorrecta(&lineas[i], i)) {
			if (primeraIncorrecta < 0)
				primeraIncorrecta = i;
			incorrectas++;
		}
	}
	if (incorrectas == 0)
		printf("Todas las líneas están como las escribió la P4\n");
	else
		printf("Líneas cambiadas: %lld. La primera es la %lld\n", (long long)incorrectas, (long long)primeraIncorrecta);

	// 2. Acceso directo a la línea k: sin leer las anteriores y sin mover el puntero de archivo
	if (k < 0)
		k = nroLineas / 2;
	if (k >= nroLineas)
		printf("El archivo no tiene una línea %lld completa\n", (long long)k);
	else {
		ImprimeLinea(&lineas[k], k);
		// 3. Cambia mayúsculas por minúsculas escribiendo en la vista: es escribir en el archivo
		if (cambia) {
			for (size_t i = 0; i < sizeof(lineas[k].texto); i++) {
				unsigned char c = (unsigned char)lineas[k].texto[i];
				lineas[k].texto[i] = (char)(isupper(c) ? tolower(c) : toupper(c));
			}
			printf("Cambiada. ");
			ImprimeLinea(&lineas[k], k);
		}
	}

	//> Deshace la vista y cierra el mapa y el archivo. Los cambios llegan al disco sin hacer nada más
	UnmapViewOfFile(lineas);
	CloseHandle(hMapa);
	CloseHandle(hArchivo);
}

// La misma lectura de la línea k, sin mapa: hay que mover el puntero de archivo y leer en una variable
static void ImprimeLineaConReadFile(const char * ruta, int64_t k)
{
	HANDLE hArchivo;
	LARGE_INTEGER tam, posicion;
	Linea linea;
	DWORD leidos;

	hArchivo = CreateFileA(ruta, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CheckError(hArchivo == INVALID_HANDLE_VALUE, 1, "ERROR abriendo %s", ruta);
	CheckError(!GetFileSizeEx(hArchivo, &tam), 2, "ERROR obteniendo el tamaño de %s", ruta);
	if (k < 0)
		k = tam.QuadPart / (int64_t)sizeof(Linea) / 2;

	posicion.QuadPart = k * (int64_t)sizeof(Linea);
	CheckError(!SetFilePointerEx(hArchivo, posicion, NULL, FILE_BEGIN), 6, "ERROR ejecutando SetFilePointerEx");
	CheckError(!ReadFile(hArchivo, &linea, sizeof(linea), &leidos, NULL), 7, "ERROR ejecutando ReadFile");
	if (leidos != sizeof(linea))
		printf("El archivo no tiene una línea %lld completa\n", (long long)k);
	else
		ImprimeLinea(&linea, k);

	CloseHandle(hArchivo);
}

int main(int argc, char *argv[])
{
	const char * ruta = RUTA_POR_DEFECTO;
	int64_t k = -1;            // -1: la línea del medio
	bool cambia = false, conReadFile = false;

	setlocale(LC_ALL, "Spanish"); // Previo

	for (int i = 1; i < argc; i++) {
		if (_stricmp(argv[i], "/cambia") == 0)
			cambia = true;
		else if (_stricmp(argv[i], "/readfile") == 0)
			conReadFile = true;
		else if (isdigit((unsigned char)argv[i][0]))
			k = _atoi64(argv[i]);
		else
			ruta = argv[i];
	}
	if (conReadFile)
		ImprimeLineaConReadFile(ruta, k);
	else
		DemoConMapa(ruta, k, cambia);

	printf("\nPulse una tecla para terminar\n");  //Solo para depurar. Quitar si no es necesario
	(void)getchar();
	return 0;
}
