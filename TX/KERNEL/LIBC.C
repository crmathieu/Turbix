/* libc.c */

/* fonctions de la libraire C */

/*---------------------------------------------------------------------------
 * strcmp - compare 2 chaines de caracteres . retourne 0 si egalite
 *---------------------------------------------------------------------------
 */
strcmp(str1, str2)
char *str1, *str2;
{
    while(*str1 == *str2++)
         if (*str1++ == '\0')
              return(0);
    return(*str1 - *(--str2));
}

Xstrcmp(str1, str2)
char *str1, *str2;
{
    while(*str1 == *str2) {
         m_printf("ST1 %c      ST2 %c\n",*str1,*str2++);
         if (*str1++ == '\0') {
              m_putc('#');
              return(0);
         }
    }
    m_printf("Difference   ST1 %c      ST2 %c\n",*str1,*str2);
    return(*str1 - *(--str2));
}

/*---------------------------------------------------------------------------
 * strlen - retourne la longueur d'une string
 *---------------------------------------------------------------------------
 */
strlen(string)
char *string;
{
  int i = 0;
  while(*(string++)) i++;
  return(i);
}
