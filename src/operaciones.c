#include "filosofos.h"

int down(int semid)
{
	struct sembuf p_buf;

	p_buf.sem_num = 0;
	p_buf.sem_op = -1;
	p_buf.sem_flg = SEM_UNDO;

	if(semop(semid, &p_buf, 1) == -1)
	{
		perror("down failed");
		exit(1);
	}

	return 0;
}

int up(int semid)
{
	struct sembuf v_buf;

	v_buf.sem_num = 0;
	v_buf.sem_op = 1;
	v_buf.sem_flg = SEM_UNDO;

	if(semop(semid, &v_buf, 1) == -1)
	{
		perror("up failed");
		exit(1);
	}

	return 0;
}

// Inicializa un semaforo
int initsem(key_t semkey)
{
	int status = 0, semid;

	// Intenta crear un semaforo
	if((semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
	{
		if(errno == EEXIST)
			semid = semget(semkey, 1, 0);
	}

	// Si ya existe
	else
		status = semctl(semid, 0, SETVAL, 1);

	if(semid == -1 || status == -1)
	{
		perror("\ninitsem failed\n");
		return -1;
	}

	else
		return semid;
}
