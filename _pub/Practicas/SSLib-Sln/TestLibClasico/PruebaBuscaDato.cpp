// PruebaBuscaDato.cpp
// Comprueba FileSys::BuscaDatoEnArchivo (SSLib) tras la corrección del 7/11/2026.
// Uso: añadir este archivo a un proyecto con SSLib configurada (por ejemplo, TestLibClasico)
// y llamar a PruebaBuscaDatoEnArchivo() desde main. Para usarlo como proyecto propio,
// definir PRUEBA_CON_MAIN en las propiedades del proyecto (C/C++ -> Preprocesador).
// Con la DLL anterior fallan los casos marcados con (*): son los que la versión
// antigua no encontraba.
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <sslib/SSLib.h>
using namespace ss;

static bool EscribeArchivoPrueba(const char* ruta, const char* datos, size_t nBytes)
{
	HANDLE h = CreateFileA(ruta, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	DWORD escritos = 0;
	BOOL ok = WriteFile(h, datos, (DWORD)nBytes, &escritos, NULL);
	CloseHandle(h);
	return ok && escritos == nBytes;
}

static int Comprueba(FileSys& fs, const char* ruta, const char* bloque, int tamBuffer,
	int esperado, size_t bytesEsperados, const char* descripcion)
{
	size_t bytes = 0;
	int res = fs.BuscaDatoEnArchivo(bloque, strlen(bloque), ruta, &bytes, tamBuffer);
	bool ok = res == esperado && (esperado < 0 || bytes == bytesEsperados);
	printf("%s  %-28s tamBuffer=%-6d -> %d (esperado %d)\n", ok ? "  OK " : "FALLO", descripcion,
		tamBuffer, res, esperado);
	return ok ? 0 : 1;
}

bool PruebaBuscaDatoEnArchivo()
{
	FileSys fs;
	char dirTemp[MAX_PATH], ruta[MAX_PATH], ruta1MiB[MAX_PATH];
	GetTempPathA(MAX_PATH, dirTemp);
	snprintf(ruta, sizeof(ruta), "%sPruebaBuscaDato.txt", dirTemp);
	snprintf(ruta1MiB, sizeof(ruta1MiB), "%sPruebaBuscaDato1MiB.dat", dirTemp);
	int fallos = 0;

	// 1. Casos pequeños, con buffers de 1, 2, 3, 7 y 1024 bytes (ocurrencias partidas entre lecturas)
	struct { const char* texto; const char* bloque; int esperado; const char* descripcion; } casos[] = {
		{ "aab",               "ab",         1, "\"ab\" en \"aab\" (*)" },
		{ "BuscaBuscaTexto",   "BuscaTexto", 1, "\"BuscaTexto\" repetido (*)" },
		{ "aaab",              "aab",        1, "\"aab\" en \"aaab\" (*)" },
		{ "aabaabaaab",        "aabaaab",    1, "\"aabaaab\" (*)" },
		{ "xxabxx",            "ab",         1, "\"ab\" en \"xxabxx\"" },
		{ "aaaa",              "aa",         2, "sin solapar: \"aa\" en \"aaaa\"" },
		{ "abababab",          "abab",       2, "sin solapar: \"abab\"" },
		{ "sin coincidencias", "xyz",        0, "sin coincidencias" },
	};
	int tamanos[] = { 1, 2, 3, 7, 1024 };
	for (auto& c : casos) {
		if (!EscribeArchivoPrueba(ruta, c.texto, strlen(c.texto))) {
			printf("No se pudo crear %s\n", ruta);
			return false;
		}
		for (int t : tamanos)
			fallos += Comprueba(fs, ruta, c.bloque, t, c.esperado, strlen(c.texto), c.descripcion);
	}

	// 2. Archivo de CreaArchivoTest de 1 MiB: líneas de 74 bytes "NNNNNNNNNN;0123456789abc...XYZ\n"
	//    "0123456789abc" aparece una vez por línea: 14170 veces (la última línea está incompleta).
	//    "000001;" solo aparece en la línea 1 ("0000000001;"): la versión anterior no la encontraba.
	if (fs.CreaArchivoTest(ruta1MiB, 1048576, 65536)) {
		int tamanosGrandes[] = { 1, 1024, 10000 };
		for (int t : tamanosGrandes) {
			fallos += Comprueba(fs, ruta1MiB, "0123456789abc", t, 14170, 1048576, "1 MiB: \"0123456789abc\"");
			fallos += Comprueba(fs, ruta1MiB, "000001;", t, 1, 1048576, "1 MiB: \"000001;\" (*)");
		}
	}
	else {
		printf("No se pudo crear %s\n", ruta1MiB);
		fallos++;
	}

	// 3. Errores: deben devolver -1 (cada uno muestra además su mensaje de CheckError)
	fallos += Comprueba(fs, ruta, "a", 0, -1, 0, "error: tamBuffer 0");
	fallos += Comprueba(fs, ruta, "", 16, -1, 0, "error: bloque vacio");
	fallos += Comprueba(fs, "Z:\\no\\existe.txt", "a", 16, -1, 0, "error: archivo inexistente");

	DeleteFileA(ruta);
	DeleteFileA(ruta1MiB);
	printf("\n%s: %d fallos\n", fallos == 0 ? "CORRECTO" : "HAY FALLOS", fallos);
	return fallos == 0;
}

#ifdef PRUEBA_CON_MAIN
int main()
{
	setlocale(LC_ALL, "Spanish");
	bool ok = PruebaBuscaDatoEnArchivo();
	printf("\nPulse ENTRAR para terminar...\n");
	(void)getchar();
	return ok ? 0 : 1;
}
#endif
