#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"TabouDLL.lib")  
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (DescenteDLL.dll et DescenteDLL.lib) doivent 
//se trouver dans le r�pertoire courant du projet pour une ex�cution � l'aide du compilateur. Indiquer les
//arguments du programme dans les propri�t�s du projet - d�bogage - arguements.
//Sinon, utiliser le r�pertoire execution.

//Permet de calculer le pas d'avanc�e maximale lors d'une s�lection d'une position
#define FRACTION 3
// Nombre de tests d'insertion avant la position actuelle
#define NBTESTSBEFORE 3
// Nombre de tests d'insertion apr�s la position actuelle
#define NBTESTSAFTER 3

// Taille de la liste de tabous
#define MAX_TABOU 7

/* D�finition du mode de d�bug:
0 : Mode d'affichage minimum avec solution finale
1 : Mode d'affichage avec affichage des solutions
2 : Mode d'affichage minimum sans ex�cution bloquante ni affichage de solution
*/
#define DEBUGMODE 0

/* D�finition du mode de descente:
0 : Echange
1 : Insertion
*/
#define CLIMBINGTYPE 1

//*****************************************************************************************
// Prototype des fonctions se trouvant dans la DLL 
//*****************************************************************************************
//DESCRIPTION:	Lecture du Fichier probleme et initialiation de la structure Problem
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TTabou &unTabou);

//DESCRIPTION:	Fonction d'affichage � l'�cran permettant de voir si les donn�es du fichier probl�me ont �t� lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION: Affichage d'une solution a l'�cran pour validation (avec ou sans d�tails des calculs)
extern "C" _declspec(dllimport) void AfficherSolution(const TSolution uneSolution, TProblem unProb, std::string Titre, bool AvecCalcul);

//DESCRIPTION: Affichage � l'�cran de la solution finale, du nombre d'�valuations effectu�es et de certains param�tres
extern "C" _declspec(dllimport) void AfficherResultats (const TSolution uneSol, TProblem unProb, TTabou unTabou);

//DESCRIPTION: Affichage dans un fichier(en append) de la solution finale, du nombre d'�valuations effectu�es et de certains param�tres
extern "C" _declspec(dllimport) void AfficherResultatsFichier (const TSolution uneSol, TProblem unProb, TTabou unTabou, std::string FileName);

//DESCRIPTION:	�valuation de la fonction objectif d'une solution et MAJ du compteur d'�valuation. 
//				Fonction objectif repr�sente le retard total pond�r�
extern "C" _declspec(dllimport) void EvaluerSolution(TSolution & uneSol, TProblem unProb, TTabou &unTabou);

//DESCRIPTION:	Cr�ation d'une s�quence al�atoire de parcours des villes et �valuation de la fonction objectif. Allocation dynamique de m�moire
// pour la s�quence (.Seq)
extern "C" _declspec(dllimport) void CreerSolutionAleatoire(TSolution & uneSolution, TProblem unProb, TTabou &unTabou);

//DESCRIPTION: Copie de la s�quence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retourn�e.
extern "C" _declspec(dllimport) void CopierSolution (const TSolution uneSol, TSolution &Copie, TProblem unProb);

//DESCRIPTION:	Lib�ration de la m�moire allou�e dynamiquement
extern "C" _declspec(dllimport) void LibererMemoireFinPgm (TSolution uneCourante, TSolution uneNext, TSolution uneBest, TProblem unProb);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************

//DESCRIPTION:	Cr�ation d'une solution voisine � partir de la solution uneSol. NB:uneSol ne doit pas �tre modifi�e
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TTabou &unTabou);

//DESCRIPTION:	 Echange de deux villes s�lectionn�e al�atoirement. NB:uneSol ne doit pas �tre modifi�e
TSolution	Echange			(const TSolution uneSol, TProblem unProb, TTabou &unTabou);

//DESCRIPTION: Insertion d'une commande s�lectionn�e al�atoirement � diff�rentes positions tir�es �galement al�atoirement
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TTabou &unAlgo);

//DESCRIPTION: Recherche de la commande que l'on a d�plac� gr�ce � l'insertion
int chercherInsertion(const TProblem unProb, const TSolution Next, const TSolution Courante);


