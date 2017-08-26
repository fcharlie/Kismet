# Kismet File Hashsum

## None

![image](./docs/images/none.png)


## SHA1 Collision Detection

![image](./docs/images/coll.png)

## Progress

![image](./docs/images/progress.png)

## Complete

![complete](./docs/images/complete.png)

## SHA3-512

![sha3-512](./docs/images/sha3-512.png)

## Theme 

Your can select system menu, click Theme, change your Panel Color, or modify Kismet.exe.ini,
change Content color, and set your title

You can use a color that fills the entire window

![theme](./docs/images/theme.png)

![theme](./docs/images/theme2.png)

Change Title

![title](./docs/images/title.png)

WORKING IN PROGRESS

## Kisasum cli hash utility

Now. we provider a cli utility called `kisasum`, support create xml or json hash result.

![Kisasum](./docs/images/kisasum.png)

Powershell invoke Kisasum:

```powershell
 $hash=.\Kisasum.exe $FilePath --format=json -a sha3-256|ConvertFrom-JSON
 foreach($f in $hash.files){
   Write-Host "$($f.hash)    $($f.name)"
 }
```