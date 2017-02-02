#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"DescenteDLL.lib")  
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (DescenteDLL.dll et DescenteDLL.lib) doivent 
//se trouver dans le répertoire courant du projet pour une exécution à l'aide du compilateur. Indiquer les
//arguments du programme dans les propriétés du projet - débogage - arguements.
//Sinon, utiliser le répertoire execution.

// Permet de calculer le pas
#define FRACTION 3
// Nombre de test d'insertion avant la position actuelle
#define BEFORE 3
// Nombre de test d'insertion après la position actuelle
#define AFTER 3


//*****************************************************************************************
// Prototype des fonctions se trouvant dans la DLL 
//*****************************************************************************************
//DESCRIPTION:	Lecture du Fichier probleme et initialiation de la structure Problem
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TAlgo &unAlgo);

//DESCRIPTION:	Fonction d'affichage à l'écran permettant de voir si les données du fichier problème ont été lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION: Affichage d'une solution a l'écran pour validation (avec ou sans détails des calculs)
extern "C" _declspec(dllimport) void AfficherSolution(const TSolution uneSolution, TProblem unProb, std::string Titre, bool AvecCalcul);

//DESCRIPTION: Affichage à l'écran de la solution finale, du nombre d'évaluations effectuées et de certains paramètres
extern "C" _declspec(dllimport) void AfficherResultats (const TSolution uneSol, TProblem unProb, TAlgo unAlgo);

//DESCRIPTION: Affichage dans un fichier(en append) de la solution finale, du nombre d'évaluations effectuées et de certains paramètres
extern "C" _declspec(dllimport) void AfficherResultatsFichier (const TSolution uneSol, TProblem unProb, TAlgo unAlgo, std::string FileName);

//DESCRIPTION:	Évaluation de la fonction objectif d'une solution et MAJ du compteur d'évaluation. 
//				Fonction objectif représente le retard total pondéré
extern "C" _declspec(dllimport) void EvaluerSolution(TSolution & uneSol, TProblem unProb, TAlgo &unAlgo);

//DESCRIPTION:	Création d'une séquence aléatoire de parcours des villes et évaluation de la fonction objectif. Allocation dynamique de mémoire
// pour la séquence (.Seq)
extern "C" _declspec(dllimport) void CreerSolutionAleatoire(TSolution & uneSolution, TProblem unProb, TAlgo &unAlgo);

//DESCRIPTION: Copie de la séquence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retournée.
extern "C" _declspec(dllimport) void CopierSolution (const TSolution uneSol, TSolution &Copie, TProblem unProb);

//DESCRIPTION:	Libération de la mémoire allouée dynamiquement
extern "C" _declspec(dllimport) void	LibererMemoireFinPgm	(TSolution uneCourante, TSolution uneNext, TSolution uneBest, TProblem unProb);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************

//DESCRIPTION:	Création d'une solution voisine à partir de la solution uneSol. NB:uneSol ne doit pas être modifiée
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TAlgo &unAlgo);

//DESCRIPTION:	 Echange de deux villes sélectionnée aléatoirement. NB:uneSol ne doit pas être modifiée
TSolution	Echange			(const TSolution uneSol, TProblem unProb, TAlgo &unAlgo);

//DESCRIPTION:	 Echange de deux villes sélectionnée aléatoirement. NB:uneSol ne doit pas être modifiée
TSolution	Insertion		(const TSolution uneSol, TProblem unProb, TAlgo &unAlgo);


//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{
	TSolution Courante;		//Solution active au cours des itérations
	TSolution Next;			//Solution voisine retenue à une itération
	TSolution Best;			//Meilleure solution depuis le début de l'algorithme //Non utilisé pour le moment 
	TProblem LeProb;		//Définition de l'instance de problème
	TAlgo LAlgo;			//Définition des paramètres de l'agorithme
	string NomFichier;
		
	//**Lecture des paramètres
	NomFichier.assign(Param[1]);
	LAlgo.NB_EVAL_MAX= atoi(Param[2]);

	//**Lecture du fichier de donnees'
	LectureProbleme(NomFichier, LeProb, LAlgo);
	//AfficherProbleme(LeProb);
	
	//**Création de la solution initiale 
	CreerSolutionAleatoire(Courante, LeProb, LAlgo);
	AfficherSolution(Courante, LeProb, "SolInitiale: ", true);

	do
	{
		Next = GetSolutionVoisine(Courante, LeProb, LAlgo);
		//AfficherSolution(Courante, LeProb, "Courante: ", false);
		//AfficherSolution(Next, LeProb, "Next: ", false);
		if (Next.FctObj <= Courante.FctObj)	//**amélioration
		{
				Courante = Next;

// 				cout << "Fct Obj Nouvelle Courante: " << Courante.FctObj << endl;
//  				AfficherSolution(Courante, LeProb, "NouvelleCourante: ", true);
		}
	}while (LAlgo.CptEval < LAlgo.NB_EVAL_MAX && Courante.FctObj!=0);

	AfficherResultats(Courante, LeProb, LAlgo);
	AfficherResultatsFichier(Courante, LeProb, LAlgo,"Resultats.txt");
	
	LibererMemoireFinPgm(Courante, Next, Best, LeProb);

 	system("PAUSE");
	return 0;
}

