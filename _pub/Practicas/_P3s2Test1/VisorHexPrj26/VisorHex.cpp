// Definición de los métodos de la clase ss::VisorHex.
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "VisorHex.h"



VisorHex::VisorHex(int bytesGrupo, int bytesLinea)
{
	if (_dbg.CheckError(bytesLinea != 8 && bytesLinea != 16 && bytesLinea != 32,
		"VisorHex: bytes por línea debe ser 8, 16 o 32.\n"))
		bytesLinea = 16;	// valor por defecto
	if (_dbg.CheckError(bytesGrupo != 1 && bytesGrupo != 2 && bytesGrupo != 4 && bytesGrupo != 8,
		"VisorHex: bytes por grupo debe ser 1, 2, 4 o 8.\n"))
		bytesGrupo = 1;	// valor por defecto
	_bytesLinea = bytesLinea;
	_bytesGrupo = bytesGrupo;
	_bloque = new uint8_t[bytesLinea];	// el archivo se lee línea a línea
	_hArchivo = INVALID_HANDLE_VALUE;
	_origen = ' ';
	_offset = 0;
	_lineas = 0;
}

VisorHex::~VisorHex()
{
	if (_hArchivo != INVALID_HANDLE_VALUE)	// por si un volcado no se terminó
		CloseHandle(_hArchivo);
	delete[] _bloque;
}

// Vuelca un archivo completo. Se abre UNA vez: cada ReadFile avanza el puntero de archivo.
bool VisorHex::VuelcaArchivo(const char* nombreArchivo, int* pBytesVolcados)
{
	_origen = 'A';
	_offset = 0;
	_lineas = 0;
	_hArchivo = CreateFileA(nombreArchivo, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_dbg.CheckError(_hArchivo = INVALID_HANDLE_VALUE,
		"No se pudo abrir el archivo:\n   %s\n", nombreArchivo))
		return false;
	_dbg.DbgPrint("Volcando %s\n\n", *nombreArchivo);

	DWORD leidos = 0;
	BOOL ok;
	while ((ok = ReadFile(_hArchivo, _bloque, _bytesLinea, &leidos, NULL)))
		ImprimeLinea(_bloque, (int)leidos);	// la última línea puede estar incompleta
	_dbg.CheckError(!ok, "Error al leer el archivo %s\n", nombreArchivo);

	CloseHandle(_hArchivo);
	_hArchivo = INVALID_HANDLE_VALUE;	// ya no hay ningún archivo abierto
	*pBytesVolcados = (int)_offset;
	return ok != FALSE;
}

// Vuelca nBytes de memoria a partir de pDatos. La columna de la izquierda es el
// desplazamiento (offset) respecto a pDatos.
int VisorHex::VuelcaMemoria(const void* pDatos, size_t nBytes)
{
	if (_dbg.CheckError(pDatos == NULL, "VuelcaMemoria: puntero nulo.\n"))
		return -1;
	const uint8_t* p = (const uint8_t*)pDatos;	// void *: dirección sin tipo
	_origen = 'M';
	_offset = 0;
	_lineas = 0;
	_dbg.DbgPrint("Memoria: %zu bytes a partir de la dirección %p\n\n", nBytes, pDatos);
	for (size_t i = 0; i < nBytes; i += _bytesLinea) {
		size_t n = nBytes - i;
		if (n > (size_t)_bytesLinea)
			n = _bytesLinea;
		ImprimeLinea(p + i, (int)n);
	}
	return (int)_offset;
}

void VisorHex::Resumen(ResumenVolcado_t* pResumen)
{
	pResumen->origen = _origen;
	pResumen->bytesVolcados = _offset;
	pResumen->bytesLinea = (uint16_t)_bytesLinea;
	pResumen->bytesGrupo = (uint8_t)_bytesGrupo;
	pResumen->lineas = _lineas;
}

// Compone una línea en un buffer (técnica buffer + len de la práctica 1) y la imprime:
//      offset  columna hexadecimal, en grupos  |columna ASCII|
void VisorHex::ImprimeLinea(const uint8_t datos[], int nDatos)
{
	char linea[50];
	size_t len = snprintf(linea, sizeof(linea), "%08X  ", _offset);

	// 1. Columna hexadecimal, en grupos de _bytesGrupo bytes
	for (int i = 0; i < nDatos; i += _bytesGrupo) {
		int resto = nDatos - i;	// bytes que quedan por mostrar en esta línea
		if (_bytesGrupo == 1) {
			len += snprintf(linea + len, sizeof(linea) - len, "%02X ", datos[i]);
		}
		else if (resto >= _bytesGrupo) {	// grupo completo: se interpreta con la unión
			Grupo_t g;
			memcpy(g.b, datos + i, _bytesGrupo);
			if (_bytesGrupo == 2)
				len += snprintf(linea + len, sizeof(linea) - len, "%04X ", g.w);
			else if (_bytesGrupo == 4)
				len += snprintf(linea + len, sizeof(linea) - len, "%08X ", g.d);
			else
				len += snprintf(linea + len, sizeof(linea) - len, "%016llX ", (unsigned long long)g.q);
		}
		else {	// grupo incompleto (final del archivo): faltan los bytes más significativos
			for (int k = _bytesGrupo - 1; k >= 0; k--) {
				if (k < resto)
					len += snprintf(linea + len, sizeof(linea) - len, "%02X", datos[i + k]);
				else
					len += snprintf(linea + len, sizeof(linea) - len, "  ");
			}
			len += snprintf(linea + len, sizeof(linea) - len, " ");
		}
	}

	// 2. Relleno, para que la columna ASCII quede alineada cuando la línea está incompleta
	int gruposLinea = _bytesLinea / _bytesGrupo;
	int gruposDatos = (nDatos + _bytesGrupo - 1) / _bytesGrupo;
	for (int k = gruposDatos; k < gruposLinea; k++)
		len += snprintf(linea + len, sizeof(linea) - len, "%*s", 2 * _bytesGrupo + 1, "");

	// 3. Columna ASCII: los bytes no imprimibles (fuera de 0x20..0x7E) se muestran como '.'
	len += snprintf(linea + len, sizeof(linea) - len, " |");
	for (int i = 0; i < nDatos; i++) {
		char c = (datos[i] >= 0x20 && datos[i] <= 0x7E) ? (char)datos[i] : '.';
		len += snprintf(linea + len, sizeof(linea) - len, "%c", c);
	}
	_dbg.DbgPrint("%s|\n", linea);

	_offset += nDatos;
	_lineas++;
}

