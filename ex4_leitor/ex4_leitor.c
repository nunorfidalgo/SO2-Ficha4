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
	int in, out;
} DADOS;

TCHAR NomeSemaforoPodeEscrever[] = TEXT("Semáforo Pode Escrever");
TCHAR NomeSemaforoPodeLer[] = TEXT("Semáforo Pode Ler");

HANDLE PodeEscrever;
HANDLE PodeLer;
HANDLE hMemoria;

int _tmain(void) {
	DADOS *shm;
	int pos;
	HANDLE MutexOut;

	system("cls");
	// criar mutex para acesso ao IN (nome = "Mutex IN")...
	// mutex
	MutexOut = CreateMutex(NULL, FALSE, "MutexIn");
	if (MutexOut == NULL) {
		_tprintf(TEXT("CreateMutex error: %d\n"), GetLastError());
		return 1;
	}

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

		// esperar o mutex
		WaitForSingleObject(MutexOut, INFINITE);
		// copiar o valor de IN para pos // pos = i % Buffers;
		pos = shm->out;
		// Actualizar valor de IN (shm)
		shm->out = (shm->out + 1) % Buffers;
		// libertar o mutex
		ReleaseMutex(MutexOut);

		_tprintf(shm->buf[pos]); // Reader reads data
		ReleaseSemaphore(PodeEscrever, 1, NULL);
	}
	UnmapViewOfFile(shm);

	CloseHandle(MutexOut); 	// apagar mutex

	CloseHandle(PodeEscrever);
	CloseHandle(PodeLer);
	CloseHandle(hMemoria);
	return 0;
}