//DESCRIPTION: Création d'une solution voisine à partir de la solution uneSol qui ne doit pas être modifiée.
//Dans cette fonction, on appel le TYPE DE VOISINAGE sélectionné + on détermine la STRATÉGIE D'ORIENTATION. 
//Ainsi, si la RÈGLE DE PIVOT nécessite l'étude de plusieurs voisins, la fonction TYPE DE VOISINAGE sera appelée plusieurs fois.
//Le TYPE DE PARCOURS dans le voisinage interviendra normalement dans la fonction TYPE DE VOISINAGE.
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TAlgo &unAlgo)
{
	//Type de voisinage : À MODIFIER (Echange 2 commandes aléatoires)
	//Parcours dans le voisinage : À MODIFIER	(Aleatoire)
	//Règle de pivot : À MODIFIER	(First-Impove)

	TSolution unVoisin;
	
// 	unVoisin = Echange(uneSol, unProb, unAlgo);
	unVoisin = Insertion(uneSol, unProb, unAlgo);
	return (unVoisin);
}

//DESCRIPTION: Echange de deux commandes sélectionnées aléatoirement
/*
TSolution Echange (const TSolution uneSol, TProblem unProb, TAlgo &unAlgo)
{
	TSolution Copie;
	int PosA, PosB, Tmp;

	//Utilisation d'une nouvelle TSolution pour ne pas modifier La solution courante (uneSol)
	CopierSolution(uneSol, Copie, unProb);
	
	//Tirage aléatoire des 2 commandes
	PosA = rand() % unProb.NbCom;
	do
	{
		PosB = rand() % unProb.NbCom;
	}while (PosA == PosB); //Validation pour ne pas consommer une évaluation inutilement
	
	//Echange des 2 commandes
	Tmp = Copie.Seq[PosA];
	Copie.Seq[PosA] = Copie.Seq[PosB];
	Copie.Seq[PosB] = Tmp;
	
	//Le nouveau voisin doit être évalué 
	EvaluerSolution(Copie, unProb, unAlgo);
	return(Copie);
}*/

TSolution createTempSolution(const TSolution uneSol, int posA, int pos, TProblem unProb)
{
	TSolution copie;
	CopierSolution(uneSol, copie, unProb);

/*
	for (int i = posA; i < pos - 1; i++)
	{
		copie.Seq[i] = copie.Seq[i + 1];
	}
	copie.Seq[pos - 1] = uneSol.Seq[posA];
*/

	for (int i = 0; i < unProb.NbCom; i++)
	{
		if (i == pos)
		{
			copie.Seq[i] = uneSol.Seq[posA];
		}
		else if (i > pos && i <= posA)
		{
			copie.Seq[i] = uneSol.Seq[i - 1];
		}
		else if (i < pos && i >= posA)
		{
			copie.Seq[i] = uneSol.Seq[i + 1];
		}
		else
		{
			copie.Seq[i] = uneSol.Seq[i];
		}
	}


	return copie;
}

TSolution compare(TSolution tmpSol, TSolution bestTmpSol)
{
	if (tmpSol.FctObj < bestTmpSol.FctObj)
	{
		return tmpSol;
	}
	else
	{
		return bestTmpSol;
	}
}

TSolution Insertion(const TSolution uneSol, const TProblem unProb, TAlgo &unAlgo)
{
	TSolution bestTmpSol, tmpSol;
	int posA, posPred;
	vector<int> posSol;

	CopierSolution(uneSol, bestTmpSol, unProb);

	posA = rand() % unProb.NbCom;
	posPred = posA;

	int i = 0;
	while (posPred >= 0 && i < BEFORE)
	{
		posPred = posPred - rand() % (unProb.NbCom / FRACTION) - 1;

		if (posPred > 0)
		{
			posSol.push_back(posPred);
		}

		i++;
	}

	i = 0;
	posPred = posA;
	while (posPred < unProb.NbCom && i < AFTER)
	{
		posPred = posPred + rand() % (unProb.NbCom / FRACTION) + 1;

		if (posPred < unProb.NbCom)
		{
			posSol.push_back(posPred);
		}

		i++;
	}

	for each (int pos in posSol)
	{
		tmpSol = createTempSolution(uneSol, posA, pos, unProb);

		if (unAlgo.CptEval < unAlgo.NB_EVAL_MAX)
		{
			EvaluerSolution(tmpSol, unProb, unAlgo);

// 			AfficherSolution(tmpSol, unProb, "----- test intermediaire ----- PosA : " + std::to_string(posA) + " / pos : " + std::to_string(pos), false);

			bestTmpSol = compare(tmpSol, bestTmpSol);
		}
	}

// 	AfficherSolution(bestTmpSol, unProb, "===== test final =====", false);

	return bestTmpSol;
}