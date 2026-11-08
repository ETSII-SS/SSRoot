// Sha256.cpp – Implementación con CNG (bcrypt). Es el único archivo que ve Win32.
#define _LIB_EXPORTS_
#include "sslib\SSLib.h"
#pragma comment(lib, "bcrypt.lib")

namespace ss {
	// Estado interno, invisible para quien usa la clase
	struct Sha256::Impl {
		BCRYPT_ALG_HANDLE  hAlg = nullptr;
		BCRYPT_HASH_HANDLE hHash = nullptr;

		~Impl()
		{
			if (hHash) BCryptDestroyHash(hHash);
			if (hAlg)  BCryptCloseAlgorithmProvider(hAlg, 0);
		}
	};

	// Convierte un NTSTATUS de error en una excepción
	static void Comprobar(NTSTATUS st, const char* funcion)
	{
		if (!BCRYPT_SUCCESS(st)) {
			char msg[128];
			snprintf(msg, sizeof(msg), "%s ha fallado (NTSTATUS 0x%08lX)", funcion, (unsigned long)st);
			throw std::runtime_error(msg);
		}
	}

	Sha256::Sha256() : p(new Impl)
	{
		Comprobar(BCryptOpenAlgorithmProvider(&p->hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0),
			"BCryptOpenAlgorithmProvider");
		Reiniciar();
	}

	Sha256::~Sha256()
	{
		delete p;
	}

	void Sha256::Reiniciar()
	{
		if (p->hHash) {
			BCryptDestroyHash(p->hHash);
			p->hHash = nullptr;
		}
		Comprobar(BCryptCreateHash(p->hAlg, &p->hHash, nullptr, 0, nullptr, 0, 0), "BCryptCreateHash");
	}

	void Sha256::Agregar(const void* datos, size_t tam)
	{
		const unsigned char* q = static_cast<const unsigned char*>(datos);

		while (tam > 0) {                                  // BCryptHashData admite como máximo un ULONG
			ULONG trozo = tam > 0x40000000 ? 0x40000000 : (ULONG)tam;
			Comprobar(BCryptHashData(p->hHash, const_cast<PUCHAR>(q), trozo, 0), "BCryptHashData");
			q += trozo;
			tam -= trozo;
		}
	}

	const Sha256::Hash256& Sha256::Parcial() const
	{
		BCRYPT_HASH_HANDLE copia = nullptr;

		Comprobar(BCryptDuplicateHash(p->hHash, &copia, nullptr, 0, 0), "BCryptDuplicateHash");
		NTSTATUS st = BCryptFinishHash(copia, hash, tamHASH256, 0);
		BCryptDestroyHash(copia);
		Comprobar(st, "BCryptFinishHash");
		return hash;
	}

	const Sha256::Hash256& Sha256::Finalizar()
	{
		Comprobar(BCryptFinishHash(p->hHash, hash, tamHASH256, 0), "BCryptFinishHash");
		Reiniciar();                                       // listo para un nuevo cálculo
		return hash;
	}
	const char* Sha256::AHex(char* cadenaAHex, size_t tam)
	{
		static const char dig[] = "0123456789abcdef";
		for (size_t i = 0; i < tamHASH256 && i < tam / 2; ++i) {
			unsigned char b = hash[i];
			cadenaAHex[i * 2] = dig[b >> 4];
			cadenaAHex[i * 2 + 1] = dig[b & 0x0F];
		}
		cadenaAHex[min(tamHASH256_HEX - 1, tam - 1)] = '\0';
		return cadenaAHex;
	}
}