#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"RecuitDLL.lib")  
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (RecuitDLL.dll et RecuitDLL.lib) doivent 
//se trouver dans le répertoire courant du projet pour une exécution à l'aide du compilateur. Indiquer les
//arguments du programme dans les propriétés du projet - débogage - arguements.
//Sinon, utiliser le répertoire execution.

//Permet de calculer le pas d'avancée maximale lors d'une sélection d'une position
#define FRACTION 3
// Nombre de tests d'insertion avant la position actuelle
#define NBTESTSBEFORE 3
// Nombre de tests d'insertion après la position actuelle
#define NBTESTSAFTER 3

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
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TRecuit &unRecuit);

//DESCRIPTION:	Fonction d'affichage à l'écran permettant de voir si les données du fichier problème ont été lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION: Affichage d'une solution a l'écran pour validation (avec ou sans détails des calculs)
extern "C" _declspec(dllimport) void AfficherSolution(const TSolution uneSolution, TProblem unProb, std::string Titre, bool AvecCalcul);

//DESCRIPTION: Affichage à l'écran de la solution finale, du nombre d'évaluations effectuées et de certains paramètres
extern "C" _declspec(dllimport) void AfficherResultats (const TSolution uneSol, TProblem unProb, TRecuit unRecuit);

//DESCRIPTION: Affichage dans un fichier(en append) de la solution finale, du nombre d'évaluations effectuées et de certains paramètres
extern "C" _declspec(dllimport) void AfficherResultatsFichier (const TSolution uneSol, TProblem unProb, TRecuit unRecuit, std::string FileName);

