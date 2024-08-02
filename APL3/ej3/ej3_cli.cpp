#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <signal.h>
#define TAMBUF 1024
using namespace std;
/***********************************
    Nombre del archivo: ej3_cli.cpp
    APL N°3, Ej N°3
    Corrales Mauro Exequiel, DNI: 40137650
    Primera entrega
***********************************/

int main()
{
    signal(SIGINT,SIG_IGN);
    pid_t pid_act = getpid();
    ifstream pidfile;
    pidfile.open("/tmp/ej3c_pid");
    if (pidfile.is_open()){
        string linea;
        pid_t pid_archivo;
        getline(pidfile,linea);
        sscanf(linea.c_str(),"%d",&pid_archivo);
        //cout << pid_archivo << endl;
        if((pid_archivo != pid_act)&&(0 == kill(pid_archivo,0)))//ver que el proceso exista y no seamos nosotros.
        {
            cout << "El cliente ya se esta ejecutando" << endl;
            pidfile.close();
            return 0;
        }
        pidfile.close();
    }
    ofstream pidfile_w;
    pidfile_w.open("/tmp/ej3c_pid");
    pidfile_w << getpid();
    pidfile_w.close();
    int srv = open("./fifoserver",O_RDONLY);
    int cli = open("./fifoclient",O_WRONLY);
    char msj[TAMBUF];
    string entrada;
    while(true){
        cout << "\nIngrese comando: ";
        fgets(msj,TAMBUF,stdin);
        for(size_t i = 0; i< sizeof(msj);i++){
            msj[i]=tolower(msj[i]);
            if(msj[i] == '\n')
                msj[i]='\0';
        }
        /*write(cli,&msj,sizeof(msj));*/
        entrada.assign(msj);
        if (entrada == "quit"){
            write(cli,&msj,sizeof(msj));
            break;
        }
        stringstream s(entrada);
        string comando;
        string chofer;
        s >> comando;
        bool ok = false;
        if (comando == "sum" || comando == "list"){
            s >> chofer;
            if (chofer != "\0"){
            ok = true;
            write(cli,comando.c_str(),TAMBUF);
            write(cli,chofer.c_str(),TAMBUF);
            read(srv,&msj,TAMBUF);
            cout << msj;}
        }
        if (comando == "max_gasoil" || comando == "avg_valor"){
            ok = true;
            write(cli,comando.c_str(),TAMBUF);
            read(srv,&msj,TAMBUF);
            cout << msj;
        }
        if (!ok){
            cout << "El comando ingresado es incorrecto."<< endl;
        }
    }
    close(srv);
    close(cli);
    return 0;
}
