REM L'executable (LaboRS.exe) ainsi que les fichiers de la DLL (RecuitDLL.dll et RecuitDLL.lib)
REM  doivent etre dans le meme repertoire que le fichier .txt

REM Liste des paramètres
REM 0)EXECUTABLE 1)NOM_FICHIER 2)Temperature initiale 3)Alpha 4)NB palier 5)NB_EVAL_MAX

::LaboRS.exe	wt40-1.txt	300	0.6	5	6000

::LaboRS.exe	wt50-2.txt	300	0.6	5	6000

::LaboRS.exe	wt100-2.txt	300	0.6	5	6000

execution_stats_40

execution_stats_50

execution_stats_100