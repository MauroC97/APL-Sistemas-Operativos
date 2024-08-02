#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>

/***********************************
    Nombre del archivo: ej2.c
    APL N°3, Ej N°2
    Corrales Mauro Exequiel, DNI: 40137650
    Primera Reentrega
***********************************/


char *siguiente_archivo = NULL;
sem_t mostrar_por_pantalla;

//retorna 1 si es un archivo .txt, 0 si no lo es.
int es_txt(struct dirent *d)
{
    if (d->d_type != DT_REG)//chequear que sea un archivo
        return 0;
    char* str = d->d_name;
    return strcmp(strrchr(str,'.'),".txt")==0 ? 1:0;//mirar la extension
}
//contar cantidad de numeros de una cadena en un array
void contar(char* txt, int* contador)
{
    //int total = 0;
    for(int i = 0; i<strlen(txt); i++)
    {
        if(txt[i] >= '0' && txt[i] <= '9')
        {
            int num = txt[i]-'0';//ascii a int
            (*(contador+num))++;//el indice se corresponde con la cantidad de ocurrencias de ese numero.
            //total++;
        }
    }
    //return total;
}
void mostrar(int* contador)
{
    for (int i = 0; i < 10; i++)
        printf("[%d]=%d ",i,(*(contador+i)));
    printf("\n");
}
//acumular los valores del arr2 en arr1
void acumular(int* arr1, int* arr2, int tam)
{
    for (int i = 0; i < tam; i++)
        (*(arr1+i))+=(*(arr2+i));
}

struct param
{
    char** path;
    int cant_archivos;
    int* contador_total;

};
//abrir el archivo, leerlo, contar los valores, cargar el contador.
int procesar_txt(char* path, int* contador)
{
    int tam = 100;//leo como maximo 100 caracteres por vez o hasta encontrar un '\n' o '\0'
    char buffer[tam];
    FILE* fp = fopen(path,"r");
    if (fp == NULL)
    {
        printf("No se pudo abrir el archivo %s\n",path);
        return 0;

    }
    while(fgets(buffer,tam,fp)!=NULL)
        contar(buffer,contador);
    fclose(fp);
    return 1;
}
//funcion que ejecutan los threads
void* thread_txt(void* p)
{
    struct param *ptr = (struct param*) p;
    char** path = ptr->path;
    int* contador_total = ptr->contador_total;
    int tam= ptr->cant_archivos;
    int i = 0;
    //printf("THREAD:%lu,PATH:%s\n",pthread_self(),path);
    while(i<tam)
    {
        int contador[] = {0,0,0,0,0,0,0,0,0,0};
        if(!procesar_txt(*(path+i),contador))
        {
            pthread_exit(0);
        }
        i++;
        sem_wait(&mostrar_por_pantalla);
        printf("THREAD %lu, Archivo %s procesado.\n",pthread_self(),*(path+(i-1)));
        mostrar(contador);
        acumular(contador_total,contador,10);
        sem_post(&mostrar_por_pantalla);
    }//fin while
    printf("THREAD %lu: TERMINADO.\n",pthread_self());
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 3 || argc == 1)
    {
        printf("Los parametros ingresados son incorrectos.\n");
        return -1;
    }
    if (argc == 2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0))
    {
        printf("Ingrese como primer parametro el numero de threads a ejecutar\ny como 2do parametro la ruta donde estan los archivos.\n");
        return 0;
    }
    int n_threads = atoi(argv[1]);
    if (n_threads == 0)
    {
        printf("El numero de threads ingresado es invalido.\n");
        return -1;
    }
    char* path = argv[2];
    if (path[strlen(path)-1]!='/')
        strcat(path,"/");
    DIR* dp = opendir(path);
    if (dp == NULL)
    {
        printf("No se pudo abrir el directorio.\n");
        return -1;
    }
    sem_init(&mostrar_por_pantalla,0,1);
    int total[] = {0,0,0,0,0,0,0,0,0,0};
    struct dirent *d = NULL;
    pthread_t threads[n_threads];
    int t_creados = 0;
    int total_txts = 0;
    //contar cantidad de archivos a procesar
    while((d =readdir(dp)) != NULL)
    {
        if(es_txt(d))
            total_txts++;
    }
    rewinddir(dp);
    if (total_txts < n_threads)//no crear mas threads de los necesarios.
        n_threads = total_txts;
    int cant_t = (int)(total_txts/n_threads);//cant paths por thread
    int extra = total_txts%n_threads;// numero de archivos sobrantes a repartir
    for (t_creados = 0 ; t_creados<n_threads; t_creados++)
    {
        struct param* p = (struct param*)malloc(sizeof(struct param));
        p->contador_total=total;
        int tot = cant_t;
        if (extra > 0)
        {
            extra--;
            tot++;
        }
        p->cant_archivos=tot;
        char** paths = (char**)malloc(tot*sizeof(char*));
        while(tot>0 && ((d =readdir(dp)) != NULL))
        {
            if(es_txt(d))
            {
                tot--;
                char* nom_archivo = d->d_name;
                int tam_path = strlen(path)+strlen(nom_archivo);
                char* temp = (char*)malloc(tam_path);
                strcpy(temp,path);
                strcat(temp,nom_archivo);
                paths[tot] = temp;

            }
        }
        p->path= paths;
        pthread_create(&threads[t_creados],NULL,&thread_txt,p);
    }

    printf("MAIN: Esperando que los threads finalizen.\n");
    for (int i = 0; i<t_creados; i++)
    {
        pthread_join(threads[i],NULL);
    }
    closedir(dp);
    sem_close(&mostrar_por_pantalla);
    printf("\nFinalizando lectura.\nTotales:\n");
    mostrar(total);
    return 0;
}
//Corrales Mauro Exequiel DNI:40137650
