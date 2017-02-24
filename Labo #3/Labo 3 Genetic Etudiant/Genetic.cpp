#include <windows.h>
#include "Entete.h"
#pragma comment (lib,"GeneticDLL.lib")

//%%%%%%%%%%%%%%%%%%%%%%%%% IMPORTANT: %%%%%%%%%%%%%%%%%%%%%%%%% 
//Le fichier de probleme (.txt) et les fichiers de la DLL (GeneticDLL.dll et GeneticDLL.lib) doivent 
//se trouver dans le répertoire courant du projet pour une exécution à l'aide du compilateur. Indiquer les
//arguments du programme dans les propriétés du projet - débogage - arguements.
//Sinon, utiliser le répertoire execution.

//*****************************************************************************************
// Prototype des fonctions se trouvant dans la DLL 
//*****************************************************************************************
//DESCRIPTION:	Lecture du Fichier probleme et initialiation de la structure Problem
extern "C" _declspec(dllimport) void LectureProbleme(std::string FileName, TProblem & unProb, TGenetic &unGenetic);

//DESCRIPTION:	Fonction d'affichage à l'écran permettant de voir si les données du fichier problème ont été lues correctement
extern "C" _declspec(dllimport) void AfficherProbleme (TProblem unProb);

//DESCRIPTION:	Évaluation de la fonction objectif d'une solution et MAJ du compteur d'évaluation. 
extern "C" _declspec(dllimport) void EvaluerSolution(TIndividu & uneSol, TProblem unProb, TGenetic &unGenetic);

//DESCRIPTION: Fonction qui génére une population initiale en s'assurant d'avoir des solutions valides*/
extern "C" _declspec(dllimport)void CreerPopInitialeAleaValide(std::vector<TIndividu> & unePop, TProblem unProb, TGenetic & unGenetic);

//DESCRIPTION: Fonction qui affiche le détail des solutions (de Debut jusqu'a Fin-1) dans la population courante
extern "C" _declspec(dllimport) void AfficherSolutions(std::vector<TIndividu> unePop, int Debut, int Fin, int Iter, TProblem unProb);

//DESCRIPTION: Fonction de tri croissant des individus dans la population entre Debut et Fin-1 INCLUSIVEMENT 
extern "C" _declspec(dllimport) void TrierPopulation(std::vector<TIndividu> & unePop, int Debut, int Fin);

//DESCRIPTION: Copie de la séquence et de la fonction objectif dans une nouvelle TSolution. La nouvelle TSolution est retournée.
extern "C" _declspec(dllimport) void CopierSolution (const TIndividu uneSol, TIndividu &Copie, TProblem unProb);

//DESCRIPTION: Fonction qui réalise la MUTATION (modification aléatoire) sur une solution: Inversion de sous-séquence et échange de 2 commandes.
extern "C" _declspec(dllimport) void Mutation(TIndividu & Mutant, TProblem unProb, TGenetic & unGen);

//DESCRIPTION: Fonction de sélection d'un individu par tournoi
extern "C" _declspec(dllexport) int Selection (std::vector<TIndividu> unePop, int _Taille, TProblem unProb);

//DESCRIPTION: Fonction affichant les résultats de l'algorithme génétique
extern "C" _declspec(dllexport) void AfficherResultats (TIndividu uneBest, TProblem unProb, TGenetic unGen);

//DESCRIPTION: Fonction affichant les résultats de l'algorithme génétique dans un fichier texte
extern "C" _declspec(dllexport) void AfficherResultatsFichier (TIndividu uneBest, TProblem unProb, TGenetic unGen, std::string FileName);

//DESCRIPTION:	Libération de la mémoire allouée dynamiquement
extern "C" _declspec(dllexport) void LibererMemoireFinPgm (std::vector<TIndividu> & unePop, std::vector<TIndividu> & unePopEnfant, TIndividu & uneBest, TProblem & unProb, TGenetic unGen);

//*****************************************************************************************
// Prototype des fonctions locales 
//*****************************************************************************************
TIndividu Croisement(TIndividu Parent1, TIndividu Parent2, TProblem unProb, TGenetic & unGen);
void Remplacement(std::vector<TIndividu> & Parents, std::vector<TIndividu> Enfants, TProblem unProb, TGenetic unGen);

