#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>

char 	*tab_proc[] = {	"8088",
			"8086",
			"V20",
			"V30",
			"80188",
			"80186",
			"80286",
			"80386"};

double	tab_freq[] = {	141094.728179999976,
			141094.728179999976,
			37290.4545399999988,
			37290.4545399999988,
			26486.2096399999973,
			26486.2096399999973,
			25241.7228999999970,
			30014.4428999999982};

main()
{
	int typ_proc;
	double freq;

	typ_proc = get_proc();
	freq	 = (double)get_freq();
	freq	 = tab_freq[typ_proc] / freq;

	printf("processeur %s\n",tab_proc[typ_proc]);
	printf("Vitesse    %5.2f Mhz\n", freq);
}
