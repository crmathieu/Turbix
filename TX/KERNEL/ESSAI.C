char buf[128];
char buf2[]="12341BCD12345676543qJe suis tres joli\n";
main(c,p)                 /* essai tache utilisateur */
char *p[];
{
  long l,k;
  char ch;

  char *argv[4];

/*  scanf("Long H %lx - Long O: %lo - CHAR = %c - String %s \n", &l, &k, &ch, buf);*/
  scanf("%lx %lo %c %s", &l, &k, &ch, buf);
  printf("LH = %lx\n", l);
  printf("ST = %s\n", buf);
  printf("LO = %lo\n", k);
  printf("CH = %c\n", ch);
  getch();
}