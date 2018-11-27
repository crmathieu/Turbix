#define  C8255    0x61    /* 8255 port B */

#define KEYBD             0x60
#define PORT_B            0x61
#define KBIT              0x80
unsigned kbd_state;	  	  /* keyboard state */

/*--------------------------------------------------------------------------*
 * keyboard management: the 8048 kbd controller triggers an interrupt       *
 * when a key is pressed or when a key is released                          *
 *     key pressed : the controller sends the key code                      *
 *     key released: the controller sends the same code with the 8th bit    *
 *                   value forced to 1                                      *
 *--------------------------------------------------------------------------*
 */
INTERRUPT _keyboard()
{
    int code,val;

    code = inp(KEYBD);           /* read code sent by keyboard */
    val  = inp(C8255);           /* read 8255 config port */

    outp( C8255,( val | 0x80));  /* send ACK to keyboard controller */
	outp(C8255, val);			 /* restore 8255 config port */

	if (code < 127) {  
		/* key is pressed: branch according to current state */
		switch(kbd_state) {
			case KBDCTRL   :
			case KBDCTRALT :
			case KBDALT    :
			case KBDALTCTRL:
			case KBDSHIFT  :
			default        :
		}
    } else {
		/* key is released */
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