//******************************************************************************************
// Fonction main
//*****************************************************************************************
int main(int NbParam, char *Param[])
{

	TProblem LeProb;					//**Définition de l'instance de problème
	TGenetic LeGenetic;					//**Définition des paramètres du recuit simulé
	std::vector<TIndividu> Pop;			//**Population composée de Taille_Pop Individus 
	std::vector<TIndividu> PopEnfant;	//**Population d'enfant
	TIndividu Best;						//**Meilleure solution depuis le début de l'algorithme
	
	int Pere, Mere;						//**Indices de solution des parents
	int i;
	double Alea;
	
	string NomFichier;
	
	//**Lecture des paramètres
	NomFichier.assign(Param[1]);
	LeGenetic.TaillePop		= atoi(Param[2]);
	LeGenetic.ProbCr		= atof(Param[3]);
	LeGenetic.ProbMut		= atof(Param[4]);
	LeGenetic.NB_EVAL_MAX	= atoi(Param[5]);
	LeGenetic.TaillePopEnfant	= (int)ceil(LeGenetic.ProbCr * LeGenetic.TaillePop);
	LeGenetic.Gen = 0;

	//**Définition de la dimension des tableaux
	Pop.resize(LeGenetic.TaillePop);				//**Le tableau utilise les indices de 0 à TaillePop-1.
	PopEnfant.resize(LeGenetic.TaillePopEnfant);	//**Le tableau utilise les indices de 0 à TaillePopEnfant-1

	//**Lecture du fichier de donnees
	LectureProbleme(NomFichier, LeProb, LeGenetic);
	AfficherProbleme(LeProb);

	//**Initialisation de la population initiale NB: Initialisation de la population entraine des evaluation de solutions.
	//**CptEval est donc = TaillePop au retour de la fonction.
	CreerPopInitialeAleaValide(Pop, LeProb, LeGenetic);
	//AfficherSolutions(Pop, 0, LeGenetic.TaillePop, LeGenetic.Gen, LeProb);
	
	//**Tri de la population
	TrierPopulation(Pop, 0, LeGenetic.TaillePop);
	AfficherSolutions(Pop, 0, LeGenetic.TaillePop, LeGenetic.Gen, LeProb);

	//**Initialisation de de la meilleure solution
	CopierSolution(Pop[0], Best, LeProb);
	cout << endl << "Meilleure solution de la population initiale: " << Best.FctObj << endl << endl;  //**NE PAS ENLEVER
	
	//**Boucle principale de l'algorithme génétique
	do 
	{
		LeGenetic.Gen ++;
		//**Sélection et croisement
		for (i=0; i<LeGenetic.TaillePopEnfant; i++)
		{
			//**SÉLECTION de deux parents
			Pere = Selection(Pop, LeGenetic.TaillePop, LeProb);
			Mere = Selection(Pop, LeGenetic.TaillePop, LeProb);
			
			//**CROISEMENT entre les deux parents. Création d'UN enfant.
			PopEnfant[i] = Croisement(Pop[Pere], Pop[Mere], LeProb, LeGenetic);

			//**MUTATION d'une solution
			Alea = double(rand()) / double(RAND_MAX);
			if (Alea < LeGenetic.ProbMut)
			{
				//Vérification pour ne pas perdre la meilleure solution connue avant mutation
				if (Best.FctObj > PopEnfant[i].FctObj) 
					CopierSolution(PopEnfant[i], Best, LeProb);
				Mutation(PopEnfant[i], LeProb, LeGenetic);
			}
		}
		//AfficherSolutions(PopEnfant, 0, LeGenetic.TaillePopEnfant, LeGenetic.Gen, LeProb);
		
		//**REMPLACEMENT de la population pour la prochaine génération
		Remplacement(Pop, PopEnfant, LeProb, LeGenetic);
// 		AfficherSolutions(Pop, 0, LeGenetic.TaillePop, LeGenetic.Gen, LeProb);

		//**Conservation de la meilleure solution
		TrierPopulation(Pop, 0, LeGenetic.TaillePop);
		if (Best.FctObj > Pop[0].FctObj)				//**NE PAS ENLEVER
			CopierSolution(Pop[0], Best, LeProb);
// 		cout << "Meilleure solution trouvee (Generation# "<< LeGenetic.Gen << "): " << Best.FctObj << endl;

	} while (LeGenetic.CptEval < LeGenetic.NB_EVAL_MAX);	//**NE PAS ENLEVER

	AfficherResultats (Best, LeProb, LeGenetic);		//**NE PAS ENLEVER
	AfficherResultatsFichier (Best, LeProb, LeGenetic, "Resutats.txt");
	
	LibererMemoireFinPgm(Pop, PopEnfant, Best, LeProb, LeGenetic);

	system("PAUSE");
	return 0;
}

std::queue<int> initQueue(int cut, TIndividu Parent1, TIndividu Parent2, TProblem unProb)
{
	std::queue<int> q;
	std::vector<int> v;

	for (auto i = 0; i < cut; i++)
	{
		v.push_back(Parent1.Seq[i]);
	}
	v.push_back(Parent1.Seq[unProb.NbVilles - 1]);

	for (auto i = 0; i < unProb.NbVilles; i++)
	{
		if (!(std::find(v.begin(), v.end(), Parent2.Seq[i]) != v.end()))
		{
			q.push(Parent2.Seq[i]);
		}
	}

	return q;
}

TIndividu initEnfant(TIndividu Parent1, int cut, TProblem unProb)
{
	TIndividu Enfant;

	CopierSolution(Parent1, Enfant, unProb);

	for (auto i = cut; i < unProb.NbVilles - 1; i++)
	{
		Enfant.Seq[i] = -1;
	}

	return Enfant;
}

