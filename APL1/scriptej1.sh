#!/bin/bash

#*************************************************#
#Nombre del script:scriptej1.sh
#APL N°1
#Ej N°1
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

#respuestas a los puntos 1,6,7,8 al final del codigo.

funcA() { #ErrorParametrosIncorrectos
    echo "Error. La sintaxis del script es la siguiente:"
    echo "$0 DIRECTORIO CANTIDAD_A_MOSTRAR, por ejemplo: $0 directorio 5" # COMPLETAR
    #Esta funcion nunca se utiliza en el codigo.
    #Muestra un cartel de error por pantalla y un texto de ayuda.
    #Deberia invocarse luego de validar los parametros si alguno no es correcto.
    #Si se llego a este punto, deberia utilizar exit para terminar la ejecucion.
    #como eso no sucede, el script continua tratando de ejecutarse con parametros invalidos.
}

funcB() { #ErrorDirectorioInvalido
    echo "Error. '$1' no es un directorio valido." # COMPLETAR
    #Al igual que funcA, como la ejecucion no termina el script continua con param. invalidos.
}

funcC() { #VerificarDirectorio
    #si el 2do parametro no es un directorio llamar a funcB
    #todos los demas parametros que recibe nunca se utilizan.
    #deberia recibir un solo parametro y verificar el 1ero.
    #deberia pasarle el directorio como parametro a funcB para mostrarlo en el cartel de error.
    #$2 debe ir entre comillas para evitar problemas con directorios con espacios.
    if [[ ! -d $2 ]]; then
        funcB #"$2"
    fi
}
#Falta validar que solo recibo los 2 parametros necesarios.
#Falta validar que el segundo parametro sea un numero
#por ejemplo:
#if [[ $# != 2 || ! $2 =~ ^[0-9]+$ ]]; then
#funcA
#fi
#operador =~ evalua expresiones regulares.

#cada vez que referencio una variable que contiene un directorio deberia hacerlo entre ""
#nombres con espacios rompen este codigo.
funcC $# $1 $2 $3 $4 $5 #solo "$1" es necesario, pero la funcion revisa el 2do parametro que recibe
LIST=$(ls -d $1*/) 
#listar todos los directorios en la ruta donde se ejecuta el script...
#...cuyo nombre empieza con el 1er parametro y almacenarlos en la variable LIST, uno por linea.
#ej. "ca" listaria "ca/" pero tambien "casa/" "camion/" etc.
#si la intencion fuera ser exacto, habria que remover el *

ITEMS=()
for d in $LIST; do
    #Listar todos los elementos del directorio (1 por linea) y luego contar las lineas.
    #Al resultado (numero) se le concatena un guion y la ruta del directorio en cuestion.
    #Los items terminan teniendo este formato -> [num_elem]-[ruta_dir] 
    #luego almacenar el item en un array.
    ITEM="`ls $d | wc -l`-$d"
    ITEMS+=($ITEM)
done


IFS=$'\n' sorted=($(sort -rV -t '-' -k 1 <<<${ITEMS[*]}))
#Se cambia el separador de campos a el caracter de salto de linea
#${ITEMS[*]} retorna todos los elementos del array separados por el caracter IFS 
#por lo tanto muestro 1 elemento por linea, lo que me habilita a usar sort.
#sort ordena lineas, en este caso con los siguientes criterios:
#-r Mayor a menor
#-V orden natural de texto con numeros.
#vease: https://www.gnu.org/software/coreutils/manual/html_node/Version-sort-overview.html
#-t '-' separador de campos
#-k 1 ordenar segun el primer campo (en este caso, cantidad de elementos)

CANDIDATES="${sorted[*]:0:$2}" #tomar los primeros $2 elementos del array (tantos como indique el 2do param)
unset IFS #devolver IFS a su valor por defecto.
echo "Primeros $2 directorios ordenados de mayor a menor segun la cantidad de elementos que poseen:" # COMPLETAR
printf "%s\n" "$(cut -d '-' -f 2 <<<${CANDIDATES[*]})"
#cut toma parte de una linea segun estos criterios:
#-d '-' delimitador
#-f 2 segundo campo
#recordando el formato de los elementos: [num_elem]-[ruta_dir], estoy tomando la ruta.

#respuestas:
#1) El script recibe dos parametros, un directorio y un numero N
#   Como resultado, el script muestra por pantalla los primeros N directorios
#   de la carpeta donde se ejecuta cuya ruta comienza con el 1er parametro,
#   ordenados de mayor a menor segun la cantidad de elementos que contienen.

#6) $# almacena el numero de parametros que se le pasaron al script (o a una funcion)
#   otras variables:
#   $0 nombre del script
#   $1,$2,$3... parametros
#   $? valor de retorno del ultimo comando.
#   $$ PID del script

#7) Comillas:
#   "" -> cadenas de texto, con reemplazos de:
#       - variables ($)
#       - secuencias de escape (\)
#       - comandos (``,$())
#   '' -> literales, cadenas de texto a las cuales no se les realiza ningun reemplazo.
#   `` -> sustitucion de comandos, se ejecuta el texto entre `` como un comando del shell 
#         y se retorna la salida del mismo.

#8) Si no paso ningun parametro, el script reconoce que no le estoy pasando un directorio
#   y muestra el cartel de error correspondiente, pero no termina la ejecucion.
#   por lo tanto continua ejecutandose con parametros invalidos, lo que causa mensajes de error
#   del comando ls y una lista vacia de directorios ordenados al final.

#Corrales Mauro Exequiel, DNI:40137650