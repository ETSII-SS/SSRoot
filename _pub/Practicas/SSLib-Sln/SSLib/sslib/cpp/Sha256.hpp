// Sha256.h — Cálculo de SHA-256 por bloques.
// Interfaz opaca: no incluye <windows.h> ni <bcrypt.h> ni expone ningún tipo de CNG.
#pragma once

namespace ss {
	class LIB_API Sha256 {
	public:
		static constexpr size_t tamHASH256 = 32;
		static constexpr size_t tamHASH256_HEX = tamHASH256 * 2 + 1; // 64 caracteres hexadecimales + '\0'
		typedef unsigned char Hash256[tamHASH256];      // 32 bytes = 256 bits

		Sha256();                                       // lanza std::runtime_error si falla
		~Sha256();
		Sha256(const Sha256&) = delete;                 // no copiable
		Sha256& operator=(const Sha256&) = delete;
		void    Agregar(const void* datos, size_t tam);	// añade un bloque de cualquier tamaño
		const Hash256& Parcial() const;                 // hash de lo añadido hasta ahora, sin terminar
		const Hash256& Finalizar();                     // hash final; el objeto queda reiniciado
		void    Reiniciar();                            // descarta lo añadido y empieza de nuevo

		const char* AHex(char *dest, size_t tam);       // "e3b0c442..." (64 caracteres)
														   // solo funciona correctamente si
													       // se ha ejecutado antes Parcial o Finalizar			

	private:
		struct Impl;                                       // definida solo en Sha256.cpp
		Impl* p = nullptr;
		mutable Hash256 hash;

	};
}