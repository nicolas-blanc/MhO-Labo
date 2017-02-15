#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"TabouDLL.lib")  
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (DescenteDLL.dll et DescenteDLL.lib) doivent 
//se trouver dans le répertoire courant du projet pour une exécution à l'aide du compilateur. Indiquer les
//arguments du programme dans les propriétés du projet - débogage - arguements.
//Sinon, utiliser le répertoire execution.

//Permet de calculer le pas d'avancée maximale lors d'une sélection d'une position
#define FRACTION 3
// Nombre de tests d'insertion avant la position actuelle
#define NBTESTSBEFORE 3
// Nombre de tests d'insertion après la position actuelle
#define NBTESTSAFTER 3

// Taille de la liste de tabous
#define MAX_TABOU 7

/* Définition du mode de débug:
0 : Mode d'affichage minimum avec solution finale
1 : Mode d'affichage avec affichage des solutions
2 : Mode d'affichage minimum sans exécution bloquante ni affichage de solution
*/
#define DEBUGMODE 0

/* Définition du mode de descente:
0 : Echange
1 : Insertion
*/
#define CLIMBINGTYPE 1

//*****************************************************************************************
// Prototype des fonctions se trouvant dans la DLL 
//*****************************************************************************************
//DESCRIPTION:	Lecture du Fichier probleme et initialiation de la structure Problem
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TTabou &unTabou);

//DESCRIPTION:	Fonction d'affichage à l'écran permettant de voir si les données du fichier problème ont été lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION: Affichage d'une solution a l'écran pour validation (avec ou sans détails des calculs)
extern "C" _declspec(dllimport) void AfficherSolution(const TSolution uneSolution, TProblem unProb, std::string Titre, bool AvecCalcul);

//DESCRIPTION: Affichage à l'écran de la solution finale, du nombre d'évaluations effectuées et de certains paramètres
extern "C" _declspec(dllimport) void AfficherResultats (const TSolution uneSol, TProblem unProb, TTabou unTabou);

//DESCRIPTION: Affichage dans un fichier(en append) de la solution finale, du nombre d'évaluations effectuées et de certains paramètres
extern "C" _declspec(dllimport) void AfficherResultatsFichier (const TSolution uneSol, TProblem unProb, TTabou unTabou, std::string FileName);

//DESCRIPTION:	Évaluation de la fonction objectif d'une solution et MAJ du compteur d'évaluation. 
//				Fonction objectif représente le retard total pondéré
extern "C" _declspec(dllimport) void EvaluerSolution(TSolution & uneSol, TProblem unProb, TTabou &unTabou);

//DESCRIPTION:	Création d'une séquence aléatoire de parcours des villes et évaluation de la fonction objectif. Allocation dynamique de mémoire
// pour la séquence (.Seq)
extern "C" _declspec(dllimport) void CreerSolutionAleatoire(TSolution & uneSolution, TProblem unProb, TTabou &unTabou);

//DESCRIPTION: Copie de la séquence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retournée.
extern "C" _declspec(dllimport) void CopierSolution (const TSolution uneSol, TSolution &Copie, TProblem unProb);

//DESCRIPTION:	Libération de la mémoire allouée dynamiquement
extern "C" _declspec(dllimport) void LibererMemoireFinPgm (TSolution uneCourante, TSolution uneNext, TSolution uneBest, TProblem unProb);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************

//DESCRIPTION:	Création d'une solution voisine à partir de la solution uneSol. NB:uneSol ne doit pas être modifiée
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TTabou &unTabou);

//DESCRIPTION:	 Echange de deux villes sélectionnée aléatoirement. NB:uneSol ne doit pas être modifiée
TSolution	Echange			(const TSolution uneSol, TProblem unProb, TTabou &unTabou);

//DESCRIPTION: Insertion d'une commande sélectionnée aléatoirement à différentes positions tirées également aléatoirement
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TTabou &unAlgo);

//DESCRIPTION: Recherche de la commande que l'on a déplacé grâce à l'insertion
int chercherInsertion(const TProblem unProb, const TSolution Next, const TSolution Courante);


//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{
	TSolution Courante;		//Solution active au cours des itérations
	TSolution Next;			//Solution voisine retenue à une itération
	TSolution Best;			//Meilleure solution depuis le début de l'algorithme
	TProblem LeProb;		//Définition de l'instance de problème
	TTabou LTabou;			//Définition des paramètres de l'agorithme
	string NomFichier;
		
	//**Lecture des paramètres
	NomFichier.assign(Param[1]);
	LTabou.NB_EVAL_MAX= atoi(Param[2]);

	//**Lecture du fichier de donnees
	LectureProbleme(NomFichier, LeProb, LTabou);
