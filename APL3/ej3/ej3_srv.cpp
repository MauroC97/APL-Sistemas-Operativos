#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#define TAMBUF 1024

/***********************************
    Nombre del archivo: ej3_srv.cpp
    APL N°3, Ej N°2
    Corrales Mauro Exequiel, DNI: 40137650
    Primera entrega
***********************************/
using namespace std;
struct viaje{
    int id;
    string chofer;
    string origen_destino;
    int kms;
    int gasoil;
    int valor;
};

string SUM_CHOFER(string objetivo,vector<struct viaje>& viajes){
    int tot_kms = 0;
    int tot_gas = 0;
    int tot_val = 0;
    for (struct viaje v : viajes){
        if (objetivo == v.chofer){
            tot_gas+=v.gasoil;
            tot_kms+=v.kms;
            tot_val+=v.valor;
        }
    }
    if(tot_kms == 0 && tot_gas == 0 && tot_val == 0)
        return "No se encontraron viajes para el chofer "+objetivo+"\n";
    return "Km: "+to_string(tot_kms)+"\nGasoil: "+to_string(tot_gas)+" Litros\nValor: $ "+to_string(tot_val)+"\n";
}

string MAX_GASOIL(vector<struct viaje>& viajes){
    int MAXG = 0;
    struct viaje MAXV;
    for (struct viaje v : viajes){
        if (v.gasoil>MAXG){
            MAXV=v;
            MAXG=MAXV.gasoil;
        }
    }
    return "ID: "+to_string(MAXV.id)+"\nChofer: "+MAXV.chofer+"\nTrayecto: "+MAXV.origen_destino+" ("+to_string(MAXV.kms)+" Km)\nGasoil: "+to_string(MAXV.gasoil)+" Lt\n";
}

string AVG_VALOR(vector<struct viaje>& viajes){
    double AVG=0;
    for (struct viaje v : viajes){
        AVG+=v.valor;
    }
    return "$ "+to_string(AVG/viajes.size())+"\n";
}

string LIST_CHOFER(string objetivo,vector<struct viaje>& viajes){
    string listado = "";
    for (struct viaje v : viajes){
        if (objetivo==v.chofer){
            //printf("%d\t%s (%d Km)\t%d Lt\t$ %d\n",v.id,v.origen_destino.c_str(),v.kms,v.gasoil,v.valor);
            listado+=(""+to_string(v.id)+"\t"+v.origen_destino+" ("+to_string(v.kms)+" Km)\t"+to_string(v.gasoil)+" Lt\t$ "+to_string(v.valor)+"\n");
        }
    }
    if (listado == "")
        return "No se encontraron viajes para el chofer "+objetivo+"\n";
    return listado;
}
bool procesar_viajes(string path,vector<struct viaje>& datos){

    ifstream fp;
    fp.open(path);
    if (!fp.is_open()){
    cout << "No se pudo abrir el archivo "<<path<<endl;
    return false;
    }
    string linea;
    int var_cargadas;
    while(getline(fp,linea)){
        struct viaje v;
        char chof[100];
        char org[100];
        var_cargadas = sscanf(linea.c_str(),"%d\t%s\t%s\t%d\t%d\t%d\n",&v.id,chof,org,&v.kms,&v.gasoil,&v.valor);
        for(size_t i = 0; i< sizeof(chof);i++){
            chof[i]=tolower(chof[i]);
            }
        if (var_cargadas==6){
            v.chofer.assign(chof);
            v.origen_destino.assign(org);
            datos.push_back(v);
        }

    }
    return true;
}

int main(int argc, char** argv)
{
    pid_t pid_act = getpid();
    ifstream pidfile;
    pidfile.open("/tmp/ej3s_pid");
    if (pidfile.is_open()){
        string linea;
        pid_t pid_archivo;
        getline(pidfile,linea);
        sscanf(linea.c_str(),"%d",&pid_archivo);
        //cout << pid_archivo << endl;
        if((pid_archivo != pid_act)&&(0 == kill(pid_archivo,0)))//ver que el proceso exista y no seamos nosotros.
        {
            cout << "El servidor ya se esta ejecutando" << endl;
            pidfile.close();
            return 0;
        }
        pidfile.close();
    }

    if (argc != 2){
    cout << "Debe proporcionar el path al archivo de viajes." << endl;
    return 0;
    }
    string path(argv[1]);
    if (path == "-h" || path == "--help"){
        cout << "El servidor se ejecuta en 2do plano, recibe por parametro la ruta del archivo de viajes." << endl;
        return 0;
    }
    signal(SIGINT,SIG_IGN);
    vector<struct viaje> datos;
    if(!procesar_viajes(path,datos))
        return -1;
    cout << "Pasando a segundo plano." << endl;
    pid_t pid = fork();
    if (pid != 0)
        return 0;

    ofstream pidfile_w;
    pidfile_w.open("/tmp/ej3s_pid");
    pidfile_w << getpid();
    pidfile_w.close();

    mkfifo("./fifoserver",0666);
    mkfifo("./fifoclient",0666);
    int srv = open("./fifoserver",O_WRONLY);
    int cli = open("./fifoclient",O_RDONLY);
    string comando;
    string chofer;
    char msj[TAMBUF];
    while(true){
        read(cli,&msj,TAMBUF);
        comando.assign(msj);
        if(comando == "quit")
            break;
        if(comando == "list"){
            read(cli,&msj,TAMBUF);
            chofer.assign(msj);
            write(srv,LIST_CHOFER(chofer,datos).c_str(),TAMBUF);
        }
        if(comando == "sum"){
            read(cli,&msj,TAMBUF);
            chofer.assign(msj);
            write(srv,SUM_CHOFER(chofer,datos).c_str(),TAMBUF);
        }
        if(comando == "avg_valor"){
            write(srv,AVG_VALOR(datos).c_str(),TAMBUF);
        }
        if(comando == "max_gasoil"){
            write(srv,MAX_GASOIL(datos).c_str(),TAMBUF);
        }
    }
    cout << "Cerrando servidor" << endl;
    close(srv);
    close(cli);
    unlink("./fifoserver");
    unlink("./fifoclient");
    return 0;
}
