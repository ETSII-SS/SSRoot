MUY IMPORTANTE : No añadir este directamente a ningún proyecto.Copiar siempre los trozos de código
que se indican en cada ejercicio a archivos ya existentes.



// Copiar esto dentro de la función main, tras setlocale...

// Clases declaradas en SSLib. Hay que incluir sslib/SSLib.h
Dbg dbg(true);
FileSys fs;
// Para depurar este programa, poner . en Propiedades del proyecto->Depurar->Directorio de trabajo
const char* pDir = argv[1];		//Nombre del directorio a procesar
// comprueba si el directorio existe
dbg.CheckError(argc < 2, 1, "Debe indicar el nombre del directorio a procesar");
DWORD atributos = GetFileAttributesA(argv[1]);
dbg.CheckError(atributos == INVALID_FILE_ATTRIBUTES, 2, "No se ha dado un nombre de directorio correcto: %s", argv[1]);
dbg.CheckError((atributos& FILE_ATTRIBUTE_DIRECTORY) == 0, 3, "No es un directorio: %s", argv[1]);

// Obtiene la lista de archivos en el directorio
int encontrados;
char** plistaArchivos;  // Este tipo se puede usar como dirección de una matriz de punteros a char. 
// Es similar a char* envp[], pero esa nomenclatura NO se puede usar en una definición de variable.

plistaArchivos = fs.ArchivosEnDirectorio(&encontrados, pDir);
dbg.CheckError(plistaArchivos == nullptr, 4, "No se han encontrado archivos en la carpeta %s\n", pDir);

printf("Se han encontrado %d archivos en la carpeta %s\n", encontrados, pDir);












static void ImprimeArchivosEnDirectorio(char* listaArchivos[]) {
	int i = 0;
	// Imprime la lista de archivos mientras no se encuentre un puntero nulo
	while (listaArchivos[i] = nullptr)
	{
		printf("#%d: %s\n", i, listaArchivos[i]);
		i++;
	}
}


// Sesión 2
//Ejercicio inicial: 
//--> desde aquí, código inicial para la práctica 4, sesión 2
#include <sslib/SSLib.h>
using namespace ss;
// Cada una de estas funciones se completa en un ejercicio posterior
static double ProcesaRuta(const char* ruta, int tamBuffer);
static void Barrido();
static void Instala(bool instalar);
#define  DIR_DATOS_P4S2 _DIR_DATOS_PRUEBA_P4S2_POR_DEFECTO
static Dbg dbg(true);
// Recuerde cambiar, en el código de la sesión 1, su función main por mainSesion1. 
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Spanish"); // Necesita #include <locale.h>
	dbg.CheckError(argc < 2 || argc > 4, 1, "Número de argumentos incorrecto.\n");

	if (_stricmp(argv[1], "/prueba") == 0) {
		dbg.CheckError(!dbg.P4S2_CreaDatosPrueba(), 2, "Error al crear datos de prueba.");
		printf("\nDatos de prueba creados en: %s\n", DIR_DATOS_P4S2);
	}
	else if (_stricmp(argv[1], "/barrido") == 0)
		Barrido();
	else if (_stricmp(argv[1], "/ventana") == 0) {
		//ProcesaRuta(argv[2], 1, true, true); // Da el resultado en ventana de texto (no en consola)
	}
	else
		ProcesaRuta(argv[1], 1);

	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();  // printf y getchar=> necesitan #include <stdio.h>
	return 0;
}











double ProcesaRuta(const char* ruta, int tamBuffer)
{
	Sha256 sha; // Objeto para calcular el hash256
	int tamArchivo = 0;
	char cadHash[Sha256::tamHASH256_HEX];

	uint8_t* buffer = new uint8_t[tamBuffer];  // Buffer de lectura, reservado dinámicamente (en heap)
	dbg.CronoInicio();
	// Completar el código para abrir el archivo, procesarlo en bloques de tamaño tamBuffer,
	           // y calcular su hash256 usando la clase Sha256.
			   // 
	// Ejemplo de uso de Sha256 para calcular el hash de un bloque de datos:
	// Se leen los datos de alguna forma. En este caso, solo lo ponemos a 0
	memset(buffer, 0, tamBuffer);
	sha.Agregar(buffer, tamBuffer);
	//... Para procesar un archivo grande, se puede leer en bloques y llamar a sha.Agregar() para cada bloque leído.
	sha.Finalizar();
	double tiempoInvertido = dbg.CronoLee();	
	printf("Calulado Hash256 para el archivo: %s\n", ruta);
	printf("  %s \n", sha.AHex(cadHash, sizeof(cadHash)));

	printf("En %f s. (%d bytes, en bloques de %d bytes)\n", tiempoInvertido, tamArchivo, tamBuffer);
	delete[] buffer;
	return tiempoInvertido;
}




void Barrido() {
	int tams[] = { 1, 16, 256, 4 * 1024, 64 * 1024, 1024 * 1024, 16 * 1024 * 1024 };
	double tiemposConHash[sizeof(tams) / sizeof(tams[0])];
	for (size_t i = 0; i < sizeof(tams) / sizeof(tams[0]); i++) {
		// Buffers pequeños, con el archivo de 4 MB: con el de 64 MB tardaría minutos
		const char* ruta = tams[i] <= 256 ? DIR_DATOS_P4S2 "\\prueba4M.dat"
			: DIR_DATOS_P4S2 "\\prueba64M.dat";
		tiemposConHash[i] = ProcesaRuta(ruta, tams[i]);
	}
	// Imprime resultados
	printf("%10s %14s  %14s \n", "Buffer", "SHA-256 MB/s", "Solo Leer MB/s");
	for (size_t i = 0; i < sizeof(tams) / sizeof(tams[0]); i++) {
		double mb = (tams[i] <= 256L ? 4.0 : 64.0);
		printf("%10d %14.2f  %14.2f \n", tams[i], mb / tiemposConHash[i], 0.0);
	}
}




void Instala(bool instalar) {}




//<--Hasta aquí, código inicial para la práctica 4, sesión 2







