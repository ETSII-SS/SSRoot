/*===========================================================================
  Conjetura de Collatz: partiendo de cualquier entero n > 0, y aplicando
  repetidamente
		n par    ->  n / 2
		n impar  ->  3 * n + 1
  se acaba siempre llegando a 1. Nadie lo ha demostrado, pero se ha
  comprobado para valores enormes de n.

  El programa recorre n = 1 ... N_LIMITE y busca cual de esas secuencias
  es la mas larga.

  Resultados esperados para N_LIMITE = 1000:
		secuencia mas larga : n = 6171, con 261 pasos
		total de pasos      : 849666

  Tamano de los bucles:
		bucle exterior : 10000 vueltas (miles de pasos)
		bucle interior : longitud variable, de 0 a 261 vueltas
		total          : unos 850000 pasos elementales por repeticion
===========================================================================*/

#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <stdint.h>     /* uint32_t, uint64_t                              */
#include <inttypes.h>   /* PRIu32, PRIu64: formatos de printf para stdint  */

#define N_LIMITE         1000U   /* rango analizado: 1 .. N_LIMITE        */
#define N_REPETICIONES     200U   /* para que el tiempo sea medible        */

static uint64_t dgbContador = 0;  // solo para ver cuantas veces pasa por determinado punto del código


/*---------------------------------------------------------------------------
  Devuelve el numero de pasos necesarios para llegar de nInicial hasta 1.

  El valor intermedio puede crecer muy por encima del inicial (para
  n < 10000 llega a superar los 27 millones), por eso se usa uint64_t.
---------------------------------------------------------------------------*/
static uint32_t LongitudCollatz(uint32_t nInicial)
{
	uint64_t n = (uint64_t)nInicial;
	uint32_t nPasos = 0;

	while (n != 1)
	{
		// El objetivo es comprobar el retraso debido a los breakpoints condicionales. Por ejemplo, poner la condición n == 0
		if (n % 2 == 0)
			n = n / 2;
		else
			n = 3 * n + 1;

		nPasos++;
	}

	return nPasos;
}

/*---------------------------------------------------------------------------
  Recorre todo el rango y devuelve, por referencia, el inicio y la longitud
  de la secuencia mas larga. El valor de retorno es el total de pasos.
---------------------------------------------------------------------------*/
static uint64_t AnalizarRango(uint32_t nLimite,
	uint32_t* pMejorInicio,
	uint32_t* pMejorLongitud)
{
	uint32_t n;
	uint32_t nPasos;
	uint32_t nMejorInicio = 1;
	uint32_t nMejorLongitud = 0;
	uint64_t nTotalPasos = 0;

	// Poner un breakpoint condicional aquí, con una condición que no se cumple nunca:
	// el tiempo aumenta un poco. 
	for (n = 1; n <= nLimite; n++)
	{
		dgbContador++;

		nPasos = LongitudCollatz(n);
		nTotalPasos += nPasos;

		if (nPasos > nMejorLongitud)
		{
			// quitar el anterior y ponerlo aquí. 
			// El tiempo aumenta bastante.
			nMejorLongitud = nPasos;
			nMejorInicio = n;
		}
	}
	// quitar el breakpoint condicional anterior y ponerlo aquí. El tiempo se dispara.
	*pMejorInicio = nMejorInicio;
	*pMejorLongitud = nMejorLongitud;

	return nTotalPasos;
}

int main(int argc, char* argv[], char* envp[])
{
	clock_t  tInicio, tFin;
	double   segundos;
	uint32_t nRepeticion;
	uint32_t nMejorInicio = 0;
	uint32_t nMejorLongitud = 0;
	uint64_t nTotalPasos = 0;

	setlocale(LC_ALL, "Spanish"); // Necesita #include <locale.h>

	tInicio = clock();   // Necesita #include <time.h>

	for (nRepeticion = 0; nRepeticion < N_REPETICIONES; nRepeticion++)
		nTotalPasos = AnalizarRango(N_LIMITE, &nMejorInicio, &nMejorLongitud);

	tFin = clock();

	/* CUIDADO: clock_t y CLOCKS_PER_SEC son tipos ENTEROS. Si se escribe
	   (tFin - tInicio) / CLOCKS_PER_SEC la división es entera y se pierden
	   los decimales. Hay que convertir a double ANTES de dividir.          */
	segundos = (double)(tFin - tInicio) / (double)CLOCKS_PER_SEC;

	printf("\nRango analizado     : 1 .. %" PRIu32 "\n", (uint32_t)N_LIMITE);
	printf("Repeticiones        : %" PRIu32 "\n", (uint32_t)N_REPETICIONES);
	printf("Secuencia más larga : n = %" PRIu32 ", con %" PRIu32 " pasos\n",
		nMejorInicio, nMejorLongitud);
	printf("Contador            : %" PRIu64 "\n", dgbContador);
	printf("Tiempo transcurrido : %.3f s\n", segundos);

	printf("\nPor favor, pulse la tecla ENTRAR para terminar ...\n");
	(void)getchar();  // printf y getchar=> necesitan #include <stdio.h>

	return 0;
}