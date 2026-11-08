#pragma once
#include <stdio.h>
#include <locale.h>
#include "sslib/SSLib.h"
using namespace ss;
using namespace std;


int TestP5_2026_main(int argc, char* argv[], char* envp[]);
int TestP4_2026_main(int argc, char* argv[], char* envp[]);

#ifdef TEST_LIB_CLASICO_CPP
static int BuscaTextoEnArchivos(const char* dirBusqueda, const char* textoABuscar);
#endif

