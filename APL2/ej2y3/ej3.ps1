<#
  .SYNOPSIS
  Renombra automaticamente todas las fotos del directorio, excepto las del dia de la semana indicado.

  .DESCRIPTION
  El script renombrara todas las fotos del directorio especificado cuyo nombre cumpla el formato:
    "yyyyMMdd_HHmmss.jpg"
  Excepto aquellas fotos cuyo dia coincida con el indicado por parametro.
  El formato de las fotos luego de renombrarlas es el siguiente:
    "dd-MM-yyyy (almuerzo|cena) del NombreDia.jpg"
  Para finalizar el monitoreo utilize el parametro -k

  .EXAMPLE
  PS> .\ej3.ps1 "imagenes/fotos" -Dia miercoles

  .EXAMPLE
  PS> .\ej3.ps1 "fotos"

#>

#*************************************************#
#Nombre del script:ej3.ps1
#APL N°2
#Ej N°3
#Corrales Mauro Exequiel, DNI:40137650
#Primera Reentrega
#*************************************************#

Param(
    [Parameter(Mandatory,ParameterSetName = "k")]
    [Switch] $k,
    [Parameter(Mandatory, Position = 1,ParameterSetName = "renombrar")]
    [ValidateScript({ Test-Path -PathType Container $_ })]
    [String]
    #Directorio que contiene las fotos
    $Directorio,
    [Parameter(Position = 2,ParameterSetName = "renombrar")]
    [ValidateNotNullOrEmpty()]
    [ValidateScript({ $_.ToLower() -in @("lunes", "martes", "miercoles", "jueves", "viernes", "sabado", "domingo","miércoles","sábado") })]
    [String]
    #Nombre del dia de la semana a ignorar
    $Dia
)

Begin {
    if($PSCmdlet.ParameterSetName -eq "k"){
        #verificar que el job "ej3" exista:
        if(-not [bool]$(Get-Job "ej3" -ErrorAction SilentlyContinue)){
            Write-Host "El script no se esta ejecutando."
            Exit
        }
        Write-Host "Terminando monitoreo."
        Stop-Job "ej3"
        Remove-Job "ej3"
        Exit
    }
    if([bool]$(Get-Job "ej3" -ErrorAction SilentlyContinue)){
        Write-Host "El script ya se esta ejecutando."
        Exit 1
    }
    $Directorio = $(Resolve-Path $Directorio).Path
    #Agregar acentos:
    switch ($Dia) {
        "miercoles" { $Dia = "miércoles" }
        "sabado" { $Dia = "sábado" }
    }
}
Process {
    Start-Job -Name "ej3" -ArgumentList $Directorio,$Dia -ScriptBlock{
        #defino las funciones dentro del job para poderlas utilizar fuera del script
        function Renombrar-Fotos {
            Param(
                [Parameter(Mandatory, Position = 1)]
                [ValidateScript({ Test-Path -PathType Container $_ })]
                [String]
                #Directorio que contiene las fotos
                $Directorio)
                function Dar-Formato {
                    Param(
                        [String]$Nom
                    )
                    $Fecha = [DateTime]::ParseExact($Nom.Substring(0, $($Nom.Length - 4)), "yyyyMMdd_HHmmss", $null)
                    #ignorar si coincide el dia.
                    $dia_archivo = $Fecha.ToString("dddd",(Get-Culture -Name 'es-AR'))
                    if($dia_archivo -eq $Dia){
                        return $null
                    }
                    $ac = $Fecha.Hour -lt 19 ? "Almuerzo" : "Cena"
                    $Nom = "$($Fecha.ToString("dd-MM-yyyy")) $ac del $dia_archivo"
                    return $Nom
                }
    
            $fotos = $(Get-ChildItem -Path $Directorio -Recurse |`
                    Where-Object { $_.Name -Match "[0-9]{8}_[0-9]{6}[\.]jpg$" } |`
                    Select-Object Name, FullName, Directory) 
        
            foreach ($f in $fotos) {
                $ren = Dar-Formato -Nom $f.Name
                if ($null -ne $ren) {
                    #Verificar si ya existe un archivo con ese nombre y de ser asi agregar numeros
                    if (Test-Path "$($f.Directory)/$ren.jpg") {
                        $cont = 1
                        while (Test-Path "$($f.Directory)/$ren`_$cont.jpg") {
                            $cont++
                        }
                        $ren = "$ren`_$cont"
                    }
                    Rename-Item -Path $f.FullName -NewName "$ren.jpg"
                }
            }
    
        }
        #fin definicion de funciones
    #Creo el filesystemwatcher y me subscribo a los eventos
    $FSW = New-Object System.IO.FileSystemWatcher
    $FSW.Path = $args[0]
    $Directorio = $args[0]
    $Dia = $args[1]
    $FSW.Filter = "*.jpg"
    $FSW.IncludeSubdirectories = $true
    Register-ObjectEvent $FSW -EventName "Created" 
    Register-ObjectEvent $FSW -EventName "Renamed"
    $FSW.EnableRaisingEvents = $true
    #renombrar una vez al inicio y luego cada vez que suceda un evento.
    Renombrar-Fotos $Directorio
    while ($true) {
        Wait-Event | Remove-Event
        Renombrar-Fotos $Directorio
    }
    }
}
#Corrales Mauro Exequiel, DNI:40137650