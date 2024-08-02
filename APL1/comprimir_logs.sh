#!/bin/bash

#*************************************************#
#Nombre del script:comprimir_logs.sh
#APL N°1
#Ej N°5
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

AYUDA="Utilize -h, -? o --help para obtener ayuda."

function verificarDirectorio() {
#-d verifica si el parametro existe y es un directorio
if [[ ! -d $1 ]]; then 
    echo "'"$1"' no es un directorio valido."
    echo "$AYUDA"
    exit 1
fi
}
#ver que haya un parametro
if [[ $# != 1 ]]; then
    echo "$AYUDA"
    exit 1
fi
#chequear por el param de ayuda
if [[ $# == 1 && ($1 == "-h" || $1 == "-?" || $1 == "--help") ]]; then
    echo "Sintaxis: "$0" ARCHIVO_CONFIG" 
    echo "El unico parametro es la ruta a un archivo de configuracion."
    echo "El mismo tiene el siguiente formato:
Primera linea: Carpeta de destino de los archivos comprimidos.
Lineas siguientes: Rutas de los logs a analizar."
    echo "Si alguna de las rutas en el archivo de configuracion es invalida, se informa por pantalla y el proceso se cancela."
    exit 0
fi

#verificar que el parametro es un archivo 
if [[ ! -f $1 ]]; then
    echo "El parametro no es un archivo de configuracion."
    echo "$AYUDA"
    exit 1
fi

#tiene que tener al menos dos lineas 
if [[ `wc -l < "$1"` < 2 ]]; then
    echo "El archivo de configuracion no es correcto o esta incompleto."
    echo "$AYUDA"
    exit 1
fi

#verificar directorios del archivo de configuracion y cargarlos en un array
IFS=$'\n'
DIRECTORIOS=()
for d in `cat $1`; do
    verificarDirectorio $d
    DIRECTORIOS+=(`realpath "$d"`)
done

#comprimirLogs()
#1er param:ruta logs
#2do param:ruta destino zip

#obtener nombre del directorio padre:
# \/[^/]+$ elimina el ultimo directorio de la cadena (donde estan los logs)
#luego elimino todos los directorios anteriores con \/.+\/
#ej: /tmp/servicios/busqueda/logs
#primer patron elimina /tmp/servicios/busqueda[/logs]
#segundo patron elimina [/tmp/servicios/]busqueda
#resultado: "busqueda"

#find . -daystart -mtime +0
#lista todos los archivos modificados antes de hoy no inclusive.
#archivos creados 1 segundo antes del comienzo del dia son listados, por ejemplo.
#para probar: touch -d "MM/DD/YYYY HH:mm:ss" nombre_archivo

function comprimirLogs(){
    #IFS=$'\n' #para el momento que invoco esta funcion IFS ya esta seteado asi.
    LOGS=()
    for l in `find "$1" -daystart -mtime +0 | grep -E "\.(txt|log|info)$"`; do
        LOGS+=($l)
    done
    NOM_ZIP=`date +"%Y%m%d%H%M%S" | awk -v DIR=$1 '{
        NOM_SERV = DIR
        sub("\/[^/]+$","",NOM_SERV)
        sub("\/.+\/","",NOM_SERV) 
        print "logs_" NOM_SERV "_" $0 ".zip"
    }'`
    #parametros de zip:
    #-j guardar solo archivos, no crear los directorios.
    #-q no mostrar mensajes por pantalla
    #-T verificar que el archivo se creo correctamente antes de eliminar
    #-m eliminar los archivos originales.
    if [[ ${#LOGS[*]} == 0 ]]; then
        echo "La carpeta '$1' no contiene logs antiguos."
    else
        zip -jqTm "$2/$NOM_ZIP" ${LOGS[*]}
        if [[ $? == 0 ]]; then
            echo "Archivo "$2/$NOM_ZIP" creado."
        fi
    fi
    #unset IFS
}

#importantes las comillas para evitar problemas con las rutas con espacios.
for f in ${DIRECTORIOS[*]:1}; do
    comprimirLogs "$f" "${DIRECTORIOS[0]}"
done
unset IFS

#Corrales Mauro Exequiel, DNI:40137650