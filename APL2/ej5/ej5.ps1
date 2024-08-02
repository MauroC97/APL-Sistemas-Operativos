<#
  .SYNOPSIS
  Comprime todos los logs que son un dia o mas antiguos y los elimina.

  .DESCRIPTION
  El script recibe las rutas a las carpetas que contienen los logs 
  y la ruta donde almacenar los archivos comprimidos en un archivo de configuracion.
  Por cada ruta indicada, se comprimen y eliminan todos los logs encontrados que sean un dia o mas
  antiguos y se almacena el archivo zip en la ruta indicada por el archivo de config.

  .EXAMPLE
  PS> .\ej5.ps1 "config.txt"

  .EXAMPLE
  PS> .\ej5.ps1 -configpath "config.txt"

#>

#*************************************************#
#Nombre del script:ej5.ps1
#APL N°2
#Ej N°5
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#


Param(
    [Parameter(Mandatory)]
    [ValidateScript({ Test-Path -PathType leaf $_ })]
    [String]
    #Ruta al archivo de configuracion
    $configpath
)
Begin {
    #cargar rutas del archivo config y verificarlas.
    $directorios = Get-Content $configpath
    #al menos 2 rutas, el directorio de salida y uno o mas directorios de logs.
    if ($directorios.Count -lt 2) {
        Write-Error "El archivo de configuracion no es valido o esta incompleto."
        Exit 1
    }
    #Cada linea debe ser un directorio valido
    foreach ($d in $directorios) {
        if (-not (Test-Path -PathType Container $d)) {
            Write-Error "$d no es un directorio valido."
            Exit 1
        }
    }
    function Comprimir-Logs {
        Param(
            [Parameter(Mandatory, Position = 1)]
            [ValidateScript({ Test-Path -PathType Container $_ })]
            [String]
            $outpath,
            [Parameter(Mandatory, Position = 2)]
            [ValidateScript({ Test-Path -PathType Container $_ })]
            [String]
            $inpath
        )
        #tomar todos los logs que sean un dia o mas antiguos que la fecha en la que se corre el script
        $logs = $(Get-ChildItem -Path $inpath -Recurse |`
                Where-Object { $_.Name -Match "\.(txt|log|info)$" -and $_.LastWriteTime -lt $((Get-Date).AddDays(-1)) } |`
                Select-Object -Expand FullName)
        if ($logs.Count -eq 0) {
            "La carpeta $inpath no contiene logs antiguos."
            Return
        }
        #Obtengo el nombre de la carpeta pedida con una expresion.
        #redirecciono salida a null para que no muestre True por pantalla
        $inpath -match "(?:.+[\/|\\])([^/\\]+)(?:[\/|\\].+$)" > $Null
        #el grupo capturado se guarda automaticamente en la variable $Matches
        $nom_serv = $Matches.1
        $fecha = $(Get-Date -Format "yyyyMMdd_HHmmss")
        $path_zip = "$outpath/logs_$nom_serv`_$fecha.zip"
        foreach ($l in $logs) {
            #Agregar logs al zip y eliminar el archivo original de a uno por ves:
            Compress-Archive -Path $l -DestinationPath $path_zip -Update > $Null
            Remove-Item $l
        }
        if ($? -eq $True) {
            "Archivo $path_zip creado."
        }
    }
}
Process {
    $out = $directorios[0]
    foreach ($d in $directorios[1..($directorios.Count - 1)]) {
        Comprimir-Logs $out $d
    }
}

#Corrales Mauro Exequiel, DNI:40137650