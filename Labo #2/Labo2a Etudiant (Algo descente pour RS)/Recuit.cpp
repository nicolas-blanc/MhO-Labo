#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"RecuitDLL.lib")  
//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (RecuitDLL.dll et RecuitDLL.lib) doivent 
//se trouver dans le r�pertoire courant du projet pour une ex�cution � l'aide du compilateur. Indiquer les
//arguments du programme dans les propri�t�s du projet - d�bogage - arguements.
//Sinon, utiliser le r�pertoire execution.

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
TSolution	Echange(const TSolution uneSol, TProblem unProb, TRecuit &unRecuit);

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
		
	//**Lecture des param�tres
	NomFichier.assign(Param[1]);
	LeRecuit.TempInit	= atoi(Param[2]);
	LeRecuit.Alpha		= atof(Param[3]);
	LeRecuit.NbPalier	= atoi(Param[4]);
	LeRecuit.NB_EVAL_MAX= atoi(Param[5]);

	//**Lecture du fichier de donnees
	LectureProbleme(NomFichier, LeProb, LeRecuit);
	//AfficherProbleme(LeProb);
	
	//**Cr�ation de la solution initiale 
	CreerSolutionAleatoire(Courante, LeProb, LeRecuit);
	AfficherSolution(Courante, LeProb, "SolInitiale: ", false);

	LeRecuit.NoPalier = 0;
	do
	{
		LeRecuit.NoPalier ++; //Non utilis� pr�sentement
		Next = GetSolutionVoisine(Courante, LeProb, LeRecuit);
		//AfficherSolution(Courante, LeProb, "Courante: ", false);
		//AfficherSolution(Next, LeProb, "Next: ", false);
		LeRecuit.Delta = Next.FctObj - Courante.FctObj;
		if (LeRecuit.Delta <= 0)	//**am�lioration
		{
				Courante = Next;
				cout << "Fct Obj Nouvelle Courante: " << Courante.FctObj << endl;
				//AfficherSolution(Courante, LeProb, "NouvelleCourante: ", false);
		}
	}while (LeRecuit.CptEval < LeRecuit.NB_EVAL_MAX);

	AfficherResultats(Courante, LeProb, LeRecuit);
	AfficherResultatsFichier(Courante, LeProb, LeRecuit,"Resultats.txt");
	
	LibererMemoireFinPgm(Courante, Next, Best, LeProb);

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

	unVoisin = Echange(uneSol, unProb, unRecuit);
	return (unVoisin);
}

//DESCRIPTION: Echange de deux commandes s�lectionn�es al�atoirement
TSolution Echange(const TSolution uneSol, TProblem unProb, TRecuit &unRecuit)
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
	EvaluerSolution(Copie, unProb, unRecuit);
	return(Copie);
}