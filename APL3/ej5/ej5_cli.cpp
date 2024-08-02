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
#include <arpa/inet.h>
#include <netinet/in.h>

#define ENVIAR_RTA 0
#define SALTAR_RTA 1
#define FINALIZAR_JUEGO -2

/***********************************
    Nombre del archivo: ej5_cli.cpp
    APL N°3, Ej N°5
    Corrales Mauro Exequiel, DNI: 40137650
    Primera entrega
***********************************/

using namespace std;

int x;
struct comp{
    int num_palabras;
    int num_letras;
    int senal;
    int puntaje;
    string def;
    string resp;
};

void cerrar_todo()
{
    close(x);
}

struct comp datos_compartidos;
void salir(int n)
{
    char buffer[256];
    sprintf(buffer,"%d",FINALIZAR_JUEGO);
    send(x,buffer,sizeof(buffer),0);
    cerrar_todo();
    exit(0);
}
int main(int argc, char** argv)
{
    signal(SIGINT,SIG_IGN);
    //Espera al menos dos parámetros: IP y Puerto de escucha
	if( argc < 3 ){
		cout << "Debe ingresar IP del servidor y puerto de escucha"<<endl;
		return 1;
	}
    pid_t pid_act = getpid();
    ifstream pidfile;
    pidfile.open("/tmp/ej5c_pid");
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
    pidfile_w.open("/tmp/ej5c_pid");
    pidfile_w << getpid();
    pidfile_w.close();

    char buffer[256];
	struct sockaddr_in sa; //Configuración del servidor
	//Crear el socket del cliente
	if((x=socket(AF_INET,SOCK_STREAM,0))==-1){
		cout<<"No se pudo crear el socket"<<endl;
		return 1;
	}
	bzero((char *) &sa, sizeof(struct sockaddr_in));
 	sa.sin_family		=	AF_INET;
 	sa.sin_port			=	htons(atoi(argv[2]));
	sa.sin_addr.s_addr=	inet_addr(argv[1]);
    //Solicitud de conexión
	if(connect(x,(struct sockaddr *) &sa,sizeof(sa))==-1){
		cout<<"Solicitud rechazada"<<endl;
		return 1;
	}
    signal(SIGINT,salir);//habilitar Ctrl+C mientras el usuario ingresa datos.
    datos_compartidos.num_palabras=0;
    while(datos_compartidos.num_palabras<1){
    cout << "Ingrese el numero de palabras a adivinar: ";
    cin >> datos_compartidos.num_palabras;
    cout << endl;}
    sprintf(buffer,"%d",datos_compartidos.num_palabras);
    send(x,buffer,sizeof(buffer),0);
    for(int i = 0; i < datos_compartidos.num_palabras; i++)
    {
        /**esperar datos del servidor (definicion y n_letras)**/
        bzero(buffer,256);
        recv(x,buffer,256,0);
        datos_compartidos.def.assign(buffer);
        //cout << datos_compartidos.def << endl;
        send(x,buffer,1,0);//ack
        bzero(buffer,256);
        recv(x,buffer,256,0);
        //cout << buffer << endl;
        datos_compartidos.num_letras=atoi(buffer);

        cout << datos_compartidos.def << ": (" << datos_compartidos.num_letras <<" letras)" << endl;
        cout << "Respuesta: ";
        cin >> datos_compartidos.resp;
        cout << endl;
        signal(SIGINT,SIG_IGN);//deshabilitar Ctrl+C durante la comunicacion con el servidor.
        if((datos_compartidos.resp)== "")
            datos_compartidos.senal = SALTAR_RTA;
        else
            datos_compartidos.senal = ENVIAR_RTA;
        sprintf(buffer,"%d",datos_compartidos.senal);
        send(x,buffer,sizeof(buffer),0);
        recv(x,buffer,1,0);//ack
        send(x,datos_compartidos.resp.c_str(),datos_compartidos.resp.size(),0);
    }
    /** esperar puntuacion final y numero de errores **/
    bzero(buffer,256);
    recv(x,buffer,256,0);
    datos_compartidos.senal=atoi(buffer);
    send(x,buffer,1,0);//ack
    bzero(buffer,256);
    recv(x,buffer,256,0);
    datos_compartidos.puntaje=atoi(buffer);
    cout << "Puntuacion final: " << datos_compartidos.puntaje << endl;
    if(datos_compartidos.senal > 0)
    {
        cout << "Palabras incorrectas y/o no respondidas:" << endl;
        for (int i = 0; i < datos_compartidos.senal; i++)
        {
            bzero(buffer,256);
            recv(x,buffer,256,0);
            datos_compartidos.def.assign(buffer);
            send(x,buffer,1,0);//ack
            bzero(buffer,256);
            recv(x,buffer,256,0);
            datos_compartidos.resp.assign(buffer);
            cout << datos_compartidos.def << ": " << datos_compartidos.resp << endl;
            send(x,buffer,1,0);//ack
        }
    }
    cerrar_todo();
    return 0;
}
