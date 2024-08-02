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
#include <bits/stdc++.h>
#include <time.h>
#include <cstring>

#define RECIBIR_RTA 0
#define SALTAR_RTA 1
#define FINALIZAR_JUEGO -2
#define ID_DATOS 12345
#define ID_DEF 23456
#define ID_RESP 34567
#define TAMBUF 1024
/***********************************
    Nombre del archivo: ej4_srv.cpp
    APL N°3, Ej N°4
    Corrales Mauro Exequiel, DNI: 40137650
    Primera Reentrega
***********************************/

using namespace std;
bool partida_en_curso = false;

//crear semaforos
sem_t* sem2 = sem_open("/sem2", O_CREAT | O_EXCL,0666,0);
sem_t* sem1 = sem_open("/sem1", O_CREAT | O_EXCL,0666,0);
sem_t* sem3 = sem_open("/sem3", O_CREAT | O_EXCL,0666,0);

struct comp
{
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

void cerrar_servidor(int n)
{
    if(partida_en_curso)
    {
        cout << "No se puede cerrar el servidor cuando hay una partida en curso." << endl;
        return;
    }
    cout << "Cerrando servidor." <<endl;
    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_unlink("/sem2");
    sem_unlink("/sem1");
    sem_unlink("/sem3");
    shmdt(&datos_compartidos);
    shmctl(shmid,IPC_RMID,NULL);
    shmdt(&def);
    shmctl(respid,IPC_RMID,NULL);
    shmdt(&resp);
    shmctl(defid,IPC_RMID,NULL);
    exit(0);
}
struct definicion
{
    string descripcion;
    string palabra;
    int letras;
};

bool cargar_definiciones(string path, vector <struct definicion> &datos)
{
    ifstream fp;
    fp.open(path);
    if (!fp.is_open())
    {
        cout << "No se pudo abrir el archivo "<<path<<endl;
        return false;
    }
    string linea;
    while(getline(fp,linea))
    {
        if(linea.empty())
            continue;
        struct definicion d;
        stringstream l(linea);
        getline(l,d.descripcion,'/');
        getline(l,d.palabra,'/');
        d.letras = d.palabra.size();
        datos.push_back(d);
    }
    fp.close();
    return true;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cout << "Debe proporcionar el path al archivo de definiciones." << endl;
        return 0;
    }
    string path(argv[1]);
    if (path == "-h" || path == "--help")
    {
        cout << "El servidor se ejecuta en 2do plano, recibe por parametro la ruta del archivo de definiciones." << endl;
        return 0;
    }

    //manejo de señales
    signal(SIGINT,SIG_IGN);//ignorar ctrl+c***/
    signal(SIGUSR1,cerrar_servidor);

    //verificar que solo haya 1 instancia.
    pid_t pid_act = getpid();
    ifstream pidfile;
    pidfile.open("/tmp/ej4s_pid");
    if (pidfile.is_open())
    {
        string linea;
        pid_t pid_archivo;
        getline(pidfile,linea);
        sscanf(linea.c_str(),"%d",&pid_archivo);
        if((pid_archivo != pid_act)&&(0 == kill(pid_archivo,0)))
        {
            cout << "El servidor ya se esta ejecutando" << endl;
            pidfile.close();
            return 0;
        }
        pidfile.close();
    }
    /*pid_t pid = fork();//pasar a 2do plano
    if (pid != 0)
        return 0;*/
    if(sem2 == NULL || sem1 == NULL || sem3 == NULL)//no se pudieron abrir los semaforos
    {
        /*** si el programa finalizo de forma incorrecta, los semaforos no se cierran correctamente ***/
        /*** destruir semaforos y volver a crearlos para ejecutar correctamente ***/
        sem_close(sem1);
        sem_close(sem2);
        sem_close(sem3);
        sem_unlink("/sem1");
        sem_unlink("/sem2");
        sem_unlink("/sem3");
        sem2 = sem_open("/sem2", O_CREAT | O_EXCL,0666,0);
        sem1 = sem_open("/sem1", O_CREAT | O_EXCL,0666,0);
        sem3 = sem_open("/sem3", O_CREAT | O_EXCL,0666,0);
        if(sem2 == NULL || sem1 == NULL || sem3 == NULL)//si aun asi fallo, el problema fue otro (nunca sucedio en mis testeos)
        {
            cout << "Error al crear los semaforos." << endl;
            exit(0);
        }
    }
    ofstream pidfile_w;
    pidfile_w.open("/tmp/ej4s_pid");
    pidfile_w << getpid();
    pidfile_w.close();

