#ifdef _SESION_1		
/* MUY IMPORTANTE : No añadir este archivo directamente a ningún proyecto. Copiar siempre los trozos de código
        que se indiquen en los ejercicios a archivos ya existentes en su proyecto. 
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






#endif
#ifdef _SESION_2		






/* MUY IMPORTANTE : No añadir este archivo directamente a ningún proyecto. Copiar siempre los trozos de código
        que se indiquen en los ejercicios a archivos ya existentes en su proyecto. 
*/

// Sesión 2
//--> desde aquí, código inicial para la práctica 5, sesión 2
#include <sslib/SSLib.h>
#include <locale.h>
using namespace ss;

#define DIR_DATOS _DIR_DATOS_PRUEBA_P4S2_POR_DEFECTO	// Datos de prueba de la P4
#define TEXTO_LINEA "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

// Una línea de los archivos de prueba (C:\tmp\P4S2\prueba*.dat). Miden 74 bytes:
//   0000000000;0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
struct LineaP4S2 {
	char numero[10];	// Número de la línea, con ceros a la izquierda (sin '\0')
	char separador;		// ';'
	char texto[62];		// TEXTO_LINEA (sin '\0')
	char fin;			// '\n'
};
static_assert(sizeof(LineaP4S2) == 74, "LineaP4S2 debe ocupar 74 bytes");
// Lo que hace falta para usar un archivo mapeado y para liberarlo después
struct ArchivoMapeado {
	HANDLE hArchivo;	// El archivo, abierto con CreateFileA
	HANDLE hMapa;		// El mapa del archivo, creado con CreateFileMappingA
	uint8_t* p;			// Dirección de la vista: el contenido del archivo
	int64_t tam;		// Tamaño del archivo, en bytes
};
// Cada una de estas funciones se usa o se completa en un ejercicio posterior
static bool Mapea(const char* ruta, bool paraEscribir, ArchivoMapeado* pAm);
static void Desmapea(ArchivoMapeado* pAm);
static void ImprimeLinea(const char* ruta, int64_t k);
static int64_t CuentaTextoMapa(const char* ruta, const char* texto);
static int64_t CuentaTextoBuffer(const char* ruta, const char* texto);
static void PrimeraLineaIncorrecta(const char* ruta);
static void InvierteBitMapa(const char* ruta, int64_t posicion, int bit);
static void OrdenaMapa(const char* ruta, bool inverso);

static Dbg dbg(true);
// Recuerde cambiar, en el código de la sesión 1, su función main por mainSesion1.
int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Spanish"); // Necesita #include <locale.h>
	dbg.CheckError(argc < 2 || argc > 5, 1, "Número de argumentos incorrecto.\n");
	const char* opcion = argv[1];
	// Si no se indica una ruta, se usa prueba1000000.dat
	const char* ruta = argc > 2 ? argv[2] : DIR_DATOS "\\prueba1000000.dat";
	int64_t n = argc > 3 ? atoll(argv[3]) : 0;	// Número de línea o posición
	if (_stricmp(opcion, "/prueba") == 0) {
		dbg.CheckError(!dbg.P4S2_CreaDatosPrueba(), 2, "Error al crear los datos.");
		printf("\nDatos de prueba creados en: %s\n", DIR_DATOS);
	}
	else if (_stricmp(opcion, "/linea") == 0)
		ImprimeLinea(ruta, n);
	else if (_stricmp(opcion, "/cuenta") == 0) {	// /cuenta ruta [texto]
		const char* texto = argc > 3 ? argv[3] : "0123456789"; // Texto a buscar
		dbg.CronoInicio();
		int64_t veces = CuentaTextoMapa(ruta, texto);
		printf("Archivo mapeado: %lld veces (%.3f s)\n", veces, dbg.CronoLee());
		dbg.CronoInicio();
		veces = CuentaTextoBuffer(ruta, texto);
		printf("Con ReadFile:    %lld veces (%.3f s)\n", veces, dbg.CronoLee());
	}
	else if (_stricmp(opcion, "/comprueba") == 0) {
		PrimeraLineaIncorrecta(ruta);
	}
	else if (_stricmp(opcion, "/invertir") == 0) {
		InvierteBitMapa(ruta, n, 0);
	}
	else if (_stricmp(opcion, "/ordena") == 0) {	// /ordena ruta [inversa]
		bool inverso = argc > 3 && _stricmp(argv[3], "inversa") == 0;
		OrdenaMapa(ruta, inverso);
	}
	else
		dbg.CheckError(true, 4, "Opción desconocida: %s\n", opcion);
	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();  // printf y getchar=> necesitan #include <stdio.h>
	return 0;
}




