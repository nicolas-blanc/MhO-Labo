#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"RecuitDLL.lib")  
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (RecuitDLL.dll et RecuitDLL.lib) doivent 
//se trouver dans le r�pertoire courant du projet pour une ex�cution � l'aide du compilateur. Indiquer les
//arguments du programme dans les propri�t�s du projet - d�bogage - arguements.
//Sinon, utiliser le r�pertoire execution.

//Permet de calculer le pas d'avanc�e maximale lors d'une s�lection d'une position
#define FRACTION 3
// Nombre de tests d'insertion avant la position actuelle
#define NBTESTSBEFORE 3
// Nombre de tests d'insertion apr�s la position actuelle
#define NBTESTSAFTER 3

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
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TRecuit &unRecuit);

//DESCRIPTION:	Fonction d'affichage � l'�cran permettant de voir si les donn�es du fichier probl�me ont �t� lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION: Affichage d'une solution a l'�cran pour validation (avec ou sans d�tails des calculs)
extern "C" _declspec(dllimport) void AfficherSolution(const TSolution uneSolution, TProblem unProb, std::string Titre, bool AvecCalcul);

//DESCRIPTION: Affichage � l'�cran de la solution finale, du nombre d'�valuations effectu�es et de certains param�tres
extern "C" _declspec(dllimport) void AfficherResultats (const TSolution uneSol, TProblem unProb, TRecuit unRecuit);

//DESCRIPTION: Affichage dans un fichier(en append) de la solution finale, du nombre d'�valuations effectu�es et de certains param�tres
extern "C" _declspec(dllimport) void AfficherResultatsFichier (const TSolution uneSol, TProblem unProb, TRecuit unRecuit, std::string FileName);

//DESCRIPTION:	�valuation de la fonction objectif d'une solution et MAJ du compteur d'�valuation. 
//				Fonction objectif repr�sente le retard total pond�r�
extern "C" _declspec(dllimport) void EvaluerSolution(TSolution & uneSol, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION:	Cr�ation d'une s�quence al�atoire de parcours des villes et �valuation de la fonction objectif. Allocation dynamique de m�moire
// pour la s�quence (.Seq)
extern "C" _declspec(dllimport) void CreerSolutionAleatoire(TSolution & uneSolution, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION: Copie de la s�quence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retourn�e.
extern "C" _declspec(dllimport) void CopierSolution (const TSolution uneSol, TSolution &Copie, TProblem unProb);

//DESCRIPTION:	Lib�ration de la m�moire allou�e dynamiquement
extern "C" _declspec(dllimport) void	LibererMemoireFinPgm	(TSolution uneCourante, TSolution uneNext, TSolution uneBest, TProblem unProb);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************

//DESCRIPTION: Cr�ation d'une solution voisine � partir de la solution uneSol qui ne doit pas �tre modifi�e.
TSolution GetSolutionVoisine (const TSolution uneSol, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION:	 Echange de deux villes s�lectionn�e al�atoirement. NB:uneSol ne doit pas �tre modifi�e
TSolution Echange(const TSolution uneSol, TProblem unProb, TRecuit &unRecuit);

//DESCRIPTION: Insertion d'une commande s�lectionn�e al�atoirement � diff�rentes positions tir�es �galement al�atoirement
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TRecuit &unAlgo);

void writeInformation(TRecuit LeRecuit, vector<int> delta, vector<double> temp, vector<int> degradations, int nbChangeTemp, int nbAcceptDegradation, int nbCalculDelta) 
{
	ofstream f_testDelta("testDelta.csv", ios::out | ios::trunc);  // ouverture en �criture avec effacement du fichier ouvert
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
	TSolution Courante;		//Solution active au cours des it�rations
	TSolution Next;			//Solution voisine retenue � une it�ration
	TSolution Best;			//Meilleure solution depuis le d�but de l'algorithme
	TProblem LeProb;		//D�finition de l'instance de probl�me
	TRecuit LeRecuit;		//D�finition des param�tres du recuit simul�
	string NomFichier;
		
	int nbChangeTemp = 0;
	int nbAcceptDegradation = 0;
	int nbCalculDelta = 0;
	vector<int> delta;
	vector<double> temp;
	vector<int> degradation;

	//**Lecture des param�tres
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
	
	//**Cr�ation de la solution initiale 
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

//DESCRIPTION: Cr�ation d'une solution voisine � partir de la solution uneSol qui ne doit pas �tre modifi�e.
//Dans cette fonction, on appel le TYPE DE VOISINAGE s�lectionn� + on d�termine la STRAT�GIE D'ORIENTATION. 
//Ainsi, si la R�GLE DE PIVOT n�cessite l'�tude de plusieurs voisins, la fonction TYPE DE VOISINAGE sera appel�e plusieurs fois.
//Le TYPE DE PARCOURS dans le voisinage interviendra normalement dans la fonction TYPE DE VOISINAGE.
TSolution GetSolutionVoisine(const TSolution uneSol, TProblem unProb, TRecuit &unRecuit)
{
	//Type de voisinage : � MODIFIER (Echange 2 commandes al�atoires)
	//Parcours dans le voisinage : � MODIFIER	(Aleatoire)
	//R�gle de pivot : � MODIFIER	(First-Impove)

	TSolution unVoisin;

	//V�rification du type de descente choisi

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

//DESCRIPTION: Echange de deux commandes s�lectionn�es al�atoirement
TSolution Echange(const TSolution uneSol, TProblem unProb, TRecuit &unAlgo)
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
	} while (PosA == PosB); //Validation pour ne pas consommer une �valuation inutilement

							//Echange des 2 commandes
	Tmp = Copie.Seq[PosA];
	Copie.Seq[PosA] = Copie.Seq[PosB];
	Copie.Seq[PosB] = Tmp;

	//Le nouveau voisin doit �tre �valu� 
	EvaluerSolution(Copie, unProb, unAlgo);
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
TSolution Insertion(const TSolution uneSol, const TProblem unProb, TRecuit &unAlgo)
{
	TSolution bestTmpSol, tmpSol;
	int posA, posPred;
	//Cr�ation d'un vecteur de positions, qui indique les positions tir�es al�atoirement
	//Ne sert pas ici
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

	//On v�rifie pour chaque position tir�e si la solution cr��e am�liore ou non l'actuelle, et on la conserve si c'est le cas
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