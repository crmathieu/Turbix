/* subprn */

/* sous programmes de gestion d'affichage selon les formats */

/*-----------------------------------------------------------------------------
 * _decimalString - ecrire au format decimal dans la string de travail
 *-----------------------------------------------------------------------------
 */
_decimalString(num, str)
unsigned num;
char *str;
{
    int i;
    char c, temp[6];

    temp[0] = '\0';
    for (i = 1; i <= 5; i++)
    {
         temp[i] = num % 10 + '0';
         num /= 10;
    }

    for  (i = 5; temp[i] == '0'; i--);
    if   (i == 0)   i++;
    while(i >= 0)  *str++ = temp[i--];
}

/*-----------------------------------------------------------------------------
 * _longDecimalString - ecrire au format decimal long dans la string de travail
 *-----------------------------------------------------------------------------
 */
_longDecimalString(num, str)
unsigned long num;
char *str;
{
    int i;
    char c, temp[11];

    temp[0] = '\0';
    for (i = 1; i <= 10; i++)
    {
         temp[i] = num % 10 + '0';
         num /= 10;
    }

    for  (i = 10; temp[i] == '0'; i--);
    if   (i == 0)       i++;
    while(i >= 0)     *str++ = temp[i--];
}

/*-----------------------------------------------------------------------------
 * _octalString - ecrire au format octal dans la string de travail
 *-----------------------------------------------------------------------------
 */
_octalString(num, str)
unsigned num;
char *str;
{
    int i;
    char c;
    char temp[7];

    temp[0] = '\0';
    for (i = 1; i <= 6; i++)
    {
         temp[i] = (num  & 07) + '0';
         num = (num >> 3) & 0037777;
    }

    temp[6] &= '1';
    for  (i = 6; temp[i] == '0'; i--);
    if   (i == 0)      i++;
    while(i >= 0)     *str++ = temp[i--];
}

/*-----------------------------------------------------------------------------
 * _longOctalString - ecrire au format octal long dans la string de travail
 *-----------------------------------------------------------------------------
 */
_longOctalString(num, str)
unsigned long num;
char *str;
{
    int i;
    char c;
    char temp[12];

    temp[0] = '\0';
    for (i = 1; i <= 11; i++)
    {
         temp[i] = (num  & 07) + '0';
         num = num >> 3;
    }

    temp[11] &= '3';
    for  (i = 11; temp[i] == '0'; i--);
    if   (i == 0)      i++;
    while(i >= 0)     *str++ = temp[i--];
}

/*-----------------------------------------------------------------------------
 * _hexaString - ecrire au format hexa dans la string de travail
 *-----------------------------------------------------------------------------
 */
_hexaString(num, str)
unsigned num;
char *str;
{
    int i;
    char c;
    char temp[5];

    temp[0] = '\0';
    for (i = 1; i <= 4; i++)
    {
         temp[i] = "0123456789ABCDEF"[num & 0x0f];
         num = num >> 4;
    }

    for  (i = 4; temp[i] == '0'; i--);
    if   (i == 0)      i++;
    while(i >= 0)     *str++ = temp[i--];
}

/*-----------------------------------------------------------------------------
 * _longHexaString - ecrire au format hexa long dans la string de travail
 *-----------------------------------------------------------------------------
 */
_longHexaString(num, str)
unsigned long num;
char *str;
{
    int i;
    char c;
    char temp[9];

    temp[0] = '\0';
    for (i = 1; i <= 8; i++)
    {
         temp[i] = "0123456789ABCDEF"[num & 0x0f];
         num = num >> 4;
    }

    for  (i = 8; temp[i] == '0'; i--);
    if   (i == 0)      i++;
    while(i >= 0)     *str++ = temp[i--];
}

/*-----------------------------------------------------------------------------
 * _StringHexaToLong - ecrire un long a partir d'une string hexa
 *-----------------------------------------------------------------------------
 */

int convert[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0,
                 10,11,12,13,14,15};
unsigned long expo(n, base)
{
   unsigned long ret = 1;
   while (n--> 0)
        ret = ret * base;
   return(ret);
}

long _stringHexaToLong(str)
char *str;
{
    int i, j, num;
    long val;
    unsigned long expo();

    strupr(str);
    for (j = 0, val = 0L, i = strlen(str) - 1; i >= 0; i--, j++)
         val += (convert[str[i] - 0x30] * expo(j, 16));

    return(val);
}



_stringHexaToInt(str)
char *str;
{
    int i, j, num;
    int val;
    unsigned long expo();

    strupr(str);
    for (j = 0, val = 0, i = strlen(str) - 1; i >= 0; i--, j++)
         val += convert[str[i] - 0x30] * expo(j, 16);

    return(val);
}

_stringHexaToO(str)
char *str;
{
    int i, j, num;
    int val;
    unsigned long expo();

    for (j = 0, val = 0, i = strlen(str) - 1; i >= 0; i--, j++)
         val += (str[i] - 0x30) * expo(j, 8);

    return(val);
}

long _stringHexaToLongO(str)
char *str;
{
    int i, j, num;
    long val;
    unsigned long expo();

    for (j = 0, val = 0, i = strlen(str) - 1; i >= 0; i--, j++)
         val += (str[i] - 0x30) * expo(j, 8);

    return(val);
}