//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{
	TSolution Courante;		//Solution active au cours des it�rations
	TSolution Next;			//Solution voisine retenue � une it�ration
	TSolution Best;			//Meilleure solution depuis le d�but de l'algorithme
	TProblem LeProb;		//D�finition de l'instance de probl�me
	TTabou LTabou;			//D�finition des param�tres de l'agorithme
	string NomFichier;
		
	//**Lecture des param�tres
	NomFichier.assign(Param[1]);
	LTabou.NB_EVAL_MAX= atoi(Param[2]);

	//**Lecture du fichier de donnees
	LectureProbleme(NomFichier, LeProb, LTabou);
//	AfficherProbleme(LeProb);
	
	//**Cr�ation de la solution initiale 
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
		//On v�rifie que l'�l�ment d�plac� de la solution retenue ne fait pas partie de la liste de tabous
		for (int i = 0; i < MAX_TABOU; i++)
		{
			if (listeTabou[i] != -1 && listeTabou[i] == insere)
				estTabou = true;
		}
		//Si Next n'est pas Tabou, ou que Next est meilleure que le Best jusqu'� pr�sent, on remplace Courante par Next
		if (!estTabou || Next.FctObj <= Best.FctObj)
		{
				Courante = Next;
				cout << "Fct Obj Nouvelle Courante: " << Courante.FctObj << endl;
				//AfficherSolution(Courante, LeProb, "NouvelleCourante: ", false);
				//Apr�s avoir remplac� la solution courante, on ajoute l'�l�ment ins�r� � la liste de tabous
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

//DESCRIPTION: Cr�ation d'une solution voisine � partir de la solution uneSol qui ne doit pas �tre modifi�e.
//Dans cette fonction, on appel le TYPE DE VOISINAGE s�lectionn� + on d�termine la STRAT�GIE D'ORIENTATION. 
//Ainsi, si la R�GLE DE PIVOT n�cessite l'�tude de plusieurs voisins, la fonction TYPE DE VOISINAGE sera appel�e plusieurs fois.
//Le TYPE DE PARCOURS dans le voisinage interviendra normalement dans la fonction TYPE DE VOISINAGE.
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TTabou &unTabou)
{
	//Type de voisinage : � indiquer (Echange 2 commandes al�atoires)
	//Parcours dans le voisinage : � indiquer	(Aleatoire)
	//R�gle de pivot : � indiquer	(First-Impove)

	TSolution unVoisin;

	//V�rification du type de descente choisi

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

//DESCRIPTION: Echange de deux commandes s�lectionn�es al�atoirement
TSolution Echange (const TSolution uneSol, TProblem unProb, TTabou &unTabou)
{
	TSolution Copie;
	int PosA, PosB, Tmp;

	//Utilisation d'une nouvelle TSolution pour ne pas modifier La solution courante (uneSol)
	CopierSolution(uneSol, Copie, unProb);
	
	//Tirage al�atoire des 2 commandes
	PosA = rand() % unProb.NbCom;
	do
	{
		PosB = rand() % unProb.NbCom;
	}while (PosA == PosB); //Validation pour ne pas consommer une �valuation inutilement
	
	//Echange des 2 commandes
	Tmp = Copie.Seq[PosA];
	Copie.Seq[PosA] = Copie.Seq[PosB];
	Copie.Seq[PosB] = Tmp;
	
	//Le nouveau voisin doit �tre �valu� 
	EvaluerSolution(Copie, unProb, unTabou);
	return(Copie);
}

//DESCRIPTION: Cr�ation d'une solution temporaire pour l'insertion, d�cale les �l�ments selon la position de l'�l�ment � ins�rer (posA) et sa nouvelle position (pos)

TSolution CreateTempSolution(const TSolution uneSol, int posA, int pos, TProblem unProb)
{
	//Cr�ation d'une copie de la solution pour r�ordonner les �l�ments
	TSolution copie;
	CopierSolution(uneSol, copie, unProb);

	for (int i = 0; i < unProb.NbCom; i++)
	{
		//Place l'�l�ment posA � sa nouvelle position pos
		if (i == pos)
		{
			copie.Seq[i] = uneSol.Seq[posA];
		}
		//Si l'�l�ment i se trouve apr�s pos et avant posA, il faut le d�caler vers la position suivante (cas o� pos < posA)
		else if (i > pos && i <= posA)
		{
			copie.Seq[i] = uneSol.Seq[i - 1];
		}
		//Si l'�l�ment i se trouve avant pos et apr�s posA, il faut le d�caler vers la position pr�c�dente (cas o� posA < pos)
		else if (i < pos && i >= posA)
		{
			copie.Seq[i] = uneSol.Seq[i + 1];
		}
		//Sinon, on recopie l'�l�ment
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

//DESCRIPTION: Insertion d'une commande s�lectionn�e al�atoirement � diff�rentes positions tir�es �galement al�atoirement
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TTabou &unAlgo)
{
	TSolution bestTmpSol, tmpSol;
	int posA, posPred;
	//Cr�ation d'un vecteur de positions, qui indique les positions tir�es al�atoirement
	vector<int> posSol;

	CopierSolution(uneSol, bestTmpSol, unProb);
	bestTmpSol.FctObj = LONG_MAX;

	//On tire al�atoirement une commande
	posA = rand() % unProb.NbCom;
	posPred = posA;

	//On tire �galement des positions al�atoires d'insertion avant posA � tester
	//On v�rifie que la nouvelle position est correcte, et que le nombre de tests d'insertion est inf�rieur au maximum
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

	//On tire �galement des positions al�atoires d'insertion apr�s posA � tester
	i = 0;
	posPred = posA;
	//On v�rifie que la nouvelle position est correcte, et que le nombre de tests d'insertion est inf�rieur au maximum
	while (posPred < unProb.NbCom && i < NBTESTSAFTER)
	{
		posPred = posPred + rand() % (unProb.NbCom / FRACTION) + 1;

		if (posPred < unProb.NbCom)
		{
			posSol.push_back(posPred);
		}

		i++;
	}

	//On v�rifie pour chaque position tir�e si la solution cr��e am�liore ou non l'actuelle meilleure, et on la conserve si c'est le cas
	for each (int pos in posSol)
	{
		tmpSol = CreateTempSolution(uneSol, posA, pos, unProb);

		if (unAlgo.CptEval < unAlgo.NB_EVAL_MAX)
		{
			EvaluerSolution(tmpSol, unProb, unAlgo);
			//Affichage de la solution interm�diaire propos�e
			if (DEBUGMODE == 1)
				AfficherSolution(tmpSol, unProb, "----- test intermediaire ----- PosA : " + std::to_string(posA) + " / pos : " + std::to_string(pos), false);

			bestTmpSol = compare(tmpSol, bestTmpSol);
		}
	}

	if (DEBUGMODE == 1)
		AfficherSolution(bestTmpSol, unProb, "===== test final =====", false);

	return bestTmpSol;
}


//DESCRIPTION: Recherche de la commande que l'on a d�plac� gr�ce � l'insertion
int chercherInsertion(const TProblem unProb, const TSolution Next, const TSolution Courante)
{
	int candidat = -1;
	for (int i = 0; i < unProb.NbCom-1; i++)
		//Si les �l�ments compar�s sont diff�rents, on v�rifie si il n'a pas �t� d�plac� vers la gauche entre Next et Courante
		//Dans le cas o� l'insertion est vers l'arri�re de la s�quence, on va d'abord rencontr� les �l�ments qui auront �t� d�cal�s, donc seront �gaux � l'�l�ment d'indice suivant dans Courante, et seront ignor�s par la condition
		//Dans le cas o� on a une insertion vers l'avant de la s�quence, l'�l�ment ins�r� sera le premier �l�ment diff�rent rencontr�, 
		//Il y a alors deux cas possibles: si l'�l�ment suivant (i+1) dans Courante est diff�rent on est s�r qu'il s'agit de l'�l�ment ins�r� car c'est le premier diff�rent
		//Il est envisageable cependant qu'ils soient �gaux, si l'insertion revient � �changer deux �l�ments cons�cutifs
		//Dans ce cas, on doit sauvegarder le premier �l�ment qui �tait diff�rent (indice i), et attendre la fin de la boucle pour �tre s�r d'�tre dans cette configuration
		if (Next.Seq[i] != Courante.Seq[i]) 
		{
			if (Next.Seq[i] != Courante.Seq[i + 1])
				return Next.Seq[i];
			if(candidat == -1)
				candidat = i;
		}
	return Next.Seq[candidat];
}