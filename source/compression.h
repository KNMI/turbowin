/* HISTORIQUE des MODIFS
	juin 2006 : 	- LGNOM passe de 64 a 256
				- Dans la structure spelem, degre, pente, origine, MNT_pente, MNT_biais
				sont declares double
	aout 2006 :	- LGNOM passe de 256 a 1024
	sept 2006 :	 - LGUNITE passe de 32 a 64
				Definition des constantes DATA_REPORT et SHORT_BURST_DATA 
	juin 2007 : 	- rajout de 2 champs dans le fichier pilote et structure spelem specel :
				MNT_mini = valeur physique minimum valide (par defaut initialisee a la valeur de specel.origine
				MNT_maxi = valeur physique maximum valide (par defaut initialisee a la valeur physique corrrespondant a specel.codemax
	nov 2007:	Definition d'un defaut (LFPW) pour la macro CENTRE
	ete 2008 : 	definition struct datad donnees[NBMAXELEM];
	avril 2009 : 	introduction de la structure "struct modlkp" donnant les types de donnees d'un modele de compression et du 
				tableau correspondant "data_types".
				limitation a MAXMODOP du nb de modes operatoires pour une station
				et a MAXNBMODL du nb de modeles de compression utilises.
	Nov 2009 :	Ajout flag Display dans struct datad
*/
#define DEBUG		0		/* Mode debugage - impreesions de controle			*/

#ifndef CENTRE
#define CENTRE "LFPW"
#endif
#define MAXLINE 	512		/* nb de carateres max des lignes  du fichier PILOTE		*/
#define LGUNITE	64			/* Longueur max du nom de l'unite				*/
#define LGNOM		1024		/* Longueur max du nom de l'element				*/
#define NBMAXELEM	1024		/* Nombre max d'elements a transmettre				*/
#define NBMETA		256		/* Nombre max de meta data					*/
#define NBMAXSPEC	16		/* Nombre max de specifications par element			*/
#define NBMAXOCT	1024		/* Nb max d'octets d'un message compresse binaire 		*/
#define MAXTXTDATA 	256		/* Nb mx de caracteres admis pour une donnee de type texte 	*/
#define MAXID		7		/* Nb max de caracteres de l'indicatif navire 			*/
#define MAXIDENT    	64     	 	/* Nb max de caracteres du champ identificateur			*/
#define TABLEIND	"VOS_ident.txt"	/* table id emetteur-indicateur					*/
#define TABLEIDFMT	"id-format.csv"	/* table id modeles de compression */
#define MAX_L_MES	69		/*Nb de carracteres max d'une ligne de message : 72 - 3 (\r\r\n)*/
#define DATA_REPORT "INMC_DR"		/* Code origine data reporting 					*/
#define SHORT_BURST_DATA_GPS "IRID_SR" 	/* Code origine sbd d'iridium avec localisation GPS		*/
#define SHORT_BURST_DATA_IRD "IRID_SI" 	/* Code origine sbd d'iridium avec localisation IRIDIUM		*/
#define DEFINT	-9999			/* Defaut pour entree d'entiers sur ligne de commande 		*/
#define DEFDBL  -999.9			/* Defaut pour entree de reels sur ligne de commande 		*/
#define ERRLOC	7			/* Erreur de localisation maximale admise pour localisation IRIDIUM (en km) */
#define DELAISLOC 86400L		/* Delais maximun de recuperation de la localisation IRIDIUM 	*/
#define MAXFORMAT 12			/* Nb de caracteres max d'une chaine format			*/
#define ADERR 234			/* Valeur mini du code erreurs repertoriees			*/
#define MAXMODOP 32			/* Nb max de modes operatoires pour une station			*/
#define MAXNBMODL 256			/* Nb max de modeles de compression utilises			*/

