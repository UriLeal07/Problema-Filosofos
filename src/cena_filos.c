#include "filosofos.h"

#define N 5								// Numero de filosofos
#define IZQUIERDO(i) ((i+N-1) % N)		// Numero del vecino izquierdo de i
#define DERECHO(i) ((i+1) % N)			// Numero del vecino derecho de i
#define PENSANDO 0						// El filosofo esta pensando
#define HAMBRIENTO 1					// El filosofo trata de obtener los tenedores
#define COMIENDO 2						// El filosofo esta comiendo

int estado[N];							// Arreglo que lleva el registro del estado de todos
int mutex;								// Exclusion mutua para las regiones criticas
int s[N];								// Un semaforo por filosofo

pid_t filos[N];							// Arreglo de identificadoes para cada filosofo
int k = -1;								// Identifica el numero de filosofo.

void pensar();
void comer();
void tomarTenedores(int i);
void ponerTenedores(int i);
void filosofo(int i);
void probar(int i);

int up(int semid);
int down(int semid);
int initsem(key_t semkey);
void filosofoSatisfecho();
void irse();
void finCena();
void iniciarSem();
void esperar();
void limpiarSem();

int main()
{
	int i;
	iniciarSem();
	signal(SIGINT, finCena);

	printf("\n\n");

	// Creamos N filosofos
	for(i = 0; i < N; i++)
	{
		if((filos[i] = fork()) < 0)
		{
			perror("\nError fork");
			exit(2);
		}

		else if(filos[i] == 0)
		{
			printf("** Filosofo entro a la mesa, pid = %d **\n", getpid(), getppid());
			signal(SIGINT, filosofoSatisfecho);
			signal(SIGUSR1, irse);
			k = i;
			break;
		}

		sleep(1);
	}

	// Entran los 5 filosofos
	if(k != -1)
		filosofo(k);

	// El padre espera a que los filosofos terminen su ejecucion
	else
	{
		for(i = 0; i < N; i++)
			wait(NULL);
	}

	limpiarSem();

	return 0;
}

void iniciarSem()
{
	int i;
	key_t key;

	key = ftok(".", 0);

	if(key == -1)
	{
		perror("\nError en ftok");
		exit(EXIT_FAILURE);
	}

	if((mutex = initsem(key)) == -1)
		exit(1);

	for(i = 0; i < N; i++)
	{
		key = ftok(".", (i+1));

		if(key == -1)
		{
			perror("\nError en ftok");
			exit(EXIT_FAILURE);
		}

		if((s[i] = initsem(key)) == -1)
			exit(1);
	}
}

void filosofo(int i)
{
	while(TRUE)
	{
		pensar();
		tomarTenedores(i);				// Adquiere dos tenedores o se bloquea
		comer();
		ponerTenedores(i);				// Pone de vuelta ambos tenedores en la mesa
	}
}

void pensar()
{
	printf("\n----- Filosofo %d Pensando... -----\n", (k+1));
	sleep(6);
}

void comer()
{
	printf("\nFilosofo %d Comiendo...\n", (k+1));
	sleep(2);
	printf("\nFilosofo %d termino de comer.\n", (k+1));
}

void tomarTenedores(int i)
{
	printf("\nFilosofo %d preparandose para comer\n\n", (k+1));
	down(mutex);						// Entra a la region critica
	estado[i] = HAMBRIENTO;				// Registra el hecho de que el filosofo i esta hambriento
	printf("Filosofo %d probando conseguir tenedores...\n", (k+1));
	probar(i);							// Trata de adquirir dos tenedores
	up(mutex);							// Sale de la region critica
	down(s[i]);							// Se bloquea si no se adquieren los tenedores
}

void ponerTenedores(int i)
{
	down(mutex);						// Entra a la region critica
	estado[i] = PENSANDO;				// El filosofo termino de comer
	printf("Filosofo %d termino de comer\n", (k+1));
	probar(IZQUIERDO(i));				// Verifica si el vecino izquierdo puede comer ahora
	probar(DERECHO(i));					// Verifica si el vecino derecho puede comer ahora
	up(mutex);							// Sale de la region critica
}

void probar(int i)
{
	if(estado[i] == HAMBRIENTO && estado[(IZQUIERDO(i))] != COMIENDO && estado[(DERECHO(i))] != COMIENDO)
	{
		estado[i] = COMIENDO;
		up(s[i]);
	}
}

void filosofoSatisfecho() { kill(getppid(), SIGINT); }

void irse()
{
	printf("\nFilosofo %d pid = %d saliendo de la mesa...\n", (k+1), getpid());
	exit(0);
}

void finCena()
{
	int i;
	printf("\n\n***** Filosofos terminando de comer *****\n\n");

	for(i = 0; i < N; i++)
		kill(filos[i], SIGUSR1);
}

void limpiarSem()
{
	int i;

	// Borra el semaforo mutex
	if(semctl(mutex, 0, IPC_RMID) == -1)
	{
		perror("\nError semctl\n");
		printf("\nSemaforo mutex, id = %d", mutex);
		exit(1);
	}

	// Borra los semaforos usados por cada filosofo
	for(i = 0; i < N; i++)
	{
		if(semctl(s[i], 0, IPC_RMID) == -1)
		{
			perror("\nError semctl\n");
			printf("\nSemaforo s[%d], id = %d", i, s[i]);
			exit(1);
		}
	}
}
