MUY IMPORTANTE: No añadir este directamente a ningún proyecto. Copiar siempre los trozos de código 
        que se indican en cada ejercicio a archivos ya existentes. 
		
		
		
		
// Copiar esta función al mismo archivo que main, y llamarla desde main
// Un experimento con estructuras y uniones
void Eje2_Inicial() {
	struct {
		int entero;
		char car;
	} vEstructura = { 256, 1 }; // Define + inicializa la estructura
	union {
		int entero;
		char car;
	}  vUnion = { 256 }; // Define + inicializa la unión

	vUnion.car = 1;
	printf(__FUNCTION__ ":\n");
	printf("\tEstructura: %d Tam: %d\n", vEstructura.entero, sizeof(vEstructura));
	printf("\tUnion: %d Tam: %d\n", vUnion.entero, sizeof(vUnion));
}
 // <- Hasta aquí ejercicio 2.1



vUnion.entero = 0x89abcdef; // byte 0: 0xef, byte 1: 0xcd, byte 2: 0xab, byte 3: 0x89
printf("vUnion.entero vale 0x%x\n", vUnion.entero);
printf("El valor del byte 2 de vUnion.entero es: 0x%x\n", 
	???);  // sustituir ??? por la xpresión que calcula el byte 2 de vUnion.entero 








#define VAL_ENTERO 0x12345678
typedef struct {
	int entero;
	char car;
} tEstructura_t;
// Escribe un bloque de memoria en un archivo binario.
// Devuelve el número de bytes escritos, o -1 si no se puede abrir el archivo.
int GuardaBloqueMemoria(const char nombre[], const void* pInicioBloque, size_t tamBloque)
{
	FILE* f;
	if (fopen_s(&f, nombre, "wb") != 0)
		return -1;
	int n = (int)fwrite(pInicioBloque, 1, tamBloque, f);
	fclose(f);
	return n;
}
void Ej2_EstructuraEnDisco()  //previo
{
	tEstructura_t varLocal = { VAL_ENTERO, 5 };     
	printf(__FUNCTION__ ":\n");
	printf("\tsizeof(tEstructura_t) = %d\n", 
		(int)sizeof(tEstructura_t));
	// Guarda un campo en un archivo binario y lo muestra con un visor hexadecimal.
	GuardaBloqueMemoria("campo_car.bin", &varLocal.car, varLocal.car);
	Eje1_Dumpbin("campo_car.bin");	// el visor hexadecimal de la P1
}




typedef union {
	uint8_t d8;	// Interpreta el union como un entero de 8 bits sin signo
	uint32_t d32; // Interpreta el union como un entero de 32 bits sin signo
} ejUnion_t;

#define TAM_BYTES_EN_LINEA 16
// Imprime el valor de un contador y una línea hexadecimal.
// Devuelve el número de caracteres escritos.
size_t ImprimeLineaHex(const unsigned char datos[], size_t nDatos, int* pContador, int nroBytesPorDato = 1)
{
	ejUnion_t* pUnion;  // Pista: ¿para qué podemos usar esto?
	char buffer[TAM_BYTES_EN_LINEA * 3 + 100];
	size_t len = snprintf(buffer, sizeof(buffer), "%08X  ", *pContador);
	for (size_t i = 0; i < TAM_BYTES_EN_LINEA && i < nDatos; i += nroBytesPorDato) {
		if (nroBytesPorDato == 1) {
			unsigned char dato8 = datos[i];
			len += snprintf(buffer + len, sizeof(buffer) - len, "%02X ", dato8);
			(*pContador)++; // incrementa el valor del contador
		}
	}
	len = printf("%s\n", buffer);
	return len;
}




// Eje1_Dumpbin de repuesto. Este es una version de Eje1_Dumpbin, que se hizo en la práctica anterior
// Vuelca por pantalla el contenido completo de un archivo, en hexadecimal, en
// líneas de 16 bytes precedidas por la posición del primer byte de la línea.
// Devuelve el número de bytes volcados, o -1 si no se puede abrir el archivo.
int Eje1_Dumpbin(const char nombre[])
{
	const size_t TAM_LINEA_DUMP = 16;
	unsigned char bloque[TAM_LINEA_DUMP], buffer[TAM_LINEA_DUMP * 3 + 100];
	FILE* f;
	if (fopen_s(&f, nombre, "rb") != 0) {
		printf("Error al abrir el archivo %s.\n", nombre);
		return -1;
	}
	printf("Contenido del archivo %s\n", nombre);
	int contador = 0;
	size_t n = fread(bloque, 1, TAM_LINEA_DUMP, f);
	while (n > 0) {                 // n < TAM_LINEA_DUMP: última línea
		ImprimeLineaHex(bloque, (size_t)n, &contador);
		n = fread(bloque, 1, TAM_LINEA_DUMP, f);   // n == 0: fin de archivo
	}
	fclose(f);
	return contador;
}






tEstructura_t var1 = { 100, 1 };
tEstructura_t var2 = { 100, 1 };
void Eje2_EfectosDelAlineamiento() {
	tEstructura_t var3 = { 100, 1 };
	int comparacion = memcmp(&var1, &var2, sizeof(tEstructura_t));
	printf(__FUNCTION__ ":\n");
	printf("\tresultado de comparar var1 y var2: %d\n", comparacion);
	comparacion = memcmp(&var1, &var3, sizeof(tEstructura_t));
	printf("\tresultado de comparar var1 y var3: %d\n", comparacion);
}
	
	
	
	
	
// Declaraciones asociadas a una clase.
// Namespace: agrupación de clases, funciones y variables globales
namespace ss {
	
	class Crono {	// Declaración de la clase Crono

	private:	//opcional. por defecto todo es privad

	public:
		Crono();  // Constructor (opcional)
		~Crono(); // Destructor (opcional)
		
	}; // fin clase
	
	
} // fin namespace ss

// Definiciones asociadas a una clase.
namespace ss {  // Namespace: agrupación de clases, funciones y variables globales

	// Definición del constructor
	Crono::Crono()
	{
	}
	// Definición del destructor
	Crono::~Crono()
	{
	}

} // fin namespace ss

void TestClase() {
	ss::Crono crono;
	crono.Inicio();
	Sleep(1200);
	double segs = crono.Lee();
	printf("Han pasado %f segundos.\n", segs);
}



/// En Java, la prueba básica de una clase similar a Crono sería
Crono crono= new Dbg();  
crono.Inicio();
Thread.sleep(1200); // Espera 1200 ms. En C-Windows usaríamos Sleep(1200)
float segs= crono.Lee();
System.out.printf("El tiempo transcurrido %f segs\n", segs);




void TestParametros(int valor, int par1, int par2[], Struct1_t par3, Union1_t par4, Clase1 par5) {
	par1 = par2[0]= par3.entero= par4.entero= par5.entero= valor;
}


// Copiar esto a la función main
TestParametros(10, var1, var2, var3, var4, var5);
printf("Valores tras la llamada printf: var1=%d, var2[0]=%d, var3.entero=%d, var4.entero=%d, var5.entero=%d\n",
	var1, var2[0], var3.entero, var4.entero, var5.entero);


