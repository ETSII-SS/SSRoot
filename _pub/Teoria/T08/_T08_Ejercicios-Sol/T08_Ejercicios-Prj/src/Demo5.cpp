#define _CRT_SECURE_NO_WARNINGS
#include <locale.h>
#include <string.h>
#include <sslib/SSLib.h>

// Demo 5: archivos mapeados en memoria.
// Pone en negro el primer pixel (abajo a la izquierda) y el último (arriba a la derecha)
// de un BMP de 24 bits. Dos versiones del mismo proceso:
//   Demo5 [archivo.bmp]             -> con el archivo mapeado en memoria (sin ReadFile/WriteFile)
//   Demo5 [archivo.bmp] /readwrite  -> con ReadFile/WriteFile y SetFilePointer, para comparar
// Si no se indica archivo, usa in.bmp. Para restaurarlo: copiar encima "in - copia.bmp".
// Formato BMP: ver BITMAPFILEHEADER y BITMAPINFOHEADER en la ayuda de la API.

// Tamaño en bytes de una fila de pixeles: cada fila ocupa un múltiplo de 4 bytes
static DWORD TamFila(LONG ancho)
{
	return ((DWORD)ancho * 3 + 3) & ~3u;
}

// Versión con el archivo mapeado en memoria
static void PuntosNegrosConMapa(const char * ruta)
{
	HANDLE hFile;
	HANDLE hMap;
	LARGE_INTEGER TamArchivo;
	BYTE * pView;

	hFile = CreateFileA(ruta, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CheckError(hFile == INVALID_HANDLE_VALUE, 5, "ERROR abriendo el archivo %s", ruta);
	CheckError(!GetFileSizeEx(hFile, &TamArchivo), 6, "ERROR obteniendo el tamaño del archivo");
	// Un archivo de 0 bytes no se puede mapear. Además, debe caber al menos la cabecera.
	CheckError(TamArchivo.QuadPart < (LONGLONG)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)), 7,
		"ERROR: el archivo es demasiado pequeño para ser un BMP");

	//> Crea el mapa (tamaño 0, 0: el del archivo)
	hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	CheckError(NULL == hMap, 1, "ERROR ejecutando CreateFileMapping");
	//> Crea vista de todo el archivo (= reserva direcciones; cada página se lee del disco al usarla)
	pView = (BYTE *)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
	CheckError(NULL == pView, 2, "ERROR ejecutando MapViewOfFile");
	// A partir de aquí ya puedo usar el puntero directamente...

	BITMAPFILEHEADER * pCabArchivo = (BITMAPFILEHEADER *)pView;
	BITMAPINFOHEADER * pCabImagen = (BITMAPINFOHEADER *)&pView[sizeof(BITMAPFILEHEADER)];
	CheckError(pCabArchivo->bfType != 0x4D42 || pCabImagen->biBitCount != 24 || pCabImagen->biCompression != BI_RGB, 8,
		"ERROR: el archivo no es un BMP de 24 bits sin comprimir");

	LONG MaxX = pCabImagen->biWidth;
	LONG MaxY = pCabImagen->biHeight < 0 ? -pCabImagen->biHeight : pCabImagen->biHeight; // negativo: filas de arriba abajo
	DWORD tamFila = TamFila(MaxX);
	LONGLONG tamImagen = (LONGLONG)tamFila * MaxY;
	CheckError(MaxX <= 0 || MaxY <= 0 || pCabArchivo->bfOffBits + tamImagen > TamArchivo.QuadPart, 9,
		"ERROR: las dimensiones de la cabecera no caben en el archivo");

	BYTE * pPixeles = &pView[pCabArchivo->bfOffBits];  // los pixeles empiezan en bfOffBits
	memset(&pPixeles[0], 0, 3);                                         // primer pixel de la primera fila
	memset(&pPixeles[(MaxY - 1) * tamFila + (MaxX - 1) * 3], 0, 3);     // último pixel de la última fila (sin el relleno)

	//>Libera vista y mapa
	UnmapViewOfFile(pView);
	CloseHandle(hMap);
	CloseHandle(hFile);
}