//******************************************************************************************************
//**Fonction qui réalise le CROISEMENT (échange de genes) entre deux parents. Retourne l'enfant produit.
//******************************************************************************************************
//**A DÉFINIR PAR L'ÉTUDIANT****************************************************************************
//**NB: IL FAUT RESPECTER LA DEFINITION DES PARAMÈTRES AINSI QUE LE RETOUR DE LA FONCTION
//****************************************************************************************************** 
TIndividu Croisement(TIndividu Parent1, TIndividu Parent2, TProblem unProb, TGenetic & unGen)
{
	//**INDICE: Le sous-programme rand() génère aléatoirement un nombre entier entre 0 et RAND_MAX (i.e., 32767) inclusivement.
	//**Pour tirer un nombre aléatoire entier entre 0 et MAX-1 inclusivement, il suffit d'utiliser l'instruction suivante : NombreAleatoire = rand() % MAX;
	
	// 	CopierSolution(Parent1, Enfant, unProb);
	TIndividu Enfant;
	std::queue<int> villeNonVisite;

	int cut = (rand() % (unProb.NbVilles - 3)) + 1;
	Enfant = initEnfant(Parent1, cut, unProb);

	villeNonVisite = initQueue(cut, Parent1, Parent2, unProb);

	std::queue<int> cp_villeNonVisite;
	cp_villeNonVisite = villeNonVisite;

	int i = cut;
	while (!villeNonVisite.empty())
	{
		int v = villeNonVisite.front();
		villeNonVisite.pop();

		Enfant.Seq[i] = v;
		i++;
	}

	//**NE PAS ENLEVER
	EvaluerSolution(Enfant, unProb, unGen);

	if (!Enfant.Valide)
	{
		throw std::logic_error("L'enfant n'est pas valide.");
	}

	return (Enfant);
}

//*******************************************************************************************************
//Fonction qui réalise le REMPLACEMENT de la population pour la prochaine génération. Cette fonction doit
//prendre les TaillePop solutions de la population "Parents" et les TaillePopEnfant solutions de la 
//population "Enfants" et retenir SEULEMENT TaillePop solutions pour commencer la prochaine génération. 
//Les TaillePop solutions retenues doivent être placées dans la populations "Parents".  
//*******************************************************************************************************
//**A DÉFINIR PAR L'ÉTUDIANT*****************************************************************************
//**NB: IL FAUT RESPECTER LA DEFINITION DES PARAMÈTRES
//******************************************************************************************************* 
void Remplacement(std::vector<TIndividu> & Parents, std::vector<TIndividu> Enfants, TProblem unProb, TGenetic unGen)
{
	//**Déclaration et dimension dynamique d'une population temporaire pour contenir tous les parents et les enfants
	//std::vector<TIndividu> Temporaire;
	//Temporaire.resize(unGen.TaillePop + unGen.TaillePopEnfant);
	
	//**Pour trier toute la population temporaire, il suffit de faire l'appel suivant: TrierPopulation(Temporaire, 0, unGen.TaillePop+unGen.TaillePopEnfant);
	//**Pour copier une solution de Parents dans Temporaire, il suffit de faire l'appel suivant: CopierSolution(Parents[i], Temporaire[i], unProb);

	//**Libération de la population temporaire
	//for(i=0; i< unGen.TaillePop; i++)
	//	Temporaire[i].Seq.clear();
	//Temporaire.clear();

	std::vector<TIndividu> Temporaire;
	Temporaire.resize(unGen.TaillePop);

	int tPop = unGen.TaillePop;
	int tPopEnfant = unGen.TaillePopEnfant;

	int a, b;
	std::vector<TIndividu>* v1;
	std::vector<TIndividu>* v2;

	for (auto i = 0; i < unGen.TaillePop; i++)
	{
		tPop = Parents.size();
		tPopEnfant = Enfants.size();

		switch (rand()%3)
		{
		case 0:
			a = rand() % tPop;
			b = rand() % tPopEnfant;

			v1 = &Parents;
			v2 = &Enfants;
			break;
		case 1:
			a = rand() % tPop;
			b = rand() % tPop;

			v1 = &Parents;
			v2 = &Parents;
			break;
		case 2:
			a = rand() % tPopEnfant;
			b = rand() % tPopEnfant;

			v1 = &Enfants;
			v2 = &Enfants;
			break;
		}

		if (v1->at(a).FctObj < v2->at(b).FctObj)
		{
			CopierSolution(v1->at(a), Temporaire[i], unProb);
			v1->erase(v1->begin() + a);
		} 
		else
		{
			CopierSolution(v2->at(b), Temporaire[i], unProb);
			v2->erase(v2->begin() + b);
		}
	}

	Parents.resize(unGen.TaillePop);
	for (auto i = 0; i < unGen.TaillePop; i++)
	{
		CopierSolution(Temporaire[i], Parents[i], unProb);
	}
}