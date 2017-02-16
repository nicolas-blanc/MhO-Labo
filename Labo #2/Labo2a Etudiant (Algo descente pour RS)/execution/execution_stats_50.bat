REM L'executable (LaboRS.exe) ainsi que les fichiers de la DLL (RecuitDLL.dll et RecuitDLL.lib)
REM  doivent etre dans le meme repertoire que le fichier .txt

REM Liste des paramètres
REM 0)EXECUTABLE 1)NOM_FICHIER 2)Temperature initiale 3)Alpha 4)NB palier 5)NB_EVAL_MAX 6)DEBUG_MODE 7)CLIMBING_TYPE

:: Définition NB_EVAL_MAX
set NB_EVAL_MAX=6000

:: Définition du nombre de répétitions
set NB_REPET=10

set TEMPERATURE_INITIALE=1
set ALPHA=0.1
set NB_PALIER=1000

set DEBUG_MODE=2
set CLIMBING_TYPE=1

@echo off
:: Activation de l'expansion retardée pour les boucles for
setlocal enabledelayedexpansion

:: Création des NB_REPET solutions dans le fichier Resultats.txt
for /l %%i in (1, 1, %NB_REPET%) do (
LaboRS.exe	wt50-2.txt	%TEMPERATURE_INITIALE%	%ALPHA%	%NB_PALIER% %NB_EVAL_MAX% %DEBUG_MODE% %CLIMBING_TYPE%

)

:: Parsing du fichier résultat pour récupérer les différents résultats
set findtext="Fitness"
set findfile="Resultats.txt"

set /a "moyenne=0"
set /a "nombre=0"
set /a "max=0"
set /a "min=(1<<31)-1"

:: Calcul de la moyenne des résultats obtenus
for /f "tokens=31* delims=: " %%a in ('findstr %findtext% %findfile%') do (
	for /f "tokens=21* delims=: " %%c in ("%%b") do (
		set /a "moyenne=(!moyenne!*!nombre!+%%c)/(!nombre!+1)"
		set /a "nombre+=1"
		if !min! gtr %%c (
			set /a "min=%%c"
		)
		if !max! lss %%c (
			set /a "max=%%c"
		)
	)
)

:: Affichage de la moyenne obtenue
echo moyenne obtenue : %moyenne%
echo maximum obtenu : %max%
echo minimum obtenu : %min%
echo nombre de solutions comptees : %nombre%


pause
exit /b

