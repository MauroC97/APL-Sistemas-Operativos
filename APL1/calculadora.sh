#!/bin/bash

#*************************************************#
#Nombre del script:calculadora.sh
#APL N°1
#Ej N°6
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

AYUDA="Utilize -h, -? o --help para obtener ayuda."

function ErrorParametrosIncorrectos(){
    echo "Los parametros ingresados son incorrectos"
    echo "$AYUDA"
    exit 1
}

#chequear por el param de ayuda
if [[ $# == 1 && ($1 == "-h" || $1 == "-?" || $1 == "--help") ]]; then
    echo "Sintaxis: $0 -n1 NUMERO1 -n2 NUMERO2 -(suma|resta|multiplicacion|division)"
    echo "Ejemplo: $0 -n1 2 -n2 2 -suma"
    echo "Tambien puede usar -s -r -m o -d"
    echo "Debe respetar el orden de los parametros."
    exit 0
fi
if [[ $# > 5 ]]; then
    ErrorParametrosIncorrectos
fi

#asumiendo que los parametros vienen siempre en orden
#ej ./script.sh -n1 1234 -n2 1234 -operacion

#verifico que los parametros sean numeros con una expresion:
#^ desde el principio de la cadena
#[-\+]? puede o no haber un signo mas o menos
#[0-9]+ asegura que hay al menos un numero.
#([\.,][0-9]+)?$ opcionalmente, le seguira un punto o una coma, seguido de uno o mas numeros
#el $ indica el final de la cadena.

function validar(){
    if [[ ! $1 =~ ^[-\+]?[0-9]+([\.,][0-9]+)?$ ]];then
        echo "Los operandos n1 y n2 deben ser numeros."
        echo "$AYUDA"
        exit 1
    fi
}
n1=''
n2=''
while true; do
    case "$1" in
        -n1 )
                if [[ $n1 != '' ]]; then
                    ErrorParametrosIncorrectos
                fi
                validar "$2"
                n1=$2
                shift;shift;;
        -n2 )
                if [[ $n2 != '' ]]; then
                    ErrorParametrosIncorrectos
                fi
                validar "$2"
                n2=$2
                shift;shift;;
        * )
            break;;
    esac
done
#como estoy asumiendo que los parametros vienen ordenados
#si para este punto no queda un unico parametro a procesar, 
#es porque no me enviaron los parametros correctos
if [[ $# != 1 ]]; then
    ErrorParametrosIncorrectos
fi

case "$1" in
        -suma | -s)
                echo "" | awk -v n1=$n1 -v n2=$n2 '{print n1 + n2}'
                exit 0;;
        -resta | -r)
                echo "" | awk -v n1=$n1 -v n2=$n2 '{print n1 - n2}'
                exit 0;;
        -multiplicacion | -m)
                echo "" | awk -v n1=$n1 -v n2=$n2 '{print n1 * n2}'
                exit 0;;
        -division | -d)
                echo "" | awk -v n1=$n1 -v n2=$n2 '{
                    if (n2!=0){
                        print n1 / n2
                    }
                    else{
                        print "No se puede dividir por cero."
                    }
                    }'
                exit 0;;
        * )
            ErrorParametrosIncorrectos;;
    esac

#Corrales Mauro Exequiel, DNI:40137650