//DESCRIPTION:	Évaluation de la fonction objectif d'une solution et MAJ du compteur d'évaluation. 
//				Fonction objectif représente le retard total pondéré
extern "C" _declspec(dllimport) void EvaluerSolution(TSolution & uneSol, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION:	Création d'une séquence aléatoire de parcours des villes et évaluation de la fonction objectif. Allocation dynamique de mémoire
// pour la séquence (.Seq)
extern "C" _declspec(dllimport) void CreerSolutionAleatoire(TSolution & uneSolution, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION: Copie de la séquence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retournée.
extern "C" _declspec(dllimport) void CopierSolution (const TSolution uneSol, TSolution &Copie, TProblem unProb);

//DESCRIPTION:	Libération de la mémoire allouée dynamiquement
extern "C" _declspec(dllimport) void	LibererMemoireFinPgm	(TSolution uneCourante, TSolution uneNext, TSolution uneBest, TProblem unProb);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************

//DESCRIPTION: Création d'une solution voisine à partir de la solution uneSol qui ne doit pas être modifiée.
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION:	 Echange de deux villes sélectionnée aléatoirement. NB:uneSol ne doit pas être modifiée
TSolution Echange(const TSolution uneSol, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION: Insertion d'une commande sélectionnée aléatoirement à différentes positions tirées également aléatoirement
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TRecuit &unAlgo);

void writeInformation(TRecuit LeRecuit, vector<int> delta, vector<double> temp, vector<int> degradations, int nbChangeTemp, int nbAcceptDegradation, int nbCalculDelta) 
{
	ofstream f_testDelta("testDelta.csv", ios::out | ios::trunc);  // ouverture en écriture avec effacement du fichier ouvert
	if (!f_testDelta)
		cerr << "Impossible d'ouvrir le fichier !" << endl;

	f_testDelta << "t0;" << LeRecuit.TempInit << ";;Nombre Delta;" << nbCalculDelta << endl;
	f_testDelta << "alpha;" << LeRecuit.Alpha << ";;Nombre Degradation;" << nbAcceptDegradation << endl;
	f_testDelta << "nbPalier;" << LeRecuit.NbPalier << ";;Nombre Temperature;" << nbChangeTemp << endl;
	f_testDelta << "nbEvalMax;" << LeRecuit.NB_EVAL_MAX << endl;
	f_testDelta << endl;

	f_testDelta << "numInc;Delta;Temp;exp(-d/T);degradation" << endl;

	int expo = 0;
	for (int i = 0; i < delta.size(); i++)
	{
		if (delta[i] != 0)
		{
			expo = exp((-delta[i]) / temp[i]);
			f_testDelta << i << ";" << delta[i] << ";" << temp[i] << ";" << expo << ";" << degradations[i] << endl;
/*
			if (expo > 0)
			{
				f_testDelta << i << ";" << delta[i] << ";" << temp[i] << ";" << expo << ";" << degradations[i] << endl;
			} 
			else
			{
				f_testDelta << i << ";" << delta[i] << ";" << temp[i] << ";0;0" << endl;
			}
*/
		}
	}

	f_testDelta.close();
}

//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{
	TSolution Courante;		//Solution active au cours des itérations
	TSolution Next;			//Solution voisine retenue à une itération
	TSolution Best;			//Meilleure solution depuis le début de l'algorithme
	TProblem LeProb;		//Définition de l'instance de problème
	TRecuit LeRecuit;		//Définition des paramètres du recuit simulé
	string NomFichier;
		
	int nbChangeTemp = 0;
	int nbAcceptDegradation = 0;
	int nbCalculDelta = 0;
	vector<int> delta;
	vector<double> temp;
	vector<int> degradation;

	//**Lecture des paramètres
	NomFichier.assign(Param[1]);
	LeRecuit.TempInit	= atoi(Param[2]);
	LeRecuit.Alpha		= atof(Param[3]);
	LeRecuit.NbPalier	= atoi(Param[4]);
	LeRecuit.NB_EVAL_MAX= atoi(Param[5]);

	int duree = 0;

	switch (CLIMBINGTYPE)
	{
		//Type Echange
	case 0:
		duree = LeRecuit.NB_EVAL_MAX / LeRecuit.NbPalier;
		break;
		//Type Insertion
	case 1:
		duree = (LeRecuit.NB_EVAL_MAX / NBTESTSAFTER) / LeRecuit.NbPalier;
		break;
	default:
		//TODO : erreur
		break;
	}

	//**Lecture du fichier de donnees
	LectureProbleme(NomFichier, LeProb, LeRecuit);
	//AfficherProbleme(LeProb);
	
	//**Création de la solution initiale 
	CreerSolutionAleatoire(Courante, LeProb, LeRecuit);
	AfficherSolution(Courante, LeProb, "SolInitiale: ", false);

	Best = Courante;
	LeRecuit.Temperature = LeRecuit.TempInit;

	while (LeRecuit.CptEval < LeRecuit.NB_EVAL_MAX)
	{
		LeRecuit.NoPalier = 0;
		do
		{
			Next = GetSolutionVoisine(Courante, LeProb, LeRecuit);
			//AfficherSolution(Courante, LeProb, "Courante: ", false);
			//AfficherSolution(Next, LeProb, "Next: ", false);
			LeRecuit.Delta = Next.FctObj - Courante.FctObj;

			nbCalculDelta++;
			delta.push_back(LeRecuit.Delta);
			temp.push_back(LeRecuit.Temperature);
	
			if (LeRecuit.Delta <= 0)
			{
				Courante = Next;
				cout << "Fct Obj Nouvelle Courante: " << Courante.FctObj << endl;
				//AfficherSolution(Courante, LeProb, "NouvelleCourante: ", false);
				if (Courante.FctObj < Best.FctObj)
				{
					Best = Courante;
				}

				degradation.push_back(0);
			}
			else
			{
				if (rand() / double(RAND_MAX) < exp((-LeRecuit.Delta) / LeRecuit.Temperature))
				{
					Courante = Next;
					nbAcceptDegradation++;
					degradation.push_back(1);
				}
				else
				{
					degradation.push_back(0);
				}
			}
			LeRecuit.NoPalier++;
		} while (LeRecuit.NoPalier != duree && LeRecuit.CptEval < LeRecuit.NB_EVAL_MAX);

		LeRecuit.Temperature = LeRecuit.Alpha * LeRecuit.Temperature;
		nbChangeTemp++;
	}

	AfficherResultats(Best, LeProb, LeRecuit);
	AfficherResultatsFichier(Best, LeProb, LeRecuit,"Resultats.txt");
	
	LibererMemoireFinPgm(Courante, Next, Best, LeProb);

	cout << "Duree : " << duree << endl;

	cout << "Nombre de calcul du Delta : " << nbCalculDelta << endl;
	cout << "Nombre de degradation : " << nbAcceptDegradation << endl;
	cout << "Nombre de changement de Temperature : " << nbChangeTemp << endl;

	writeInformation(LeRecuit, delta, temp, degradation, nbChangeTemp, nbAcceptDegradation, nbCalculDelta);

	system("PAUSE");
	return 0;
}

//DESCRIPTION: Création d'une solution voisine à partir de la solution uneSol qui ne doit pas être modifiée.
//Dans cette fonction, on appel le TYPE DE VOISINAGE sélectionné + on détermine la STRATÉGIE D'ORIENTATION. 
//Ainsi, si la RÈGLE DE PIVOT nécessite l'étude de plusieurs voisins, la fonction TYPE DE VOISINAGE sera appelée plusieurs fois.
//Le TYPE DE PARCOURS dans le voisinage interviendra normalement dans la fonction TYPE DE VOISINAGE.
TSolution GetSolutionVoisine(const TSolution uneSol, TProblem unProb, TRecuit &unRecuit)
{
	//Type de voisinage : À MODIFIER (Echange 2 commandes aléatoires)
	//Parcours dans le voisinage : À MODIFIER	(Aleatoire)
	//Règle de pivot : À MODIFIER	(First-Impove)

	TSolution unVoisin;

	//Vérification du type de descente choisi

	switch (CLIMBINGTYPE)
	{
		//Type Echange
	case 0:
		unVoisin = Echange(uneSol, unProb, unRecuit);
		break;
		//Type Insertion
	case 1:
		unVoisin = Insertion(uneSol, unProb, unRecuit);
		break;
	default:
		//TODO : erreur
		break;
	}
	return (unVoisin);
}

//DESCRIPTION: Echange de deux commandes sélectionnées aléatoirement
TSolution Echange(const TSolution uneSol, TProblem unProb, TRecuit &unAlgo)
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
	} while (PosA == PosB); //Validation pour ne pas consommer une évaluation inutilement

							//Echange des 2 commandes
	Tmp = Copie.Seq[PosA];
	Copie.Seq[PosA] = Copie.Seq[PosB];
	Copie.Seq[PosB] = Tmp;

	//Le nouveau voisin doit être évalué 
	EvaluerSolution(Copie, unProb, unAlgo);
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
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TRecuit &unAlgo)
{
	TSolution bestTmpSol, tmpSol;
	int posA, posPred;
	//Création d'un vecteur de positions, qui indique les positions tirées aléatoirement
	//Ne sert pas ici
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

	//On vérifie pour chaque position tirée si la solution créée améliore ou non l'actuelle, et on la conserve si c'est le cas
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