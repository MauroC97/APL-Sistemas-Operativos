#*************************************************#
#Nombre del script:ej1.ps1
#APL N°2
#Ej N°1
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

[CmdletBinding()]
Param (
    [Parameter(Position = 1, Mandatory = $false)]
    #Validar que sea un directorio
    [ValidateScript( { Test-Path -PathType Container $_ } )]
    #Ruta del directorio, que no sea obligatoria es un error.
    [String] $ruta,
    #Cantidad de elementos a mostrar, por defecto 0 (lo que no tiene mucho sentido).
    #deberia de ser obligatoria y >= 1
    #como no es posicional, debo si o si utilizar -elem para setearlo.
    [int] $elem = 0
)
#listar todos los directorios de la ruta pasada por parametro
$LIST = Get-ChildItem -Path $ruta -Directory
#iterar por cada elemento encontrado y por cada uno almacenar un objeto que contiene 
#la ruta del directorio y la cantidad de elementos que contiene
$ITEMS = ForEach ($ITEM in $LIST) {
    #Deberia usar .Count en ves de .Length, para este caso funciona igual.
    $COUNT = (Get-ChildItem -Path $ITEM).Length
    $props = @{
        name  = $ITEM
        count = $COUNT
    }
    New-Object psobject -Property $props
}
#ordenar los directorios de mayor a menor segun su cantidad de elementos (count)
#luego tomar los primeros $elem (2do parametro) y mostrar las rutas en formato de tabla sin el encabezado
$CANDIDATES = $ITEMS | Sort-Object -Property count -Descending | Select-Object -First $elem | Select-Object -Property name
Write-Output "Primeros $elem directorios ordenados de mayor a menor segun su cantidad de elementos:" # COMPLETAR
$CANDIDATES | Format-Table -HideTableHeaders

<#
    Respuestas:
        1)El script recibe 2 parametros: una ruta y un numero
          Muestra los primeros (numero) directorios de la (ruta) ordenados de mayor a menor
          segun la cantidad de elementos que contienen.

        4)Respondido a lo largo del codigo, agregaria una validacion al 2do parametro para que 
          este sea >= 1, de otra forma no muestro ningun resultado.

        5)[CmdletBinding()] cambia el modo en el que la funcion/script recibe parametros.
        Cuando se utiliza CmdletBinding, la funcion no acepta parametros desconocidos o 
        parametros que no respetan las posiciones definidas. (Sin CmdletBinding estos se encontrarian
        en la variable $Args)

        6)"": se utilizan para cadenas de texto con reemplazos de variables $ y comandos $()
          '': se utilizan para cadenas de texto literales (sin reemplazos)
          `:  se utiliza como caracter de escape (equivalente a la barra invertida \ en bash)

        7)Si se ejecuta sin parametros, el script solo muestra el mensaje de Write-Output y nada mas.
          No se producen errores pero tampoco realiza ninguna accion util.

Corrales Mauro Exequiel, DNI:40137650
#>