// Abre el archivo ruta y crea una vista de todo su contenido.
// Si paraEscribir es true, la vista permite modificar el archivo.
// Devuelve false si algo falla: en ese caso no hay nada que liberar.
bool Mapea(const char* ruta, bool paraEscribir, ArchivoMapeado* pAm){
	pAm->hArchivo = INVALID_HANDLE_VALUE;
	pAm->hMapa = NULL;
	pAm->p = NULL;
	pAm->tam = 0;
	// Completar (Ejercicio 5.23): abrir el archivo y obtener su tamaño
	// Completar (Ejercicio 5.23): crear el mapa con CreateFileMappingA.
	// Completar (Ejercicio 5.24): crear la vista con MapViewOfFile.
	return !dbg.CheckError(pAm->p == NULL, "Mapea: no se ha mapeado %s\n", ruta);
}
// Deshace lo que hizo Mapea.
void Desmapea(ArchivoMapeado* pAm) {
	// Completar (Ejercicio 5.24): deshacer la vista y cerrar.
}



// Imprime la línea k del archivo ruta, sin leer las anteriores.
void ImprimeLinea(const char* ruta, int64_t k){
	ArchivoMapeado am;
	if (!Mapea(ruta, false, &am))
		return;
	const LineaP4S2* lineas = (const LineaP4S2*)am.p;	// El archivo es una matriz
	int64_t nLineas = am.tam / sizeof(LineaP4S2);		// Solo las completas
	if (k < 0 || k >= nLineas)
		printf("%s tiene %lld líneas: no hay línea %lld\n", ruta, nLineas, k);
	else {
		printf("Línea %lld, en el byte %lld del archivo y en la dirección %p:\n",
			k, k * (int64_t)sizeof(LineaP4S2), (const void*)&lineas[k]);
		printf("%.*s\n", (int)sizeof(LineaP4S2) - 1, (const char*)&lineas[k]);
	}
	Desmapea(&am);
}




// Devuelve cuántas veces aparece texto en el archivo ruta, con el archivo
// mapeado. Devuelve -1 si no se puede mapear el archivo.
int64_t CuentaTextoMapa(const char* ruta, const char* texto){
	int64_t veces = 0;
	// Completar (Ejercicio 5.25)
	return veces;
}

// Lo mismo, leyendo con ReadFile en un buffer de 64 KB, como en la práctica 4.
int64_t CuentaTextoBuffer(const char* ruta, const char* texto){
	const DWORD tamBuffer = 64 * 1024;
	size_t tam = strlen(texto);
	HANDLE hArchivo = CreateFileA(ruta, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dbg.CheckError(hArchivo == INVALID_HANDLE_VALUE, "No abre %s\n", ruta))
		return -1;
	uint8_t* buffer = new uint8_t[tamBuffer];
	int64_t veces = 0;
	DWORD leidos = 0;
	while (ReadFile(hArchivo, buffer, tamBuffer, &leidos, NULL) && leidos > 0)
		for (DWORD i = 0; i + tam <= leidos; i++)
			if (memcmp(buffer + i, texto, tam) == 0)
				veces++;
	delete[] buffer;
	CloseHandle(hArchivo);
	return veces;
}
// true si pLinea apunta a una línea k como la escribió la práctica 4
static bool LineaCorrecta(const LineaP4S2* pLinea, int64_t k){
	char numero[12];
	snprintf(numero, sizeof(numero), "%010lld", k);
	return memcmp(pLinea->numero, numero, sizeof(pLinea->numero)) == 0
		&& pLinea->separador == ';' && pLinea->fin == '\n'
		&& memcmp(pLinea->texto, TEXTO_LINEA, sizeof(pLinea->texto)) == 0;
}

