#!/bin/bash

#*************************************************#
#Nombre del script:ej2y3.sh
#APL N°1
#Ej N°2 y N°3
#Corrales Mauro Exequiel, DNI:40137650
#Primera Reentrega
#Correcciones: vea linea 179
#*************************************************#

AYUDA="Utilize -h, -? o --help para obtener ayuda."
FREQ=5 #tiempo en segundos entre cada monitoreo.

#chequear por el param de ayuda
if [[ $# == 1 && ($1 == "-h" || $1 == "-?" || $1 == "--help") ]]; then
    echo "Sintaxis: "$0" -p | --path DIRECTORIO -d | --dia DIA"
    echo "El script monitorea la carpeta indicada por -p y renombra automaticamente los archivos." 
    echo "El parametro dia es opcional, si se incluye se renombraran todos los archivos excluyendo aquellos cuyo dia coincida con el especificado."
    echo "Escriba el nombre del dia sin tildes."
    echo "Ejecute: '$0 -k' para finalizarlo."
    exit 0
fi

#detectar si el script ya esta en ejecucion:
#pgrep retorna los pid de los procesos cuyo nombre cumplen la expresion.
#si encuentro mas de 1 pid, ya se esta ejecutando (ejecucion actual + otras anteriores).
#parametros utilizados:
#-c no retornar los pid, sino el numero de pids encontrados.
#-x evaluar el patron contra el nombre completo
#ejemplo: sin -x "pgrep ba" encuentra "bash", con -x no.

#finalizar ejecucion si se envio -k :
N=`basename $0` #nombre del archivo del script.
if [[ $# == 1 && $1 == "-k" ]]; then
    if [[ `pgrep -cx $N` > 1 ]]; then
        for pid in `pgrep -x $N`; do 
            if [[ $pid != $$ ]]; then #asegurarse de no terminarnos a nosotros mismos.
                kill $pid
            fi
        done
        echo "Finalizando monitoreo."
        exit 0
    else 
        echo "El script no esta ejecutandose actualmente."
        echo "$AYUDA"
        exit 0
    fi
fi

#verificar que el script no se este ejecutando:
if [[ `pgrep -cx $N` > 1 ]]; then
    echo "El script ya se esta ejecutando."
    echo "Ejecute: '$0 -k' para finalizarlo."
    exit 0
fi

function verificarDia() {
    #por defecto awk chequea si se cumplen patrones contra $0, la linea actual
    #en este caso primero paso la linea a minuscula y luego chequeo que no cumpla el patron.
    echo "$1" | awk '
    tolower($0) !~ /^(lunes|martes|miercoles|jueves|viernes|sabado|domingo)$/ {
        exit 1
    }'
#si retorno 0 fue un dia de la semana, 1 si no lo es.
if [ $? == 1 ]; then
    echo "'"$1"' no es un dia de la semana." 
    echo "$AYUDA"
    exit 1
fi
}

function verificarDirectorio() {
#-d verifica si el parametro existe y es un directorio
if [[ ! -d $1 ]]; then 
    echo "'"$1"' no es un directorio valido."
    echo "$AYUDA"
    exit 1
fi
}
function ErrorParametrosIncorrectos(){
    echo "Los parametros son incorrectos."
    echo "$AYUDA"
    exit 1
}

#verificacion de parametros:

dir=''
dia=''
#puede recibir los parametros en cualquier orden mientras se envien correctamente
#si dir o dia no son == '' es porque me pasaron parametros repetidos.
while true; do
    case "$1" in
        -p | --path )
                    if [[ $dir != '' ]]; then
                        ErrorParametrosIncorrectos
                    fi
                    verificarDirectorio "$2"
                    dir="$2"
                    shift;shift;;
        -d | --dia )
                    if [[ $dia != '' ]]; then
                        ErrorParametrosIncorrectos
                    fi
                    verificarDia "$2"
                    dia="$2"
                    shift;shift;;
        * )
            break;;
    esac
done
#si sali por break y hay parametros sin procesar, son incorrectos
if [[ $# > 0 ]]; then
    ErrorParametrosIncorrectos
fi

#el parametro path es obligatorio
if [[ $dir == '' ]]; then
    echo "Debe indicar un directorio."
    echo "$AYUDA"
    exit 1
fi

#logica del programa

#darFormato
#1er param ruta 2do param dia

#dar formato cambia el nombre del archivo por el formato pedido manteniendo la ruta
#de esta forma puedo enviarlo directamente al comando mv
#si el dia a excluir coincide con el del archivo no realiza ningun cambio
#funciones de tiempo:
#https://www.gnu.org/software/gawk/manual/html_node/Time-Functions.html
#utilizar strftime() en awk para obtener el nombre del dia me lo devuelve en ingles siempre.
#por lo tanto lo obtengo en dos partes:
#1 - utilizo la funcion mktime() en awk para llevar la fecha del nombre del archivo a un formato estandar (Tiempo Unix).
#2 - utilizo date para obtener el nombre del dia en el idioma de la maquina a partir de la fecha procesada.

function darFormato(){
#escribo la expresion de esa forma porque mawk no soporta cuantificadores {}
#(mawk viene por defecto con ubuntu)
TIEMPO=`echo "$1" | awk '
    {
        n = match($0,"[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]_[0-9][0-9][0-9][0-9][0-9][0-9]\.jpg$")
        nom = substr($0,n)
        fecha = sprintf("%s %s %s %s %s %s",substr(nom,0,4),substr(nom,5,2),substr(nom,7,2),substr(nom,10,2),substr(nom,12,2),substr(nom,14,2))
        print mktime(fecha)
    }'`

#obtengo el nombre del dia en el idioma de la maquina con date:
DIA=`date -d "@$TIEMPO" +"%A"`

#ignoro el archivo o devuelvo el nombre con el nuevo formato segun corresponda:
#utilizo aux para no perder el acento en la verificacion.
#D/aux = dia del archivo
#D2 = dia pasado por parametro, si existe.
echo "$1" | awk -v T=$TIEMPO -v D=$DIA -v D2=$2 '
    {   
        aux = D
        sub("á","a",aux)
        sub("é","e",aux)
        if (tolower(aux) == tolower(D2)){
            print ""
        }
        else{
        n = match($0,"[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]_[0-9][0-9][0-9][0-9][0-9][0-9]\.jpg$")
        nom = substr($0,n)
        path = substr($0,0,n-1)
        ac = strftime("%H",T)<19 ? "almuerzo" : "cena"
        print path strftime("%Y-%m-%d ",T) sprintf("%s del ",ac) D 
    }}'

}




    #************************************************************#
    # Correccion:
    # el error de los archivos eliminados no era causado por el parametro -d
    # sucedia cuando se tenian varios archivos del mismo dia pero en horas distintas, 
    # ya que como la hora es ignorada en el nombre del archivo final, se sobreescribian.
    # para corregirlo: verifico si el archivo renombrado ya existe, y de ser asi agrego un numero al nombre final.
    #************************************************************#





function renombrar(){
    #find separa los archivos en la salida con saltos de linea
    IFS=$'\n'
    #listar la ruta completa de todos los archivos del directorio indicado 
    #y de los subdirectorios que cumplen con el formato especificado:
    ARCHIVOS=`find "$dir" -type f | grep -E '[0-9]{8}_[0-9]{6}[\.]jpg$'`
    
    for f in $ARCHIVOS; do
    ren=`darFormato "$f" "$dia"`
    if [[ $ren != "" ]]; then
        if [[ ! -e "$ren.jpg" ]]; then #verificar que no existan archivos con el mismo nombre
            mv "$f" "$ren.jpg"
        else
            #cuento cuantos archivos con el mismo nombre ya existen y agrego ese valor al nombre nuevo.
            cant=`find $ren* | wc -l`
            mv "$f" "$ren $cant.jpg"
        fi
    fi 
    done
    unset IFS
}

function monitoreo(){
    while true; do
        renombrar
        sleep $FREQ
    done
}

echo "Monitoreando la carpeta `realpath "$dir"`"
monitoreo &

#Corrales Mauro Exequiel, DNI:40137650