//Escritor.c
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define BufferSize 100

#define Buffers 10
TCHAR NomeMemoria[] = TEXT("Nome da Mem�ria Partilhada");

//TCHAR(*PtrMemoria)[Buffers][BufferSize];
typedef struct {
	TCHAR buf[Buffers][BufferSize];
	// add: o valor indice IN (int)... 
	int In, Out; // para o leitor
} DADOS;

TCHAR NomeSemaforoPodeEscrever[] = TEXT("Sem�foro Pode Escrever");
TCHAR NomeSemaforoPodeLer[] = TEXT("Sem�foro Pode Ler");

HANDLE PodeEscrever;
HANDLE PodeLer;
HANDLE hMemoria;

int _tmain(void) {
	DADOS *shm;
	int pos;
	char init = 1;

	HANDLE MutexIn;

	system("cls");

	// criar mutex para acesso ao IN (nome = "Mutex IN")...


	//PodeEscrever = CreateSemaphore(NULL, Buffers, Buffers, NomeSemaforoPodeEscrever);
	PodeEscrever = CreateSemaphore(NULL, Buffers, Buffers, NomeSemaforoPodeEscrever);
	PodeLer = CreateSemaphore(NULL, 0, Buffers, NomeSemaforoPodeLer);
	hMemoria = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DADOS), NomeMemoria);
	if (PodeEscrever == NULL || PodeLer == NULL || hMemoria == NULL) {
		_tprintf(TEXT("[Erro]Cria��o de objectos do Windows(%d)\n"), GetLastError());
		return -1;
	}

	// verificar se ja existe shm
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		init = 0;
	}

	shm = (DADOS*)MapViewOfFile(hMemoria, FILE_MAP_WRITE, 0, 0, sizeof(DADOS));
	if (shm == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		return -1;
	}

	// Criar MUTEX
	// mutex
	MutexIn = CreateMutex(NULL, TRUE, "MutexIn");
	if (MutexIn == NULL) {
		_tprintf(TEXT("CreateMutex error: %d\n"), GetLastError());
		return 1;
	}

	// verificar se ja existe shm
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		// colocar IN a zero...
		shm->In = 0;
		shm->Out = 0;
		// (Mutex ou) restaurar valor do semaforo PodeEscrever(+10)... ou seja release de 10 unidades, alterar na criacao para 0 ate 10
		/*ReleaseSemaphore(PodeEscrever, 10, NULL);*/
		ReleaseMutex(MutexIn);
	}

	for (int i = 0; i < 100; i++)
	{
		WaitForSingleObject(PodeEscrever, INFINITE);

		// esperar o mutex
		WaitForSingleObject(MutexIn, INFINITE);
		// copiar o valor de IN para pos // pos = i % Buffers;
		pos = shm->In;
		// Actualizar valor de IN (shm)
		shm->In = (shm->In + 1) % Buffers;
		// libertar o mutex


		_tprintf(TEXT("Escrever para buffer %i\n"), i);
		_stprintf_s(shm->buf[pos], BufferSize, TEXT("Pedido%d_#%02i"), GetCurrentProcessId(), i);

		ReleaseMutex(MutexIn);

		_tprintf(TEXT("Escrever para o buffer %d o valor '%s'\n"), pos, shm->buf[pos]);
		Sleep(1000);
		ReleaseSemaphore(PodeLer, 1, NULL);
	}
	UnmapViewOfFile(shm);

	CloseHandle(MutexIn); 	// apagar mutex

	CloseHandle(PodeEscrever);
	CloseHandle(PodeLer);
	CloseHandle(hMemoria);
	return 0;
}