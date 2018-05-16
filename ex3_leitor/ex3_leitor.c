//Leitor.c
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define BufferSize 100
#define Buffers 10

TCHAR NomeMemoria[] = TEXT("Nome da Memória Partilhada");

//TCHAR(*PtrMemoria)[Buffers][BufferSize];
typedef struct {
	TCHAR buf[Buffers][BufferSize];
} DADOS;

TCHAR NomeSemaforoPodeEscrever[] = TEXT("Semáforo Pode Escrever");
TCHAR NomeSemaforoPodeLer[] = TEXT("Semáforo Pode Ler");

HANDLE PodeEscrever;
HANDLE PodeLer;
HANDLE hMemoria;

int _tmain(void) {
	DADOS *shm;

	PodeEscrever = CreateSemaphore(NULL, Buffers, Buffers, NomeSemaforoPodeEscrever);
	PodeLer = CreateSemaphore(NULL, 0, Buffers, NomeSemaforoPodeLer);
	hMemoria = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DADOS), NomeMemoria);
	if (hMemoria == NULL || PodeLer == NULL || hMemoria == NULL) {
		_tprintf(TEXT("[Erro]Criação de objectos do Windows(%d)\n"), GetLastError());
		return -1;
	}
	shm = (DADOS*)MapViewOfFile(hMemoria, FILE_MAP_WRITE, 0, 0, sizeof(DADOS));
	if (shm == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"), GetLastError());
		return -1;
	}
	for (int i = 0;; ++i)
	{
		WaitForSingleObject(PodeLer, INFINITE);
		//_tprintf(shm->buf[i % 10]); // Reader reads data
		_tprintf(TEXT("Ler do buffer %d o valor '%s'\n"), i % Buffers, shm->buf[i % Buffers]);
		ReleaseSemaphore(PodeEscrever, 1, NULL);
	}
	UnmapViewOfFile(shm);
	CloseHandle(PodeEscrever);
	CloseHandle(PodeLer);
	CloseHandle(hMemoria);
	return 0;
}