// Versión tradicional, con ReadFile/WriteFile y SetFilePointer, para comparar
static void PuntosNegrosConReadWrite(const char * ruta)
{
	HANDLE hArchivo;
	DWORD Movido;
	BITMAPFILEHEADER CabArchivo;
	BITMAPINFOHEADER CabImagen;
	BYTE negro[3] = { 0, 0, 0 };

	hArchivo = CreateFileA(ruta, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CheckError(INVALID_HANDLE_VALUE == hArchivo, 2, "ERROR: No se pudo abrir el archivo %s", ruta);

	// Lee las dos cabeceras completas
	CheckError(!ReadFile(hArchivo, &CabArchivo, sizeof(CabArchivo), &Movido, NULL) || Movido != sizeof(CabArchivo),
		3, "Error: No se ha podido leer la cabecera del archivo");
	CheckError(!ReadFile(hArchivo, &CabImagen, sizeof(CabImagen), &Movido, NULL) || Movido != sizeof(CabImagen),
		3, "Error: No se ha podido leer la cabecera de la imagen");
	CheckError(CabArchivo.bfType != 0x4D42 || CabImagen.biBitCount != 24 || CabImagen.biCompression != BI_RGB, 8,
		"ERROR: el archivo no es un BMP de 24 bits sin comprimir");

	LONG MaxX = CabImagen.biWidth;
	LONG MaxY = CabImagen.biHeight < 0 ? -CabImagen.biHeight : CabImagen.biHeight;
	DWORD tamFila = TamFila(MaxX);
	LARGE_INTEGER TamArchivo;
	CheckError(!GetFileSizeEx(hArchivo, &TamArchivo), 6, "ERROR obteniendo el tamaño del archivo");
	CheckError(MaxX <= 0 || MaxY <= 0 || CabArchivo.bfOffBits + (LONGLONG)tamFila * MaxY > TamArchivo.QuadPart, 9,
		"ERROR: las dimensiones de la cabecera no caben en el archivo");

	// Primer pixel: al inicio de la matriz de pixeles
	Movido = SetFilePointer(hArchivo, (LONG)CabArchivo.bfOffBits, NULL, FILE_BEGIN);
	CheckError(INVALID_SET_FILE_POINTER == Movido, 5, "Error al ejecutar SetFilePointer");
	CheckError(!WriteFile(hArchivo, negro, 3, &Movido, NULL) || Movido != 3, 5, "Error al escribir pixel");

	// Último pixel de la última fila (sin el relleno)
	Movido = SetFilePointer(hArchivo, (LONG)(CabArchivo.bfOffBits + (MaxY - 1) * tamFila + (MaxX - 1) * 3), NULL, FILE_BEGIN);
	CheckError(INVALID_SET_FILE_POINTER == Movido, 6, "Error al ejecutar SetFilePointer");
	CheckError(!WriteFile(hArchivo, negro, 3, &Movido, NULL) || Movido != 3, 6, "Error al escribir pixel");

	CloseHandle(hArchivo);
}

int main(int argc, char *argv[])
{
	const char * ruta = "in.bmp";
	bool conReadWrite = false;

	setlocale(LC_ALL, "Spanish"); // Previo

	for (int i = 1; i < argc; i++) {
		if (_stricmp(argv[i], "/readwrite") == 0)
			conReadWrite = true;
		else
			ruta = argv[i];
	}
	if (conReadWrite)
		PuntosNegrosConReadWrite(ruta);
	else
		PuntosNegrosConMapa(ruta);
	printf("\nPuntos negros escritos en %s\n", ruta);

	printf("\nPulse una tecla para terminar\n");  //Solo para depurar. Quitar si no es necesario
	(void)getchar();
	return 0;
}
