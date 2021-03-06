REM L'executable (LaboRS.exe) ainsi que les fichiers de la DLL (RecuitDLL.dll et RecuitDLL.lib)
REM  doivent etre dans le meme repertoire que le fichier .txt

REM Liste des param�tres
REM 0)EXECUTABLE 1)NOM_FICHIER 2)Temperature initiale 3)Alpha 4)NB palier 5)NB_EVAL_MAX 6)DEBUG_MODE 7)CLIMBING_TYPE

:: D�finition NB_EVAL_MAX
set NB_EVAL_MAX=6000

:: D�finition du nombre de r�p�titions
set NB_REPET=5

set TEMPERATURE_INITIALE=300
set ALPHA=0.6
set NB_PALIER=5

set DEBUG_MODE=2
set CLIMBING_TYPE=1

@echo off
:: Activation de l'expansion retard�e pour les boucles for
setlocal enabledelayedexpansion

:: Cr�ation des NB_REPET solutions dans le fichier Resultats.txt
for /l %%i in (1, 1, %NB_REPET%) do (
LaboRS.exe	wt100-2.txt	%TEMPERATURE_INITIALE%	%ALPHA%	%NB_PALIER% %NB_EVAL_MAX% %DEBUG_MODE% %CLIMBING_TYPE%

)

:: Parsing du fichier r�sultat pour r�cup�rer les diff�rents r�sultats
set findtext="Fitness"
set findfile="Resultats.txt"

set /a "moyenne=0"
set /a "nombre=0"
set /a "max=0"
set /a "min=(1<<31)-1"

:: Calcul de la moyenne des r�sultats obtenus
for /f "tokens=31* delims=: " %%a in ('findstr %findtext% %findfile%') do (
	for /f "tokens=31* delims=: " %%c in ("%%b") do (
		for /f "tokens=30* delims=: " %%e in ("%%d") do (
			for /f "tokens=10* delims=: " %%g in ("%%f") do (
				set /a "moyenne=(!moyenne!*!nombre!+%%g)/(!nombre!+1)"
				set /a "nombre+=1"
				if !min! gtr %%g (
					set /a "min=%%g"
				)
				if !max! lss %%g (
					set /a "max=%%g"
				)
			)
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

