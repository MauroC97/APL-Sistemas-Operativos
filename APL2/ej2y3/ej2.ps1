<#
  .SYNOPSIS
  Renombra automaticamente todas las fotos del directorio, excepto las del dia de la semana indicado.

  .DESCRIPTION
  El script renombrara todas las fotos del directorio especificado cuyo nombre cumpla el formato:
    "yyyyMMdd_HHmmss.jpg"
  Excepto aquellas fotos cuyo dia coincida con el indicado por parametro.
  El formato de las fotos luego de renombrarlas es el siguiente:
    "dd-MM-yyyy (almuerzo|cena) del NombreDia.jpg"

  .EXAMPLE
  PS> .\ej2.ps1 "imagenes/fotos" -Dia miercoles

  .EXAMPLE
  PS> .\ej2.ps1 "fotos"

#>

#*************************************************#
#Nombre del script:ej2.ps1
#APL N°2
#Ej N°2
#Corrales Mauro Exequiel, DNI:40137650
#Primera Entrega
#*************************************************#

Param(
    [Parameter(Mandatory, Position = 1)]
    [ValidateScript({ Test-Path -PathType Container $_ })]
    [String]
    #Directorio que contiene las fotos
    $Directorio,
    [Parameter(Position = 2)]
    [ValidateNotNullOrEmpty()]
    [ValidateScript({ $_.ToLower() -in @("lunes", "martes", "miercoles", "jueves", "viernes", "sabado", "domingo","miércoles","sábado") })]
    [String]
    #Nombre del dia de la semana a ignorar
    $Dia
)

Begin {
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

    function Renombrar-Fotos {
        $fotos = $(Get-ChildItem -Path $Directorio -Recurse |`
                Where-Object { $_.Name -Match "[0-9]{8}_[0-9]{6}[\.]jpg$" } |`
                Select-Object Name, FullName, Directory) 
        
                foreach ($f in $fotos) {
            $ren = Dar-Formato -Nom $f.Name
            if ($null -ne $ren){
                #Verificar si ya existe un archivo con ese nombre y de ser asi agregar numeros
                if(Test-Path "$($f.Directory)/$ren.jpg"){
                $cont=1
                while(Test-Path "$($f.Directory)/$ren`_$cont.jpg"){
                    $cont++
                }
                $ren="$ren`_$cont"
            }
                Rename-Item -Path $f.FullName -NewName "$ren.jpg"
            }
        }
    
    }
    #Agregar acentos:
    switch ($Dia) {
        "miercoles" { $Dia = "miércoles" }
        "sabado" { $Dia = "sábado" }
    }
}
Process {
    Renombrar-Fotos
}
#Corrales Mauro Exequiel, DNI:40137650