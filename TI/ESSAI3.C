/* help.c : gestion du help */

   int i, Erreur;

   Erreur = FALSE;
                        Erreur = TRUE;
                Erreur = TRUE;
        }

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  FIN LIB multitache */

   if (Erreur) {
       pushPop_MESS(UnknownStr, ERRMSG);
       ret_value = FUNC|F6;
       goto SKIP_HELP;
   }
