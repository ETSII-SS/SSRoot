#include "VisorHexPrj26.h"
using namespace ss;

int main(int argc, char* argv[], char* envp[])
{
	setlocale(LC_ALL, "Spanish");
	Dbg dbg(true);	// Errores por consola y por la ventana Salida del depurador

	// 1. Argumentos de la línea de comandos: archivo [bytesPorGrupo] [bytesPorLinea] [test]
	dbg.CheckError(argc < 2, ErrorFaltanArgumentos, "Faltan argumentos.\n" USO);
	dbg.CheckError(argc > 5, ErrorSobranArgumentos, "Sobran argumentos.\n" USO);

	// Primer argumento: archivo que se va a volcar
	const char* nombreArchivo = argv[0];

	// Segundo argumento
	int bytesGrupo = (int)argv[1];
	dbg.CheckError(bytesGrupo != 1 && bytesGrupo != 2 && bytesGrupo != 4 && bytesGrupo != 8,
		ErrorBytesPorGrupo, "Bytes por grupo no válido (%d). Debe ser 1, 2, 4 u 8.\n", bytesGrupo);

	// Tercer argumento (opcional): bytes por línea
	int bytesLinea = (int)argv[2];
	dbg.CheckError(bytesLinea != 8 && bytesLinea != 16 && bytesLinea != 32,
		ErrorBytesPorLinea, "Bytes por línea no válido (%d). Debe ser 8, 16 o 32.\n", bytesLinea);

	// Cuarto argumento (opcional): si vale test, el archivo se busca en la carpeta de pruebas
	bool modoTest = false;
	if (argc == 5 && 
		CompareStringA(LOCALE_USER_DEFAULT, LINGUISTIC_IGNORECASE,	argv[3], sizeof(argv[3]), 
			"test", sizeof("test")) == CSTR_EQUAL)
		modoTest = true;	// usamos CompareStringA porque permite ignorar mayúsculas/minusculas
	char ruta[MAX_PATH];
	// 2. Variable de entorno AAComm (solo en modo test): localiza la carpeta de pruebas
	if (modoTest) {
		char aaComm[MAX_PATH];
		DWORD n = GetEnvironmentVariableA("AAComm", aaComm, sizeof(aaComm));
		dbg.CheckError(n == 0 || n >= sizeof(aaComm), ErrorVariableAAComm,
			"No se pudo leer la variable de entorno AAComm.\n");
		snprintf(ruta, sizeof(ruta), "%s%s", aaComm, CARPETA_PRUEBAS, nombreArchivo);
		nombreArchivo = ruta;
	}
	// Comprueba que el archivo existe
	DWORD atributos = GetFileAttributesA(nombreArchivo);
	dbg.CheckError(atributos == INVALID_FILE_ATTRIBUTES, ErrorArchivo,
		"No se pudo abrir el archivo de pruebas:\n   %s\n", nombreArchivo);
	// Obtiene la ruta completa del archivo (para que se vea en el volcado)
	char rutaCompleta[MAX_PATH];
	dbg.CheckError(GetFullPathNameA(nombreArchivo, sizeof(rutaCompleta), rutaCompleta, NULL) == 0,
		ErrorArchivo, "No se pudo obtener la ruta completa del archivo:\n   %s\n", nombreArchivo);
	nombreArchivo = rutaCompleta;

	// 3. Volcado del archivo
	VisorHex visor(bytesLinea, bytesGrupo);
	int bytes = -1;
	bool ok = visor.VuelcaArchivo(nombreArchivo, &bytes);
	dbg.CheckError(!ok || bytes < 0, ErrorArchivo, "No se pudo volcar el archivo.\n");

	// 4. Resumen. Autocomprobación: el resumen también se vuelca, desde la memoria
	ResumenVolcado_t resumen;	// sin inicializar a propósito: deja ver el relleno (padding)
	visor.Resumen(&resumen);

	printf("\nVolcados %u bytes en %u líneas (%d bytes por dato, %d byte/s por línea).\n", resumen.bytesVolcados, resumen.lineas,
		resumen.bytesGrupo, resumen.bytesLinea);

	printf("\n\nEjemplo de volcado de memoria:\nEstructura ResumenVolcado_t (%zu bytes: char-uint32-uint16-uint8-uint32):\n",
		sizeof(ResumenVolcado_t));
	visor.VuelcaMemoria(&resumen, sizeof(resumen));

	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();
	return 0;
}
