/* messages FR pour mkd_make et mkd_comp */

/* commun */

char checkString[] = "  Scanner     ";
char compString[]  = "  Compiler    ";
char assString[]   = "  Assembler   ";
char linkString[]  = "  Lier        ";

char nCfileStr[]   = " fichiers compil-s";
char nAfileStr[]   = " fichiers assembl-s";


char doCompStr[] = "Compilation";
char doAssStr[]  = "Assemblage";
char mkeStr[]    = "Construire";
char linkStr[]   = " Lier le Noyau - ";

char compStr[] = " Compiler  fichier ";
char assStr[]  = " Assembler fichier ";

char errmalloc[] = " Pas assez de m-moire ";
char errStr[]  = " Erreur ... Appuyer sur une touche ";
char succStr[] = " Succ-s ... Appuyer sur une touche ";
char noCompStr[] = " Impossible de lancer le compilateur : Voir le menu d'installation ";
char noAssStr[]  = " Impossible de lancer l'assembleur : Voir le menu d'installation ";

char checkDepStr[] = " V-rifier les d-pendances ";
char tempStr[]     = " Impossible de cr-er fichier temporaire ";
char mmodStr[]     = " Module multit-che construit ... ";

/* mkd_make */
char noPrjStr[]    = " Pas de fichier de projet - Lancer le menu PROJET ";
char noPrjFileStr[]= " Fichier de projet introuvable - Lancer le menu PROJET ";

char syntErr1[]    = " (%d) : marqueur de fin de relation ';' absent ";
char syntErr2[]    = " (%d) : op-rateur de relation ':' dupliqu- ";
char syntErr3[]    = " (%d) : Erreur de syntaxe - ':' absent ";
char chkStr[]      = " V-rifier ";
char fileNfStr[]   = " (%d) : fichier %s introuvable ";
char fileNsrcStr[] = " (%d) : fichier source absent ";
char macNfStr[]    = " (%d) : pseudo dep. %s introuvable ";
char fileUpDStr[]  = " fichier(s) - jour ... ";
char undefMacStr[] = " (%d) : %s reference une pseudo dep. non d-finie : %s ";
char redefMacStr[] = " (%d) : %s red-finition de pseudo dep. ";
char tooMaMacStr[] = " (%d) : Trop de pseudo dependance ";
char cannotCrMac[] = " (%d) : %s introuvable - impossible de cr-er pseudo dep. ";
char breakMake[]   = " Construction interrompue ";
char noUsrStr[]    = " Pas de r-pertoire utilisateur - Voir le menu d'installation ";
char multitaskStr[]= " Fusion du Noyau avec ";

/* mkd_link */
char doLinkStr[] = " Lier ";
char noLinkStr[] = " Impossible de lancer le linkeur : Voir le menu d'installation ";
char notFoundStr[] = " : fichier introuvable ";

/* mkd_ref */
char DbaseUpd[] = " Base de r-f-rences - jour ... ";
char breakRef[]   = " Cr-ation des r-f-rences interrompue ";
char dbaseStr[] = " La base de r-f-rences a -t- cr-ee ";
char refStr[]   = " Cr-er R-f-rences ";
char lnkDBstr[] = " Lier r-f-rences ";
char mkeDBstr[] = " Cr-er la base ";
char recDBfiles[] = " Enregistrement de la Base ";
char scanStr[]  = " Scanner ";
char bldStr[]   = " Cr-er   ";
char dbStr[]    = " G-n-ration de la Base ";
char outofmem[] = " Pas assez de m-moire pour cr-er la base ";

char dbaseErrStr[]  = " Echec dans la cr-ation de la base ";
char dbaseFerrStr[] = " Erreur Cr-ation fichier de la base ";
char dbaseOpenErr[] = " %s : Echec ouverture ";
char dbaseIntegrityErr[] = " Erreur d'int-grit- ";
char generateStr[] = "  G-n-rer     ";
char dbaseLnkStr[] = "  Lier        ";
char dbaseFunc[]   = "  Fonction    ";

char nDefStr[]  = " Declarations ";
char nExtStr[]  = " Ref externes ";
char creaDBstr[] = " Cr-ation des fichiers de la BASE ";

/* REFCALL */
char refCallOpenf[] = " ouverture fichier %s impossible ";
char refCallIncorr[]= " %s : Nom de fichier incorrect ";
char refCallwkf[]   = " impossible de cr-er fichier de travail ";
char refCallOver[]  = " trop de fonctions ";