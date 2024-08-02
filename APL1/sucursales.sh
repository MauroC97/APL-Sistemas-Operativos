#!/bin/bash

#*************************************************#
#Nombre del script:sucursales.sh
#APL N°1
#Ej N°4
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

AYUDA="Utilize -h, -? o --help para obtener ayuda."

#chequear por el param de ayuda
if [[ $# == 1 && ($1 == "-h" || $1 == "-?" || $1 == "--help") ]]; then
    echo "Sintaxis: "$0" -d DIR_CSV -o DIR_SALIDA -e SUCURSAL (opcional)" 
    echo "Ejemplo: "$0" -d \"CarpetaCSV\" -o \"../Salida\" -e \"SanJusto\""
    echo "Si se indica, el archivo .csv de la sucursal indicada con -e no sera tenido en cuenta."
    echo "Solo se puede excluir 1 sucursal, el nombre no diferencia entre mayusculas y minusculas."
    exit 0
fi

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

dir_entrada=''
dir_salida=''
sucursal=''
while true; do
    case "$1" in
        -d )
            if [[ $dir_entrada != '' ]]; then
                ErrorParametrosIncorrectos
            fi
            verificarDirectorio "$2"
            dir_entrada="$2"
            shift;shift;;
        -e )
            if [[ $sucursal != '' ]]; then
                ErrorParametrosIncorrectos
            fi
            sucursal=$2
            shift;shift;;
        -o )
            if [[ $dir_salida != '' ]]; then
                ErrorParametrosIncorrectos
            fi
            verificarDirectorio "$2"
            dir_salida=$2
            shift;shift;;        
        * )
            break;;
    esac
done
#si sali por break y hay parametros sin procesar, son incorrectos
if [[ $# > 0 ]]; then
    ErrorParametrosIncorrectos
fi

#el parametro -d es obligatorio
if [[ $dir_entrada == '' ]]; then
    echo "Debe indicar el directorio con los archivos CSV."
    echo "$AYUDA"
    exit 1
fi

#-o tambien
if [[ $dir_salida == '' ]]; then
    echo "Debe indicar el directorio donde se generara el resumen."
    echo "$AYUDA"
    exit 1
fi

#realpath resuelve rutas relativas y devuelve las absolutas.
if [[ `realpath "$dir_salida"` == `realpath "$dir_entrada"` ]]; then
    echo "Los directorios de entrada y salida no deben coincidir."
    echo "$AYUDA"
    exit 1
fi

#logica del programa
#obtengo una lista con la ruta completa de los .csv en el directorio y los subdirectorios.
ARCHIVOS=`find "$dir_entrada" -type f | grep '[\.]csv$'`
if [[ -z $ARCHIVOS ]]; then
    echo "No se encontraron archivos .csv en el directorio especificado."
    exit 1
fi
#filtro los archivos vacios contando las lineas que tienen, e informo si encuentro alguno.
IFS=$'\n'
ENTRADAS=()
for a in $ARCHIVOS; do
    l=`wc -l < "$a"`
    # 0 = archivo vacio
    # 1 = solo tiene el encabezado, lo cuento como vacio.
    if [[ $l == 0 || $l == 1 ]]; then
        echo "Error de procesamiento en el archivo "$a""
    else
        ENTRADAS+=($a)
    fi
done

#Si todos los archivos estaban vacios no hay nada que hacer.
if [[ ${#ENTRADAS[*]} == 0 ]]; then
    echo "Todos los archivos encontrados tenian errores de procesamiento."
    unset IFS 
    exit 1
fi

#no valido el contenido de los archivos .csv, asumo que todos tienen el formato correcto.
#FNR == 1 -> utilizo la primer linea de cada archivo para decidir si lo debo ignorar o no.
#es la linea del encabezado.
#obtener nombre del archivo:
#\/[^/]+\.csv$ -> un / seguido de al menos un caracter que no sea / con .csv al final
#si uso .+ en lugar de [^/]+ obtengo el directorio completo, que no es lo que busco.
#sumo uno para obtener el indice donde empieza el nombre ignorando la / que lo precede
#luego elimino la extension del archivo para quedarme con el nombre y lo comparo.

DATOS=`awk -v EXCLUIR=$sucursal '
BEGIN{
    FS = ","
}
FNR == 1 {
    inicio = match(FILENAME,"\/[^/]+\.csv$") + 1
    nombre_sucursal = substr(FILENAME,inicio)
    sub("\.csv$","",nombre_sucursal)
    if (tolower(EXCLUIR) == tolower(nombre_sucursal)){
        nextfile
    }
}
FNR > 1 {
    datos[tolower($1)] += $2
    }
END{
    for (d in datos){
        out = out d ":" datos[d] "\n"
    }
    print out
}' ${ENTRADAS[*]}`
unset IFS

#puede pasar que solo un archivo sea valido y lo excluya con -e
if [[ $DATOS == '' ]]; then
    echo "Se excluyeron todos los archivos .csv validos."
    exit 0
fi

#datos contiene los valores acumulados, una entrada por linea para poder ordenarlos con sort.
#luego vuelvo a pasarle los datos ahora ordenados a awk para crear el archivo json

sort <<< $DATOS | awk '
BEGIN{
    FS = ":"
    out = "{ "
}
{
    sub("^[a-z]",toupper(substr($1,0,1)),$1)
    out = out $1 ": " $2 ", "
}
END{
    sub(", $" , " }" , out)
    print out
}
' > "$dir_salida/salida.json"
if [ $? == 0 ]; then
    echo "Archivo '`realpath "$dir_salida"`/salida.json' creado."
fi

#Corrales Mauro Exequiel, DNI:40137650