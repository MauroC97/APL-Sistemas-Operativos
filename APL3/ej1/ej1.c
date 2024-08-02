#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

/***********************************
    Nombre del archivo: ej1.c
    APL N°3, Ej N°1
    Corrales Mauro Exequiel, DNI: 40137650
    Primera entrega

    Orden de creacion de los procesos: 1 2 5 9 6 7 10 11 3 8
***********************************/

int main(int argc, char *argv[])
{
    if (argc == 2){
        if (strcmp("--help",argv[1])==0 || strcmp("-h",argv[1])==0){
            printf("El programa crea un arbol de procesos siguiendo el grafo planteado en el ejercicio.");
            printf("\nOrden de creacion de los procesos: 1 2 5 9 6 7 10 11 3 8");
            printf("\nEjecute el programa sin parametros para comenzar.\n");
            return 0;
        }
        else{
            printf("Parametros incorrectos.");
            return 1;
        }
    }
    if (argc >2){
            printf("Parametros incorrectos.");
            return 1;
        }
    int padres[] = {getpid(),0,0};

    //1
    printf("Soy el PID %d, Padres -\n",padres[0]);
    pid_t hijo = fork();
    if (hijo == -1)
    {
        printf("Error al crear el proceso.");
        return 1;
    }
    if (hijo == 0)
    {
        //2
        padres[1] = getpid();
        printf("Soy el PID %d, Padres %d\n",getpid(),padres[0]);
        hijo = fork();
        if (hijo == -1)
        {
            printf("Error al crear el proceso.");
            return 1;
        }
        if (hijo == 0)
        {
            //5
            padres[2] = getpid();
            printf("Soy el PID %d, Padres %d %d\n",getpid(),padres[0],padres[1]);
            hijo = fork();
            if (hijo == -1)
            {
                printf("Error al crear el proceso.");
                return 1;
            }
            if (hijo == 0)
            {
                //9
                printf("Soy el PID %d, Padres %d %d %d\n",getpid(),padres[0],padres[1],padres[2]);
                return 0;
            }
            waitpid(hijo,NULL,0);//9
        }
        else
        {
            waitpid(hijo,NULL,0);//5
            //6
            hijo = fork();
            if (hijo == -1)
            {
                printf("Error al crear el proceso.");
                return 1;
            }
            if (hijo == 0)
            {
                printf("Soy el PID %d, Padres %d %d\n",getpid(),padres[0],padres[1]);
                return 0;
            }
            waitpid(hijo,NULL,0);//6
            //7
            hijo = fork();
            if (hijo == -1)
            {
                printf("Error al crear el proceso.");
                return 1;
            }
            if (hijo == 0)
            {
                padres[2] = getpid();
                printf("Soy el PID %d, Padres %d %d\n",getpid(),padres[0],padres[1]);
                //10
                hijo = fork();
                if (hijo == -1)
                {
                    printf("Error al crear el proceso.");
                    return 1;
                }
                if (hijo == 0)
                {
                    printf("Soy el PID %d, Padres %d %d %d\n",getpid(),padres[0],padres[1],padres[2]);
                    return 0;

                }
                waitpid(hijo,NULL,0);//10
                //11
                hijo = fork();
                if (hijo == -1)
                {
                    printf("Error al crear el proceso.");
                    return 1;
                }
                if (hijo == 0)
                {
                    printf("Soy el PID %d, Padres %d %d %d\n",getpid(),padres[0],padres[1],padres[2]);
                    return 0;

                }
                waitpid(hijo,NULL,0);//11

            }
            waitpid(hijo,NULL,0);//7
        }

    }
    else{
    waitpid(hijo,NULL,0);//2
    //3
    hijo = fork();
    if (hijo == -1)
    {
        printf("Error al crear el proceso.");
        return 1;
    }
    if (hijo == 0)
    {
        padres[1] = getpid();
        printf("Soy el PID %d, Padres %d\n",getpid(),padres[0]);
        //8
        hijo = fork();
        if (hijo == -1)
        {
            printf("Error al crear el proceso.");
            return 1;
        }
        if (hijo == 0)
        {
            padres[2] = getpid();
            printf("Soy el PID %d, Padres %d %d\n",getpid(),padres[0],padres[1]);
            return 0;
        }
        waitpid(hijo,NULL,0);//8
    }
    waitpid(hijo,NULL,0);//3
    }
    return 0;
}
//Corrales Mauro Exequiel DNI:40137650
