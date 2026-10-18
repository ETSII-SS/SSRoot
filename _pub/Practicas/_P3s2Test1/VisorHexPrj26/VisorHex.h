#pragma once
// Declaración de la clase ss::VisorHex: visor hexadecimal de archivos y de memoria.
#include <stdint.h>
#include <windows.h>
#include <sslib/SSLib.h>

// Un grupo de 1, 2, 4 u 8 bytes, visto de cuatro formas distintas (uniones, práctica 2)
typedef union {
	uint8_t  b[8];	// como matriz de bytes
	uint16_t w;		// como entero de 16 bits (b[0] y b[1])
	uint32_t d;		// como entero de 32 bits (b[0] a b[3])
	uint64_t q;		// como entero de 64 bits (b[0] a b[7])
} Grupo_t;

// Resumen del último volcado. El programa lo vuelca a él mismo como autocomprobación.
typedef struct {
	char     origen;		// 'A': archivo, 'M': memoria
	uint32_t bytesVolcados;
	uint16_t bytesLinea;
	uint8_t  bytesGrupo;
	uint32_t lineas;
} ResumenVolcado_t;

class VisorHex
{
private:
	int      _bytesLinea;	// bytes por línea: 8, 16 o 32
	int      _bytesGrupo;	// bytes por grupo: 1, 2, 4 u 8
	uint8_t* _bloque;		// buffer para una línea (se reserva en el constructor)
	HANDLE   _hArchivo;		// archivo abierto, o INVALID_HANDLE_VALUE si no hay ninguno
	char     _origen;		// origen del último volcado: 'A' (Archivo) o 'M' (Memoria)
	uint32_t _offset;		// posición del siguiente byte que se va a volcar
	uint32_t _lineas;		// líneas volcadas
	ss::Dbg  _dbg = ss::Dbg(true);		// Errores y mensajes por ventana Salida del depurador y consola

	void ImprimeLinea(const uint8_t datos[], int nDatos);
public:
	VisorHex(int bytesGrupo = 1, int bytesLinea = 16);
	~VisorHex();
	// Devuelve false si hay error. Los bytes volcados se dejan en *pBytesVolcados
	bool VuelcaArchivo(const char* nombreArchivo, int* pBytesVolcados);
	// Devuelve el número de bytes volcados, o -1 si hay error
	int VuelcaMemoria(const void* pDatos, size_t nBytes);
	// Rellena *pResumen con los datos del último volcado
	void Resumen(ResumenVolcado_t* pResumen);

	// N3: Prohíbe copiar: dos copias compartirían _bloque y _hArchivo (doble delete[] y doble CloseHandle)
	VisorHex(const VisorHex&) = delete;
	VisorHex& operator=(const VisorHex&) = delete;
};

