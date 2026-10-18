#pragma once
/*
  VisorHexPrj26: visor hexadecimal de archivos. Práctica 3, sesión 2.

  Añadir a:
  Propiedades del proyecto -> Depuración -> Entorno
  (todas las configuraciones, todas las plataformas) la línea:

     PATH=$(AAComm)bin\sslib\$(PlatformTarget);%PATH%
*/

// Uso de _CRT_SECURE_NO_WARNINGS. Ver https://t.ly/aZuv
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <windows.h>
#include <sslib/SSLib.h>

#include "VisorHex.h"

// Códigos de terminación del programa
enum {
	ErrorFaltanArgumentos = 1,
	ErrorSobranArgumentos,
	ErrorBytesPorGrupo,
	ErrorBytesPorLinea,
	ErrorVariableAAComm,
	ErrorArchivo,
};

#define USO "Uso: VisorHexPrj26 archivo bytesPorDato (1, 2, 4 u 8) bytesPorLinea (8, 16 o 32) [test]\n"


// Carpeta de los archivos de prueba, relativa a la variable de entorno AAComm
#define CARPETA_PRUEBAS "..\\_utils\\_docs\\P1\\"