    //cargar las definiciones del archivo de texto.
    vector <struct definicion> definiciones;
    if(!cargar_definiciones(path,definiciones))
    {
        return -1;
    }
    /*
    //mostrar las definiciones cargadas.
    for (auto i = definiciones.begin();i!=definiciones.end();i++)
        cout << i->descripcion << " " << i->palabra << endl;
    cout << definiciones.size() << endl;
    //*/

    //inicializar vector de indices y generador de valores aleatorios
    vector <int> idx; //ordeno los indices de forma aleatoria, y luego accedo a los primeros N
    srand(time(NULL));//seed para los valores aleatorios, basada en el tiempo actual.
    for (size_t i= 0; i < definiciones.size(); i++)
        idx.push_back(i);//cargar valores
    vector<int> errores;
    bool finalizar;
    //iniciar juego
    while(true)
    {
        finalizar = false;
        partida_en_curso = false;//permitir SIGUSR1
        datos_compartidos->senal = 0;
        /** esperar al cliente (num palabras) **/
        cout << "Esperando a un cliente para iniciar una nueva partida."<<endl;
        sem_post(sem1);
        sem_wait(sem2);
        if (datos_compartidos->senal == FINALIZAR_JUEGO)
        {
            cout << "El cliente finalizo la partida." << endl;
            continue;
        }
        partida_en_curso = true;//ignorar SIGUSR1
        random_shuffle(idx.begin(),idx.end());//preparar indices aleatorios
        /** inicializar datos **/
        datos_compartidos->puntaje= 0;
        errores.clear();
        int n_p = datos_compartidos->num_palabras;
        cout << "Numero de palabras recibido: "<<n_p<<endl;
        for(int i = 0; i<n_p; i++)
        {
            def = strncpy(def,definiciones.at(idx.at(i)).descripcion.c_str(),TAMBUF);
            datos_compartidos->num_letras = definiciones.at(idx.at(i)).letras;
            cout << "Enviando definicion: " << def <<endl;
            cout << "Enviando n_letras: " << datos_compartidos->num_letras << endl;
            /**indicar al cliente que los datos estan listos**/
            sem_post(sem1);
            /**esperar datos del cliente(señal)**/
            cout << endl;
            cout << "Esperando al cliente." << endl;
            sem_wait(sem2);
            switch(datos_compartidos->senal)
            {
            case RECIBIR_RTA:
                cout << "Recibida respuesta: " << resp <<endl;
                if(strcmp(definiciones.at(idx.at(i)).palabra.c_str(),resp)==0)
                    datos_compartidos->puntaje++;
                else
                {
                    datos_compartidos->puntaje--;
                    errores.push_back(idx.at(i));
                }
                break;
            case SALTAR_RTA:
                cout << "Respuesta salteada."<< endl;
                errores.push_back(idx.at(i));
                break;
            case FINALIZAR_JUEGO:
                finalizar = true;
                break;
            }
            if (finalizar)
            {
                cout << "El cliente finalizo la partida."<<endl;
                break;
            }
            cout << endl;
            /*** avisar al cliente que estamos listos para la sig. ***/
            sem_post(sem1);

        }//for palabras
        if (finalizar)
            continue;
        //utilizo la variable señal para informar la cantidad de errores
        datos_compartidos->senal = errores.size();
        /**informar al cliente que puede leer la cant de errores y el puntaje**/
        cout << "Partida finalizada, Puntaje: "<< datos_compartidos->puntaje << " Errores: " <<errores.size() << endl;
        sem_post(sem3);
        for (size_t i = 0; i < errores.size(); i++)
        {
            def = strncpy(def,definiciones.at(errores.at(i)).descripcion.c_str(),TAMBUF);
            resp = strncpy(resp,definiciones.at(errores.at(i)).palabra.c_str(),TAMBUF);
            cout << def << " : " << resp << endl;
            /**esperar que cliente y servidor esten listos para continuar**/
            sem_post(sem1);
            sem_wait(sem2);

        }
        cout << endl;
    }
    return 0;
}
