#include <iostream>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <sstream>
#include <cstring>

#define ENVIAR_RTA 0
#define SALTAR_RTA 1
#define FINALIZAR_JUEGO -2

#define ID_DATOS 12345
#define ID_DEF 23456
#define ID_RESP 34567
#define TAMBUF 1024
/***********************************
    Nombre del archivo: ej4_cli.cpp
    APL N°3, Ej N°4
    Corrales Mauro Exequiel, DNI: 40137650
    Primera Reentrega
***********************************/
using namespace std;

sem_t* sem1 = sem_open("/sem1", O_CREAT);
sem_t* sem2 = sem_open("/sem2", O_CREAT);
sem_t* sem3 = sem_open("/sem3", O_CREAT);

struct comp{
    int num_palabras;
    int num_letras;
    int senal;
    int puntaje;
};
//memoria compartida
int shmid =shmget(ID_DATOS,sizeof(struct comp),IPC_CREAT | 0666);
int defid =shmget(ID_DEF,TAMBUF,IPC_CREAT | 0666);
int respid =shmget(ID_RESP,TAMBUF,IPC_CREAT | 0666);
struct comp* datos_compartidos = (struct comp*)shmat(shmid,NULL,0);
char* def = (char*)shmat(defid,NULL,0);
char* resp = (char*)shmat(respid,NULL,0);


void cerrar_todo()
{
    sem_close(sem3);
    sem_close(sem2);
    sem_close(sem1);
    shmdt(&datos_compartidos);
    shmdt(&def);
    shmdt(&resp);
}
void salir(int n)
{
    datos_compartidos->senal = FINALIZAR_JUEGO;
    /**informar al server**/
    sem_post(sem2);
    cerrar_todo();
    exit(0);
}
int main()
{
    ifstream pidfile_s;
    pidfile_s.open("/tmp/ej4s_pid");
    if (pidfile_s.is_open())
    {
        string linea;
        pid_t pid_archivo_s;
        getline(pidfile_s,linea);
        sscanf(linea.c_str(),"%d",&pid_archivo_s);
        if(0 != kill(pid_archivo_s,0))
        {
            cout << "El servidor no esta ejecutandose." << endl;
            pidfile_s.close();
            return 0;
        }
        pidfile_s.close();
    }
    else{//el archivo de pid no existe
            cout << "El servidor no esta ejecutandose." << endl;
            pidfile_s.close();
            return 0;
    }
    pid_t pid_act = getpid();
    ifstream pidfile;
    pidfile.open("/tmp/ej4c_pid");
    if (pidfile.is_open())
    {
        string linea;
        pid_t pid_archivo;
        getline(pidfile,linea);
        sscanf(linea.c_str(),"%d",&pid_archivo);
        if((pid_archivo != pid_act)&&(0 == kill(pid_archivo,0)))
        {
            cout << "El cliente ya se esta ejecutando" << endl;
            pidfile.close();
            return 0;
        }
        pidfile.close();
    }
    ofstream pidfile_w;
    pidfile_w.open("/tmp/ej4c_pid");
    pidfile_w << getpid();
    pidfile_w.close();
    /** escribir en area compartida **/
    sem_wait(sem1);
    signal(SIGINT,salir);//habilitar Ctrl+C mientras el usuario ingresa datos.
    cout << "Ingrese el numero de palabras a adivinar: ";
    cin >> datos_compartidos->num_palabras;
    cin.get();//eliminar salto de linea del buffer de entrada para usar getline mas adelante
    cout << endl;
    /**informar al servidor que estamos listos**/
        sem_post(sem2);
    for(int i = 0; i < datos_compartidos->num_palabras; i++)
    {
        /**esperar datos del servidor (definicion y n_letras)**/
        sem_wait(sem1);
        cout << def << ": (" << datos_compartidos->num_letras <<" letras)" << endl;
        cout << "Respuesta: ";
        string re;
        getline(cin,re);
        cout << endl;
        resp = strncpy(resp,re.c_str(),TAMBUF);
        signal(SIGINT,SIG_IGN);//deshabilitar Ctrl+C durante la comunicacion con el servidor.
        if(re=="")
            datos_compartidos->senal = SALTAR_RTA;
        else
            datos_compartidos->senal = ENVIAR_RTA;
        /*** enviar respuesta ***/
        sem_post(sem2);
        /*** esperar al servidor para la prox. pregunta ***/
        sem_wait(sem1);

    }
    /** esperar puntuacion final y numero de errores **/
    sem_wait(sem3);//utilizar un tercer semaforo solo para indicar el fin de la partida facilito la sincronizacion.
    cout << "Puntuacion final: " << datos_compartidos->puntaje << endl;
    if(datos_compartidos->senal > 0)
    {
        cout << "Palabras incorrectas y/o no respondidas:" << endl;
        for (int i = 0; i < datos_compartidos->senal; i++)
        {
            /*** esperar que el servidor cargue los datos ***/
            sem_wait(sem1);
            cout << def << ": " << resp << endl;
            /*** informar que estamos listos ***/
            sem_post(sem2);
        }
    }
    cerrar_todo();
    return 0;
}
