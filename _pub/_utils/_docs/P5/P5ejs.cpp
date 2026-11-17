/* MUY IMPORTANTE : No añadir este directamente a ningún proyecto.Copiar siempre los trozos de código
        que se indican en cada ejercicio a archivos ya existentes. 
*/		
		

// Sesión 1
//--> desde aquí, código inicial para la práctica 5, sesión 1
#include <sslib/SSLib.h>
#include <locale.h>
using namespace ss;

#define DIR_DATOS    _DIR_DATOS_PRUEBA_P4S2_POR_DEFECTO   // Los datos de prueba son los de la práctica 4
#define FLUJO_SELLO  ":SS.sha256"   // Nombre del flujo donde se guarda la huella (el sello)
#define TAM_BUFFER   (64 * 1024)    // Tamaño del buffer de lectura
enum Resultado { CORRECTO, MODIFICADO, SIN_SELLO };
static const char* textoResultado[] = { "CORRECTO", "MODIFICADO", "SIN SELLO" };

// Cada una de estas funciones se usa o se completa en un ejercicio posterior
static void CalculaHuella(const char* ruta, char cadHash[], int64_t* pTamArchivo);
static bool Sella(const char* ruta);
static Resultado Verifica(const char* ruta);
static bool InvierteBit(const char* ruta, int64_t posicion, int bit);

static Dbg dbg(true);
int main(int argc, char* argv[]){
	setlocale(LC_ALL, "Spanish"); // Necesita #include <locale.h>
	dbg.CheckError(argc < 2 || argc > 4, 1, "Número de argumentos incorrecto.\n");

	if (_stricmp(argv[1], "/prueba") == 0) {
		dbg.CheckError(!dbg.P4S2_CreaDatosPrueba(), 2, "Error al crear datos de prueba.");
		printf("\nDatos de prueba creados en: %s\n", DIR_DATOS);
	}
	else if (argv[1][0] != '/') {	// Sin opción: solo calcula la huella de argv[1]
		char cadHash[Sha256::tamHASH256_HEX];
		int64_t tamArchivo;
		CalculaHuella(argv[1], cadHash, &tamArchivo);
		printf("%s  %s (%lld bytes)\n", cadHash, argv[1], tamArchivo);
	}
	else {							// El resto de las opciones necesitan una ruta en argv[2]
		dbg.CheckError(argc < 3, 3, "Falta la ruta.\n");
		if (_stricmp(argv[1], "/sellar") == 0)
			Sella(argv[2]);
		else if (_stricmp(argv[1], "/verificar") == 0)
			printf("%s: %s\n", textoResultado[Verifica(argv[2])], argv[2]);
		else if (_stricmp(argv[1], "/invertir") == 0)
			InvierteBit(argv[2], argc > 3 ? atoll(argv[3]) : 0, 0);
		else
			dbg.CheckError(true, 4, "Opción desconocida: %s\n", argv[1]);
	}
	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();  // printf y getchar=> necesitan #include <stdio.h>
	return 0;
}





// Calcula la huella SHA-256 del archivo ruta. Es ProcesaRuta, de la práctica 4, ya terminada.
// Deja en cadHash la huella en hexadecimal y en *pTamArchivo el número de bytes leídos.
void CalculaHuella(const char* ruta, char cadHash[], int64_t* pTamArchivo) {
	Sha256 sha; // Objeto para calcular el hash256
	*pTamArchivo = 0;
	HANDLE hArchivo = CreateFileA(ruta, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	dbg.CheckError(hArchivo == INVALID_HANDLE_VALUE, 5, "No se puede abrir %s\n", ruta);
	uint8_t* buffer = new uint8_t[TAM_BUFFER];  // Buffer de lectura, reservado dinámicamente (en heap)
	DWORD leidos = 1;
	BOOL ok;
	while (leidos>0) {
		ok = ReadFile(hArchivo, buffer, TAM_BUFFER, &leidos, NULL);
		dbg.CheckError(!ok, "Error al leer %s\n", ruta);
		if (leidos>0) {
			sha.Agregar(buffer, leidos);	
			*pTamArchivo += leidos;
		}
	} 
	CloseHandle(hArchivo);
	delete[] buffer;
	sha.Finalizar();
	sha.AHex(cadHash, Sha256::tamHASH256_HEX);
}



// Guarda la huella del archivo ruta en un flujo del propio archivo: ruta:SS.sha256
bool Sella(const char* ruta)
{
	char cadHash[Sha256::tamHASH256_HEX];
	char nombreFlujo[SSLIB_MAX_RUTA];
	int64_t tamArchivo;
	CalculaHuella(ruta, cadHash, &tamArchivo);
	snprintf(nombreFlujo, sizeof(nombreFlujo), "%s%s", ruta, FLUJO_SELLO);
	// Completar: crear el flujo nombreFlujo, escribir en él los 64 caracteres de la huella,
	//            comprobar que se han escrito todos y cerrar.
	printf("Sellado: %s\n   %s\n", ruta, cadHash);
	return true;
}
// Comprueba si el archivo ruta ha cambiado desde que se selló.
Resultado Verifica(const char* ruta)
{
	char nombreFlujo[SSLIB_MAX_RUTA];
	snprintf(nombreFlujo, sizeof(nombreFlujo), "%s%s", ruta, FLUJO_SELLO);
	// Completar: abrir el flujo nombreFlujo para leer. Si no existe, devolver SIN_SELLO.
	// Completar: leer el sello (64 caracteres) y cerrar.
	// Completar: calcular la huella actual, compararla con el sello
	//            y devolver CORRECTO o MODIFICADO.
	return SIN_SELLO;
}




// Invierte un bit del byte que está en posicion, sin leer ni escribir el resto del archivo.
// O eso pretende.
bool InvierteBit(const char* ruta, int64_t posicion, int bit)
{
	HANDLE hArchivo = CreateFileA(ruta, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dbg.CheckError(hArchivo == INVALID_HANDLE_VALUE, "No se puede abrir %s\n", ruta))
		return false;

	LARGE_INTEGER pos;
	pos.QuadPart = posicion;
	uint8_t dato = 0;
	DWORD n = 0;
	bool ok = SetFilePointerEx(hArchivo, pos, NULL, FILE_BEGIN)	// Coloca el puntero de archivo
		&& ReadFile(hArchivo, &dato, 1, &n, NULL) && n == 1;	// Lee un byte
	if (ok) {
		dato = (uint8_t)(dato ^ (1 << bit));					// Invierte el bit
		ok = WriteFile(hArchivo, &dato, 1, &n, NULL) && n == 1;	// Escribe el byte
	}
	dbg.CheckError(!ok, "No se puede modificar el byte %lld de %s\n", posicion, ruta);
	CloseHandle(hArchivo);
	return ok;
}



//<-- hasta aquí, código inicial para la práctica 5, sesión 1

