#include "sslib/SSLib.h"
using namespace ss;
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


bool ss::Dbg::P4S2_CreaDatosPrueba(const char* directorio)
{
	Dbg dbg(true); // Salida por pantalla y por depuración
	bool todoOk = true;

	if (dbg.CheckError(directorio == nullptr || directorio[0] == '\0',
		"Directorio base no válido.\n")) {
		return false;
	}

	const char* dirs[] = { "P4S2",  // la ruta base es directorio + esta carpeta
		"carpeta", "carpeta\\sub" };
	struct { const char* nombre; uint64_t tam; } archivos[] = {
		{ "prueba1.dat", 1 },
		{ "prueba1000000.dat", 1000000 },
		{ "prueba4M.dat", 4 * 1024 * 1024 },
		{  "prueba64M.dat", 64 * 1024 * 1024 },
		{  "carpeta\\a.dat", 1000 },         // Carpeta del Ejercicio 4.24
		{  "carpeta\\b.dat", 20000 },
		{  "carpeta\\sub\\c.dat", 300000 },
		{  "carpeta\\z.dat", 4000000 },
	};
	FileSys fs;
	CHAR ruta[MAX_PATH], rutaAux[MAX_PATH], rutaBase[MAX_PATH];
	// Crea el directorio base
	bool ok = CreateDirectoryA(directorio, NULL);
	if (dbg.CheckError(!ok && GetLastError() != ERROR_ALREADY_EXISTS,
		"No se pudo crear el directorio %s\n", directorio)) {
		todoOk = false;
	}

	for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		if (i == 0) {
			if (dbg.CheckError(NULL == PathCombineA(rutaBase, directorio, dirs[i]),
				"No se pudo combinar la ruta %s con %s\n", directorio, dirs[i])) {
				todoOk = false;
				continue;
			}
			if (dbg.CheckError(0 == GetFullPathNameA(rutaBase, sizeof(ruta), ruta, NULL),
				"No se pudo obtener la ruta absoluta de %s\n", rutaBase)) {
				todoOk = false;
				continue;
			}
			strcpy(rutaBase, ruta);
		}
		else {
			if (dbg.CheckError(NULL == PathCombineA(rutaAux, rutaBase, dirs[i]),
				"No se pudo combinar la ruta %s con %s\n", rutaBase, dirs[i])) {
				todoOk = false;
				continue;
			}
			// Comvierte la ruta a ruta absoluta
			if (dbg.CheckError(0 == GetFullPathNameA(rutaAux, sizeof(ruta), ruta, NULL),
				"No se pudo obtener la ruta absoluta de %s\n", rutaAux)) {
				todoOk = false;
				continue;
			}
		}
		BOOL ok;

		ok= CreateDirectoryA(ruta, NULL);
		if (dbg.CheckError(!ok && GetLastError() != ERROR_ALREADY_EXISTS,
			"No se pudo crear el directorio %s\n", ruta)) {
			todoOk = false;
		}
	}
	for (size_t i = 0; i < sizeof(archivos) / sizeof(archivos[0]); i++) {
		if (dbg.CheckError(NULL == PathCombineA(rutaAux, rutaBase, archivos[i].nombre),
			"No se pudo combinar la ruta %s con %s\n", rutaBase, archivos[i].nombre)) {
			todoOk = false;
			continue;
		}
		// Comvierte la ruta a ruta absoluta
		if (dbg.CheckError(0 == GetFullPathNameA(rutaAux, sizeof(ruta), ruta, NULL),
			"No se pudo obtener la ruta absoluta de %s\n", rutaAux)) {
			todoOk = false;
			continue;
		}
		if (!fs.CreaArchivoTest(ruta, archivos[i].tam)) {
			dbg.CheckError(true, "No se pudo crear o validar el archivo de prueba %s\n", ruta);
			todoOk = false;
		}
	}

	return todoOk;
}
