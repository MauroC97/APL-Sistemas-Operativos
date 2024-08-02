<#
  .SYNOPSIS
  Acumula todas las ventas registradas en archivos .csv y genera un archivo .json

  .DESCRIPTION
  El script carga todos los archivos .csv del directorio especificado, cuenta las ventas realizadas para cada producto
  y al final genera un archivo .json en la ruta especificada con los resultados obtenidos.
  Si se pasa el parametro -excluir, cualquier archivo con el nombre indicado sera ignorado.
  Si hay archivos .csv vacios se informa por pantalla y se los ignora.

  .EXAMPLE
  PS> .\ej4.ps1 "/ventas/archivosCSV" "/ventas/salida"

  .EXAMPLE
  PS> .\ej4.ps1 -directorio "/ventas" -out "/salida" -excluir "sanjusto"

#>

#*************************************************#
#Nombre del script:ej4.ps1
#APL N°2
#Ej N°4
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

Param(
    [Parameter(Mandatory, Position = 1)]
    [ValidateScript({ Test-Path -PathType Container $_ })]
    [String]
    #Directorio que contiene los archivos .csv
    $directorio,
    [Parameter(Mandatory, Position = 2)]
    [ValidateScript({ Test-Path -PathType Container $_ })]
    [String]
    #Directorio donde se creara el archivo json
    $out,
    [ValidateNotNullOrEmpty()]
    [String]
    #Nombre del archivo csv a excluir en el procesamiento
    $excluir
)
Begin {
    if ($((Resolve-Path $directorio).Path) -eq $((Resolve-Path $out).Path)) {
        Write-Error "Los directorios de entrada y salida no deben coincidir."
        Exit 1
    }
}
Process {
    #listar todos los csv, se guarda el nombre del archivo y su path(FullName).
    $archivos = $(Get-ChildItem -Path $directorio -Recurse |`
            Where-Object { $_.Name -Match "\.csv$" } |`
            Select-Object Name, FullName)
    if ($Null -eq $archivos) {
        Write-Error "No se encontraron archivos .csv en el directorio especificado."
        Exit 1
    }
    #SortedList funciona como un diccionario pero las keys estan siempre en orden.
    $entradas = [System.Collections.SortedList]::new()
    foreach ($a in $archivos) {
        #Ignorar si se paso un nombre a -excluir y coincide con el nombre del archivo
        if ($Null -ne $excluir -and $a.Name.ToLower() -eq "$excluir.csv".ToLower()) {
            "Excluyendo $($a.Name)"
            continue
        }
        #Importar el .csv, si esta vacio se informa y si no se acumulan sus valores en $entradas.
        $csv = Import-Csv $a.FullName
        if ($Null -ne $csv) {
            foreach ($c in $csv) {
                #hardcodeado, como se obtienen los nombres del encabezado a partir del objeto?
                $entradas[$c.NombreProducto.ToLower()] += [Int]$c.ImporteRecaudado
            }
        }
        else {
            "Error de procesamiento en el archivo $($a.Name)."
        }
    }
    #No se crea un json si todos los archivos estaban vacios.
    if ($entradas.Count -eq 0) {
        Write-Error "Todos los archivos .csv estaban vacios o tenian errores de procesamiento."
        Exit 1
    }
}
End {
    #-Compress escribe el json entero en una linea igual que en el enunciado.
    ConvertTo-Json -Compress $entradas > "$out/salida.json"
}

#Corrales Mauro Exequiel, DNI:40137650