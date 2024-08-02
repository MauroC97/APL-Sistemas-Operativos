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
#include <netdb.h>
#include <sys/socket.h>

#define RECIBIR_RTA 0
#define SALTAR_RTA 1
#define FINALIZAR_JUEGO -2
#define ID_DATOS 12345
#define TAMBUF 1024
/***********************************
    Nombre del archivo: ej5_srv.cpp
    APL N°3, Ej N°5
    Corrales Mauro Exequiel, DNI: 40137650
    Primera entrega
***********************************/
using namespace std;
bool partida_en_curso = false;
int server_socket;
int client_socket;

struct comp
{
    int num_palabras;
    int num_letras;
    int senal;
    int puntaje;
    string def;
    string resp;
};

void cerrar_servidor(int n)
{
    if(partida_en_curso)
    {
        cout << "No se puede cerrar el servidor cuando hay una partida en curso." << endl;
        return;
    }
    cout << "Cerrando servidor." <<endl;
    close(client_socket);
    close(server_socket);

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
    if (argc != 3)
    {
        cout << "Parametros Incorrectos." << endl;
        return 0;
    }
    string path(argv[1]);
    if (path == "-h" || path == "--help")
    {
        cout << "El servidor se ejecuta en 2do plano, recibe por parametro la ruta del archivo de definiciones y el puerto." << endl;
        return 0;
    }
    string puerto = argv[2];

    //manejo de señales
    signal(SIGINT,SIG_IGN);//ignorar ctrl+c
    signal(SIGUSR1,cerrar_servidor);

    //verificar que solo haya 1 instancia.
    pid_t pid_act = getpid();
    ifstream pidfile;
    pidfile.open("/tmp/ej5s_pid");
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

    ofstream pidfile_w;
    pidfile_w.open("/tmp/ej5s_pid");
    pidfile_w << getpid();
    pidfile_w.close();

    //cargar las definiciones del archivo de texto.
    vector <struct definicion> definiciones;
    if(!cargar_definiciones(path,definiciones))
    {
        return -1;
    }
    //inicializar vector de indices y generador de valores aleatorios
    vector <int> idx; //ordeno los indices de forma aleatoria, y luego accedo a los primeros N
    srand(time(NULL));//seed para los valores aleatorios, basada en el tiempo actual.
    for (size_t i= 0; i < definiciones.size(); i++)
        idx.push_back(i);//cargar valores
    vector<int> errores;
    bool finalizar;

    //socket
    socklen_t cl=sizeof(struct sockaddr_in);
    struct sockaddr_in sa;
    struct sockaddr_in ca;
    char buffer[256];
    int habilitar = 1;
    if((server_socket=socket(AF_INET,SOCK_STREAM,0)) ==-1)
    {
        cout<<"No se pudo crear el socket."<<endl;
        return 1;
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &habilitar, sizeof(int)) < 0)
    {
        cout<<"No se pudo configurar el socket."<<endl;
        return 1;
    }
    //Configuración del servidor
    bzero((char *) &sa, sizeof(struct sockaddr_in));
    sa.sin_family 		= AF_INET;
    sa.sin_port 		= htons(atoi(puerto.c_str()));
    sa.sin_addr.s_addr= INADDR_ANY;

    //Vincular socket con configuracion del servidor
    bind(server_socket,(struct sockaddr *)&sa,sizeof(struct sockaddr_in));

    //Marcar el socket como "Socket Pasivo" y Establecer la máxima
    //cantidad de peticiones que pueden ser encoladas
    listen(server_socket,5);
    //El servidor ser bloquea a la espera de una peticion de conexión
    //desde el cliente (connect)
    client_socket=accept(server_socket,(struct sockaddr *) &ca,&cl);
    cout << "Cliente conectado."<< endl;
    //iniciar juego
    while(true)
    {
        finalizar = false;
        partida_en_curso = false;//permitir SIGUSR1
        struct comp datos_compartidos;
        /** esperar al cliente (num palabras) **/
        bzero(buffer,256);
        recv(client_socket,buffer,256,0);
        datos_compartidos.num_palabras=atoi(buffer);
        if(datos_compartidos.num_palabras == FINALIZAR_JUEGO)
        {
            cout << "Partida finalizada por el cliente." << endl;
            close(client_socket);
            client_socket=accept(server_socket,(struct sockaddr *) &ca,&cl);
            cout << "Cliente conectado."<< endl;
            continue;
        }

        cout << "Num Palabras: "<< datos_compartidos.num_palabras << endl;
        partida_en_curso = true;//ignorar SIGUSR1
        random_shuffle(idx.begin(),idx.end());//preparar indices aleatorios
        datos_compartidos.puntaje= 0;
        errores.clear();
        for(int i = 0; i<datos_compartidos.num_palabras; i++)
        {
            datos_compartidos.def = definiciones.at(idx.at(i)).descripcion;
            datos_compartidos.num_letras = definiciones.at(idx.at(i)).letras;
            cout << "Definicion: "<<datos_compartidos.def<< endl;
            cout << "Num. Letras: "<< datos_compartidos.num_letras<< endl;
            /**indicar al cliente que los datos estan listos**/
            bzero(buffer,256);
            send(client_socket,datos_compartidos.def.c_str(),datos_compartidos.def.size(),0);
            recv(client_socket,buffer,1,0);//ack
            bzero(buffer,256);
            sprintf(buffer,"%d",datos_compartidos.num_letras);
            send(client_socket,buffer,sizeof(buffer),0);
            /**esperar datos del cliente(señal)**/
            bzero(buffer,256);
            recv(client_socket,buffer,256,0);
            datos_compartidos.senal=atoi(buffer);
            send(client_socket,buffer,1,0);//ack
            bzero(buffer,256);
            recv(client_socket,buffer,256,0);
            datos_compartidos.resp.assign(buffer);
            switch(datos_compartidos.senal)
            {
            case RECIBIR_RTA:
                cout << "Recibida respuesta: " << datos_compartidos.resp << endl;
                if(definiciones.at(idx.at(i)).palabra == datos_compartidos.resp)
                    datos_compartidos.puntaje++;
                else
                {
                    datos_compartidos.puntaje--;
                    errores.push_back(idx.at(i));
                }
                break;
            case SALTAR_RTA:
                cout << "Respuesta salteada." << endl;
                errores.push_back(idx.at(i));
                break;
            case FINALIZAR_JUEGO:
                cout << "Partida finalizada por el cliente." << endl;
                finalizar = true;
                break;
            }
            if (finalizar)
            {
                close(client_socket);
                client_socket=accept(server_socket,(struct sockaddr *) &ca,&cl);
                cout << "Cliente conectado."<< endl;
                continue;
            }

        }//for palabras
        //utilizo la variables señal para informar la cantidad de errores
        datos_compartidos.senal = errores.size();
        cout << "Errores: " << datos_compartidos.senal <<endl;
        /**informar al cliente que puede leer la cant de errores y el puntaje**/
        sprintf(buffer,"%d",datos_compartidos.senal);
        send(client_socket,buffer,sizeof(buffer),0);
        recv(client_socket,buffer,1,0);//ack
        sprintf(buffer,"%d",datos_compartidos.puntaje);
        send(client_socket,buffer,sizeof(buffer),0);
        for (size_t i = 0; i < errores.size(); i++)
        {
            datos_compartidos.def =definiciones.at(errores.at(i)).descripcion;
            datos_compartidos.resp =definiciones.at(errores.at(i)).palabra;
            send(client_socket,datos_compartidos.def.c_str(),datos_compartidos.def.size(),0);
            recv(client_socket,buffer,1,0);//ack
            send(client_socket,datos_compartidos.resp.c_str(),datos_compartidos.resp.size(),0);
            recv(client_socket,buffer,1,0);//ack

        }
        close(client_socket);
        partida_en_curso = false;
        client_socket=accept(server_socket,(struct sockaddr *) &ca,&cl);
        cout << "Cliente conectado."<< endl;
    }
    return 0;
}