//	AfficherProbleme(LeProb);
	
	//**Création de la solution initiale 
	CreerSolutionAleatoire(Courante, LeProb, LTabou);
	AfficherSolution(Courante, LeProb, "SolInitiale: ", false);

	Best = Courante;

	int listeTabou[MAX_TABOU];
	for (int i = 0; i < MAX_TABOU; i++)
	{
		listeTabou[i] = -1;
	}

	do
	{
		Next = GetSolutionVoisine(Courante, LeProb, LTabou);
		//AfficherSolution(Courante, LeProb, "Courante: ", false);
		//AfficherSolution(Next, LeProb, "Next: ", false);
		int insere = chercherInsertion(LeProb, Next, Courante);
		bool estTabou = false;
		//On vérifie que l'élément déplacé de la solution retenue ne fait pas partie de la liste de tabous
		for (int i = 0; i < MAX_TABOU; i++)
		{
			if (listeTabou[i] != -1 && listeTabou[i] == insere)
				estTabou = true;
		}
		//Si Next n'est pas Tabou, ou que Next est meilleure que le Best jusqu'à présent, on remplace Courante par Next
		if (!estTabou || Next.FctObj <= Best.FctObj)
		{
				Courante = Next;
				cout << "Fct Obj Nouvelle Courante: " << Courante.FctObj << endl;
				//AfficherSolution(Courante, LeProb, "NouvelleCourante: ", false);
				//Après avoir remplacé la solution courante, on ajoute l'élément inséré à la liste de tabous
				for (int i = 0; i < MAX_TABOU; i++)
				{
					if (listeTabou[i] == -1)
					{
						listeTabou[i] = insere;
						break;
					}
				}
		}
		if (Courante.FctObj <= Best.FctObj)
			Best = Courante;
	}while (LTabou.CptEval < LTabou.NB_EVAL_MAX && Courante.FctObj!=0);

	AfficherResultats(Best, LeProb, LTabou);
	AfficherResultatsFichier(Best, LeProb, LTabou,"Resultats.txt");
	
	LibererMemoireFinPgm(Courante, Next, Best, LeProb);

	system("PAUSE");
	return 0;
}

//DESCRIPTION: Création d'une solution voisine à partir de la solution uneSol qui ne doit pas être modifiée.
//Dans cette fonction, on appel le TYPE DE VOISINAGE sélectionné + on détermine la STRATÉGIE D'ORIENTATION. 
//Ainsi, si la RÈGLE DE PIVOT nécessite l'étude de plusieurs voisins, la fonction TYPE DE VOISINAGE sera appelée plusieurs fois.
//Le TYPE DE PARCOURS dans le voisinage interviendra normalement dans la fonction TYPE DE VOISINAGE.
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TTabou &unTabou)
{
	//Type de voisinage : à indiquer (Echange 2 commandes aléatoires)
	//Parcours dans le voisinage : à indiquer	(Aleatoire)
	//Règle de pivot : à indiquer	(First-Impove)

	TSolution unVoisin;

	//Vérification du type de descente choisi

	switch (CLIMBINGTYPE)
	{
		//Type Echange
	case 0:
		unVoisin = Echange(uneSol, unProb, unTabou);
		break;
		//Type Insertion
	case 1:
		unVoisin = Insertion(uneSol, unProb, unTabou);
		break;
	default:
		//TODO : erreur
		break;
	}
	return (unVoisin);	
}

//DESCRIPTION: Echange de deux commandes sélectionnées aléatoirement
TSolution Echange (const TSolution uneSol, TProblem unProb, TTabou &unTabou)
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
	EvaluerSolution(Copie, unProb, unTabou);
	return(Copie);
}

//DESCRIPTION: Création d'une solution temporaire pour l'insertion, décale les éléments selon la position de l'élément à insérer (posA) et sa nouvelle position (pos)

TSolution CreateTempSolution(const TSolution uneSol, int posA, int pos, TProblem unProb)
{
	//Création d'une copie de la solution pour réordonner les éléments
	TSolution copie;
	CopierSolution(uneSol, copie, unProb);

	for (int i = 0; i < unProb.NbCom; i++)
	{
		//Place l'élément posA à sa nouvelle position pos
		if (i == pos)
		{
			copie.Seq[i] = uneSol.Seq[posA];
		}
		//Si l'élément i se trouve après pos et avant posA, il faut le décaler vers la position suivante (cas où pos < posA)
		else if (i > pos && i <= posA)
		{
			copie.Seq[i] = uneSol.Seq[i - 1];
		}
		//Si l'élément i se trouve avant pos et après posA, il faut le décaler vers la position précédente (cas où posA < pos)
		else if (i < pos && i >= posA)
		{
			copie.Seq[i] = uneSol.Seq[i + 1];
		}
		//Sinon, on recopie l'élément
		else
		{
			copie.Seq[i] = uneSol.Seq[i];
		}
	}


	return copie;
}