char REPAUX[256];			/* Repertoire par defaut contenant les donnees auxiliaires 	*/		
struct spelem 	{			/* Struture fixant les specification d'un element a comprimer	*/
    char		BUFR[8]; 	/* Numero de descripteur code BUFR				*/
    char        	ident[MAXIDENT]; /* Identificateur capteur ou autre 				*/
    unsigned char nbit;			/* nb de bits de compression					*/
    double		degre;		/* Degre (inverse) du polynome de codage			*/
    double		pente;		/* Coef multiplicateur a appliquer a la valeur physique 	*/
    double		origine;	/* Valeur de reference origine					*/
    long	    	codemax;	/* Valeur max codee						*/
    unsigned long	modeo;		/* Mode operatoire choisi					*/
    char		unite[LGUNITE];	/* Chaine donnant l'unite					*/
    double	      	MNT_pente;	/* Pente a appliquer en correction 				*/
    double	      	MNT_biais;	/* Biais a appliquer en correction				*/
    double		MNT_mini;	/* Valeur physique minimum valide (apres application pente et biais correctifs)	*/
    double	      	MNT_maxi;	/* Valeur physique maximum valide (apres ....)			*/
    unsigned char	paraOK;		/* Flag parametre a transmette					*/
    char	     	nom[LGNOM];	/* Nom du parametre						*/
    		};
struct spelem specel[NBMAXELEM];	/* Tableau des specifications de tous les elements a transmettre*/
int nbelem	;					/* Nb d'elements transmis			*/

struct meta	{
    		char 	BUFR[8]; 			/* Numero de descripteur code BUFR		*/
    		char    	ident[MAXIDENT];	/* Identificateur capteur ou autre 		*/
		char 	valeur[32];			/* Valeur de la meta data a convertir
					  				 eventuellement en numerique	*/
		char		nom[LGNOM];		/* Nom du parametre				*/
	};
struct meta meta_data[NBMETA];		/* Tableau des meta data 					*/
int nbmeta;				/* nombre de meta data 						*/
	
struct modlkp	{					/* types de donnees d'un modele de compression 	*/
		char		code[16];		/* Code du modele				*/
		char		nom[128];		/* Nom du modele				*/
		int		nb;			/* Nb de mode operatoires			*/
		int		modop[MAXMODOP];	/* Tableau contenant les modes operatoires	*/
		int		types[MAXMODOP] ;	/* Tableau contenant les codes des types bufr 	*/
		};
struct modlkp data_types[MAXNBMODL];	/* Tablaeu des types de donnees					*/
int nb_modl;				/* Nb de modeles de compression					*/
		
struct datad	{			/* Structure d'une donnee decompressee  			*/
	struct spelem specif;		/* Specifications du parametre fixee dans fichier pilote 	*/
	int 	present;		/* Flag a 1 si donnee valide,  a 0 si invalide ou absente	*/
	double	brute;			/* Valeur physique du parametre (avant monitoring)		*/
	double	super;			/* Valeur physique du parametre apres supervision		*/
	char brut_txt[MAXTXTDATA];	/* Valeur texte de la donnee brute 				*/
	char super_txt[MAXTXTDATA];	/* Valeur texte de la donnee supervisee 			*/
	char format[MAXFORMAT];		/* Format de sortie texte de la valeur 				*/
	char 	Display[8];		/* Flag affecte du rang du parametre observe (sous forme de 
					   chaine Dnnnnnn, nnnnnn= ordre specifie par le fichier pilote,
					   s'il s'agit d'une donnee observee. 
					   Flag a chaine vide si meta-data				*/
};
struct datad donnees[NBMAXELEM];	/* Tableau des donnees decompressees */
	

double ms2kt  ;					/* conversion M:s en Kt */

/* Parametres lies a la transmission IRIDIUM et recuperes sur ligne de commande*/
int 	CEPradius; 		/* -e nombre	Erreur de localisation en km (champ CEPradius du mail) */
double 	irid_lat;		/* -g nombre 	Longitude */
double 	irid_lon;		/* -k nombre  	Latitude */

char TRANS[16];			/* code "type de message emis" */
