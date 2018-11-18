#define  C8255    0x61               /* port B du 8255  */

#define KEYBD             0x60
#define PORT_B            0x61
#define KBIT              0x80
unsigned kbd_state;	  /* etat du clavier */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*
 * gestion du clavier : le controleur 8048 du kbd declenche                 *
 * une it quand une touche est enfoncee ET quand une touche                 *
 * est relachee .                                                           *
 *     Touche ENFONCEE : le controleur emet le code de la touche            *
 *     Touche RELACHEE : le controleur emet le meme code avec le            *
 *                       8eme BIT forc‚ … 1                                 *
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*
 */
INTERRUPT _keyboard()
{
    int code,val;

    code = inp(KEYBD);           /* lecture du code emis par le kbd     */
    val  = inp(C8255);           /* lire le port de config du 8255      */

    outp( C8255,( val | 0x80));  /* envoyer ack au controleur du kbd    */
    outp( C8255, val );          /* restaurer le port de config du 8255 */

    if (code < 127) {  /* la touche est enfoncee */

	 switch(kbd_state) {   /* suivant l'etat se brancher au traitement
				* concern‚
				*/
	   case KBDCTRL   :
	   case KBDCTRALT :
	   case KBDALT    :
	   case KBDALTCTRL:
           case KBDSHIFT  :
	   default        :
         }
    }
    else {
			       /* la touche est relachee */
	 switch(kbd_state) {
	   case KBD       :
	   case KBDCTRL   :
           case KBDALTCTRL:
	   case KBDCTRALT :
	   case KBDALT    :
	   case KBDSHIFT  :
	   default        :
	 };
    }
}