//DESCRIPTION: Comparaison de deux solutions et retourne la meilleure (objectif le plus faible)
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

//DESCRIPTION: Insertion d'une commande sélectionnée aléatoirement à différentes positions tirées également aléatoirement
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TTabou &unAlgo)
{
	TSolution bestTmpSol, tmpSol;
	int posA, posPred;
	//Création d'un vecteur de positions, qui indique les positions tirées aléatoirement
	vector<int> posSol;

	CopierSolution(uneSol, bestTmpSol, unProb);
	bestTmpSol.FctObj = LONG_MAX;

	//On tire aléatoirement une commande
	posA = rand() % unProb.NbCom;
	posPred = posA;

	//On tire également des positions aléatoires d'insertion avant posA à tester
	//On vérifie que la nouvelle position est correcte, et que le nombre de tests d'insertion est inférieur au maximum
	int i = 0;
	while (posPred >= 0 && i < NBTESTSBEFORE)
	{
		posPred = posPred - rand() % (unProb.NbCom / FRACTION) - 1;

		if (posPred > 0)
		{
			posSol.push_back(posPred);
		}

		i++;
	}

	//On tire également des positions aléatoires d'insertion après posA à tester
	i = 0;
	posPred = posA;
	//On vérifie que la nouvelle position est correcte, et que le nombre de tests d'insertion est inférieur au maximum
	while (posPred < unProb.NbCom && i < NBTESTSAFTER)
	{
		posPred = posPred + rand() % (unProb.NbCom / FRACTION) + 1;

		if (posPred < unProb.NbCom)
		{
			posSol.push_back(posPred);
		}

		i++;
	}

	//On vérifie pour chaque position tirée si la solution créée améliore ou non l'actuelle meilleure, et on la conserve si c'est le cas
	for each (int pos in posSol)
	{
		tmpSol = CreateTempSolution(uneSol, posA, pos, unProb);

		if (unAlgo.CptEval < unAlgo.NB_EVAL_MAX)
		{
			EvaluerSolution(tmpSol, unProb, unAlgo);
			//Affichage de la solution intermédiaire proposée
			if (DEBUGMODE == 1)
				AfficherSolution(tmpSol, unProb, "----- test intermediaire ----- PosA : " + std::to_string(posA) + " / pos : " + std::to_string(pos), false);

			bestTmpSol = compare(tmpSol, bestTmpSol);
		}
	}

	if (DEBUGMODE == 1)
		AfficherSolution(bestTmpSol, unProb, "===== test final =====", false);

	return bestTmpSol;
}


//DESCRIPTION: Recherche de la commande que l'on a déplacé grâce à l'insertion
int chercherInsertion(const TProblem unProb, const TSolution Next, const TSolution Courante)
{
	int candidat = -1;
	for (int i = 0; i < unProb.NbCom-1; i++)
		//Si les éléments comparés sont différents, on vérifie si il n'a pas été déplacé vers la gauche entre Next et Courante
		//Dans le cas où l'insertion est vers l'arrière de la séquence, on va d'abord rencontré les éléments qui auront été décalés, donc seront égaux à l'élément d'indice suivant dans Courante, et seront ignorés par la condition
		//Dans le cas où on a une insertion vers l'avant de la séquence, l'élément inséré sera le premier élément différent rencontré, 
		//Il y a alors deux cas possibles: si l'élément suivant (i+1) dans Courante est différent on est sûr qu'il s'agit de l'élément inséré car c'est le premier différent
		//Il est envisageable cependant qu'ils soient égaux, si l'insertion revient à échanger deux éléments consécutifs
		//Dans ce cas, on doit sauvegarder le premier élément qui était différent (indice i), et attendre la fin de la boucle pour être sûr d'être dans cette configuration
		if (Next.Seq[i] != Courante.Seq[i]) 
		{
			if (Next.Seq[i] != Courante.Seq[i + 1])
				return Next.Seq[i];
			if(candidat == -1)
				candidat = i;
		}
	return Next.Seq[candidat];
}