// Comprueba con LineaCorrecta todas las líneas completas del archivo ruta.
void PrimeraLineaIncorrecta(const char* ruta) {
	int64_t primera = -2;
	// Completar (Ejercicio 5.26)
	if (primera == -2)
		printf("No se puede mapear %s\n", ruta);
	else if (primera == -1)
		printf("Todas las líneas completas son correctas: %s\n", ruta);
	else if (primera >= 0)
		printf("La línea %lld es incorrecta: %s\n", primera, ruta);
	return;
}







// Invierte un bit del byte que está en posicion, como InvierteBit (sesión 1), pero
// sin SetFilePointerEx, ReadFile ni WriteFile: con el archivo mapeado.
// O eso pretende.
void InvierteBitMapa(const char* ruta, int64_t posicion, int bit){
	ArchivoMapeado am;
	if (Mapea(ruta, false, &am)) {
		am.p[posicion] = (uint8_t)(am.p[posicion] ^ (1 << bit));	// Invierte el bit
		Desmapea(&am);
		printf("Invertido el bit %d del byte %lld de %s\n",  bit, posicion, ruta);
	}
	else
		printf("No se puede mapear %s\n", ruta);
}


// Ejemplo de qsort: ordena una matriz de enteros de menor a mayor.
// qsort llama a la función de comparación con las direcciones de dos elementos.
// Debe devolver un valor negativo si el primero va antes, 0 si da igual y un
// valor positivo si el primero va después.
static int ComparaEnteros(const void* a, const void* b){
	int x = *(const int*)a;		// a apunta a un elemento de la matriz: un int
	int y = *(const int*)b;
	return x < y ? -1 : (x > y ? 1 : 0);
}
static void EjemploQsort(){
	int v[] = { 5, 2, 9, 1, 7 };
	size_t n = sizeof(v) / sizeof(v[0]);		// Número de elementos de v
	// Matriz, número de elementos, tamaño de cada uno y función de comparación
	qsort(v, n, sizeof(v[0]), ComparaEnteros);
	for (size_t i = 0; i < n; i++)
		printf("%d ", v[i]);					// Imprime: 1 2 5 7 9
	printf("\n");
}

// Como ComparaEnteros, pero cada elemento es una línea: compara sus números.
static int ComparaLineas(const void* a, const void* b){
	// Completar (Ejercicio 5.29)
	return 0;
}
// Para ordenar de mayor a menor basta con intercambiar a y b.
static int ComparaLineasInverso(const void* a, const void* b){
	return ComparaLineas(b, a);
}
// Ordena por su número las líneas completas del archivo ruta: de menor a mayor o,
// si inverso es true, de mayor a menor. Los bytes del final que no forman una
// línea completa no se mueven.
void OrdenaMapa(const char* ruta, bool inverso){
	ArchivoMapeado am = {};
	// Completar (Ejercicio 5.29): mapear el archivo.

	LineaP4S2* lineas = (LineaP4S2*)am.p;	// El archivo es una matriz de líneas
	size_t nLineas = (size_t)(am.tam / sizeof(LineaP4S2));
	dbg.CronoInicio();
	// Completar (Ejercicio 5.29): ordenar las nLineas líneas con qsort, como en
	//            EjemploQsort, con ComparaLineas o ComparaLineasInverso.

	printf("%zu líneas ordenadas en %.3f s, %s\n", 
		nLineas, dbg.CronoLee(), inverso ? "de mayor a menor" : "de menor a mayor");
	Desmapea(&am);
}









//<-- hasta aquí, código inicial para la práctica 5, sesión 2









#endif