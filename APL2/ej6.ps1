<#
  .SYNOPSIS
  Ejecuta operaciones matematicas basicas

  .DESCRIPTION
  El script funciona como una calculadora, se indican dos numeros y una operacion y se muestra por pantalla el resultado.

  .EXAMPLE
  PS> .\ej6.ps1 2 3 -suma 

  .EXAMPLE
  PS> .\ej6.ps1 -n1 2 -n2 3.5 -resta

#>

#*************************************************#
#Nombre del script:ej6.ps1
#APL N°2
#Ej N°6
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#


Param (
    [Parameter(Mandatory,
    ParameterSetName = 's')]
    [switch]$suma,
    [Parameter(Mandatory,
    ParameterSetName = 'r')]
    [switch]$resta,
    [Parameter(Mandatory,
    ParameterSetName = 'd')]
    [switch]$division,
    [Parameter(Mandatory,
    ParameterSetName = 'm')]
    [switch]$multiplicacion,
    [Parameter(Position = 1, Mandatory)]
    [ValidateNotNullOrEmpty()] 
    [Double]$n1,
    [Parameter(Position = 2, Mandatory)]
    [ValidateNotNullOrEmpty()]
    [Double]$n2
)
Begin{
    if ($division -and $n2 -eq 0){
        Write-Error "No se puede dividir por cero."
        Exit 1
    }
}
Process{
    switch ($PSCmdlet.ParameterSetName){
        "s" {"$($n1+$n2)"}
        "r" {"$($n1-$n2)"}
        "m" {"$($n1*$n2)"}
        "d" {"$($n1/$n2)"}
    }
}

#Corrales Mauro Exequiel, DNI:40137650