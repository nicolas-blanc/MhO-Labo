#ifndef __ENTETE_H_
#define __ENTETE_H_

#include <cstdio>
#include <cstdlib> 
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>  
#include <cmath>
#include <vector>
using namespace std;


struct TProblem							//**D�finition du probl�me:
{
	std::string Nom;					//**Nom du fichier de donn�es
	int NbCom;							//**Nombre de commandes indiqu� dans le fichier
	std::vector<int> Temps;				//**Temps de production pour fabriquer une commande. NB: Tableau de 0 � NbCom-1.  
	std::vector<int> Poids;				//**Poids (importance) de la commande. NB: Tableau de 0 � NbCom-1.
	std::vector<int> DateDue;			//**Date due de la commande. NB: Tableau de 0 � NbCom-1.
};

struct TSolution						//**D�finition d'une solution: 
{
	std::vector<int> Seq;				//**Indique la s�quence de production des commandes. NB: Tableau de 0 � NbCom-1.
	long FctObj;						//**Valeur de la fonction obj: Sommation des retards pond�r�s
	std::vector<int> Fin;				//**Temps de terminaison d'une commande dans la s�quence
	std::vector<int> Retard;			//**Indique le nombre d'unit�s de temps qu'une commande est produite en retard: Max {0, Fin-DateDue}
	std::vector<int> RetardP;			//**Calcul le retard pond�r� pour une commande (Retard*Poids)	
};

struct TRecuit
{
	double	Temperature;				//Temperature courante du systeme
	int		TempInit;					//Temperature initiale du systeme 
	int		NoPalier;					//Compteur de palier pour la "Dur�e de la temp�rature" ou "Longueur d'�poque".
										//Dur�e de la temp�rature: nombre de r�p�titions � effectuer pour chaque valeur de la temp�rature.
	int		Delta;						//D�gradation courante entre la solution Courante et la solution Next
	double	Alpha;						//Schema de "reduction de la temp�rature" : Temp(t+1) = Alpha * Temp(t)
	int		NbPalier;					//Schema de "Duree de la temp�rature": Duree = NB_EVAL_MAX/NbPalier

	int		CptEval;					//**COMPTEUR DU NOMBRE DE SOLUTIONS EVALUEES. SERT POUR CRITERE D'ARRET.
	int		NB_EVAL_MAX;				//**CRITERE D'ARRET: MAXIMUM "NB_EVAL_MAX" EVALUATIONS.
};

#endif