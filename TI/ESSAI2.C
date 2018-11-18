unsigned char deltaTab[256];
char linebuf[] = "   abbvbbbscbccbb";
char findSTR[] = "bscb";
main()
{
        boyerMoore(strlen(findSTR), 0, strlen(linebuf));
}


boyerMoore(Pi, offset, Si)
{
        int i, j, delta;

        for (i = 1; i <= Pi; i++) {
        if (deltaTab[j = findSTR[Pi-i]]) /* cara deja enregistre */
            continue;
        deltaTab[j] = i;
        }


        if ((j = Pi-1+offset) > Si-1) /* la pattern depasse la ligne */
                return(-1);

        for (i = Pi-1; j <= Si-1;) {
                if (findSTR[i] != linebuf[j]) {
                        if (delta = deltaTab[linebuf[j]])
                                /* le carac existe dans findSTR */
                                j += delta-1;
                        else
                                j += Pi;
                        i  = Pi-1;
                 }
                 else { /* egalite */
                        if (i == 0)   return(j);
                        i--;
                        j--;
                 }
        }
        return(-1);
}
