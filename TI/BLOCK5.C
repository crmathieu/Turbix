/* block.c : gestion des blocs */

#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>

/*
 *--------------------------------------------
 * ctrlkb - mark block begin
 * ctrlkk - mark block end
 * ctrlkc - copy defined block
 * ctrlkv - move defined block
 * ctrlkh - hide defined block
 * ctrlky - delete defined block
 * ctrlkr - read block from paste file        +
 * ctrlkw - copy defined block to paste file  +
 *---------------------------------------------
 */

/*--------------------------------------------------------
 * 1 -  ctrlkb   mark bloc begin
 *-------------------------------------------------------
 */
ctrlkb(wp)
struct OBJ *wp;
{
      int firstCh, i, j;

      if (fflag & FF_LINE_UPDATE)
          insert_lnbuf(wp,ERASE);

      firstCh = INF((i = wp->leftCh + wp->curX), j = lnlen(wp,current));
      topBlock   = current + firstCh;
/*      if (current != getdebln(wp, topBlock))
          topBlock++;*/
      if ((firstCh <= i) && (i >= j) && (topBlock != current))
        topBlock++;


      if (fflag & FF_BLOCKDEF) {  /* un bloc est deja defini */
          /* supprimer le bloc precedent si cette ligne est superieure
           * au bas de bloc ou (cas ou current == bottomBlock) si le 1er
           * carac est superieur au dernier carac
           */
          if (bottomBlock < topBlock)
              fflag &= ~FF_BLOCKDEF;

          write_page_att(wp);
      }

      if (fflag & FF_BLOCKDEF)
          return(TRUE);
      else
          return(NIL);
}

/*--------------------------------------------------------
 * 2 -   mark_bloc_end
 *-------------------------------------------------------
 */
ctrlkk(wp)
struct OBJ *wp;
{
      int lastCh;
      if (fflag & FF_LINE_UPDATE)
          insert_lnbuf(wp,ERASE);

      lastCh = INF(wp->leftCh + wp->curX,lnlen(wp,current));
      bottomBlock = current + lastCh;

      fflag        &= ~FF_BLOCKDEF;
      if (topBlock  != NIL_LN ) { /* on a deja marqu- un debut bloc */
          if (topBlock <= bottomBlock)
              fflag |= FF_BLOCKDEF;

          write_page_att(wp);
      }
      if (fflag & FF_BLOCKDEF)   return(TRUE);
      else                           return(NIL);
}

/*--------------------------------------------------------
 * hide block
 *-------------------------------------------------------
 */
ctrlkh(wp)
struct OBJ *wp;
{
      fflag        &= ~FF_BLOCKDEF;
      topBlock = bottomBlock = NIL_LN;
      write_page_att(wp);
}

/*-----------------------------------------------------------
 * 1 -  ctrlkc   copy a block in the current cursor position
 *-----------------------------------------------------------
 */
ctrlkc(wp)
struct OBJ *wp;
{
   int diff, off, plen;

   if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
   }

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   /* tester si position courante apres fin de ligne */
   diff = (off = wp->leftCh + wp->curX) - (plen = lnlen(wp,current));
   if (off > plen) {
/*        if (!(fflag & FF_LINE_UPDATE)) {
              fill_line(linebuf,(nb_car_per_line -  1) * 2,EOS);
              lncpy(wp);
              fflag |= FF_LINE_UPDATE;
        }*/
        fflag |= FF_LINE_UPDATE;
        formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
        /* combler par des blancs */
        fillCh = off - plen;  /* memoriser nb de blancs rajout-s */
        while (plen < off)
               linebuf[plen++] = BLANK;
   }
   else
        diff = 0;
   if (fflag & FF_BLOCKDEF) {
       /* verifier si la copie ne s'effectue pas dans le bloc defini */
       if ((current + off  < bottomBlock + diff) &&
           (current + off > topBlock + diff))  return(-1);

       copy_block(wp,COPY_BLOCK);  /* copy block in the same file */
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
       fflag |= FF_DIRTY;
       return(0);
   }
   return(-1);
}

/*--------------------------------------
 * 1 -  ctrlky   delete a defined block
 *--------------------------------------
 */
ctrlky(wp)
struct OBJ *wp;
{

   if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
   }

   if (fflag & FF_BLOCKDEF) {
       if (fflag & FF_LINE_UPDATE)
           insert_lnbuf(wp,ERASE);
       delete_block(wp,DELETE_BLOCK);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
       fflag |= FF_DIRTY;
   }
}

/*--------------------------------------------------------
 * 1 -  ctrlkv   move  block
 *-------------------------------------------------------
 */
ctrlkv(wp)
struct OBJ *wp;
{
    unsigned topB,botB,finaltopB,finalbotB;
    int size;

    int diff, off, plen;

   if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
   }

    if (fflag & FF_BLOCKDEF) {
        if (fflag & FF_LINE_UPDATE)
            insert_lnbuf(wp,ERASE);

        /* tester si position courante apres fin de ligne */
        diff = (off = wp->leftCh + wp->curX) - (plen = lnlen(wp,current));
        if (off > plen) {
 /*           fill_line(linebuf,(nb_car_per_line -  1) * 2,EOS);
            lncpy(wp);*/
            fflag |= FF_LINE_UPDATE;
            formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);

            /* combler par des blancs */
            fillCh = off - plen;  /* memoriser nb de blancs rajout-s */
            while (plen < off)
                   linebuf[plen++] = BLANK;
        }
        else
            diff = 0;

        /* verifier si la copie ne s'effectue pas dans le bloc defini */
        if ((current + off  < bottomBlock + diff) &&
            (current + off > topBlock + diff))  return(-1);

        topB = topBlock;
        botB = bottomBlock;
        size = botB - topB;

        /* ajuster les marques si copie avant le block */
        if (current + off < topB + diff) {
            topB += (size+diff);
            botB += (size+diff);
        }
        copy_block(wp,MOVE_BLOCK);
        finaltopB = topBlock;
        finalbotB = bottomBlock;
        /* ajuster les marques si block supprim- avant block final */
        if (topB < finaltopB) {
            finaltopB -= size;
            finalbotB -= size;
        }
        topBlock = topB;
        bottomBlock = botB;
        delete_block(wp,MOVE_BLOCK);
        topBlock = finaltopB;
        bottomBlock = finalbotB;

        wgotoxy(wp,topBlock - getdebln(wp,topBlock), wp->curY);

        set_win(wp);
        formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
        fflag |= FF_DIRTY;
    }
}

/*-----------------------------------------------------------
 * ctrlkr   copy a block from the paste file
 *-----------------------------------------------------------
 */
ctrlkr(wp)
struct OBJ *wp;
{
   int diff, off, plen;


   if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
   }

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   /* tester si position courante apres fin de ligne */
   diff = (off = wp->leftCh + wp->curX) - (plen = lnlen(wp,current));
   if (off > plen) {
        fflag |= FF_LINE_UPDATE;
        formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
        /* combler par des blancs */
        fillCh = off - plen;  /* memoriser nb de blancs rajout-s */
        while (plen < off)
               linebuf[plen++] = BLANK;
   }
   else
        diff = 0;
/*   if (fflag & FF_BLOCKDEF) {
       /* verifier si la copie ne s'effectue pas dans le bloc defini */
/*       if ((current + off  < bottomBlock + diff) &&
           (current + off > topBlock + diff))  return(-1);
   }*/
   topBlock = bottomBlock = 0;
   fflag &= ~FF_BLOCKDEF;
   if (copy_block_from_paste(wp) == -1)  /* copy block in the same file */
       return(-1);
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
   fflag |= FF_DIRTY;
   return(0);

}

/*-----------------------------------------------------------
 * copy_block_from_paste   copy a block from the paste file
 *-----------------------------------------------------------
 */
copy_block_from_paste(wp)
struct OBJ *wp;
{

    struct stat sbuf;
    struct OBJ errwin;
    int zebeb, firstCh, fd;
    unsigned blklen, logicLen, pos, u, h;

    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,NO_ERASE);

    /* evaluer la taille du fichier de paste (qui se trouve
     * dans la home directory)
     */
    if (stat(pastefile, &sbuf) == 0) {
        /* existence */
        fd = open(pastefile, O_RDWR);
        blklen = (int)sbuf.st_size;
    }
    else
        return(-1);

    /* offset debut de copie */
    pos = current + (firstCh = wp->curX + wp->leftCh);

    /* check size */
    if ((long)fsize + blklen >= BIGBUF_SIZE - 1) {
        pushPop_MESS(outofmem, ERRMSG);
        return(-1);
    }

    /* deplacer le bloc a partir de la position courante */
    memmove(bigbuf + pos + blklen,
            bigbuf + pos,
            fsize - pos);

    /* copier le block */
    push_MESS(readblk, &errwin);

    if ((logicLen = read(fd, bigbuf + pos, blklen)) < blklen)
         /* ajuster la longueur */
         memmove(bigbuf + pos + logicLen,
                 bigbuf + pos + blklen,
                 fsize - pos);
    pop_MESS();

    fsize += logicLen;

    /* mise a jour position du bloc courant */
    topBlock = current + firstCh;
    bottomBlock = topBlock + logicLen;
    fflag |= FF_BLOCKDEF;

    /* verifier si la 1ere ligne du bloc copi- n'est pas trop longue */
    if ((h =lnlen(wp,current)) > nb_car_per_line - 1) {
        line_too_big(wp);
        zebeb = 1;
        current = lnprev(wp,current);
        /* augmenter la taille du bloc si ce dernier est coup- en deux */
        if ((current + h >= topBlock) &&
            (current + h <= bottomBlock))
             bottomBlock++;
        current_line_no--;
    }
    else
       zebeb = 0;

    if (current != bottom)
        bottom += logicLen;
    else
        bottom = getdebln(wp,bottomBlock);

    /* M A J  numero de la derniere ligne */
    bottom_line_no += getnln(wp,topBlock,bottomBlock) - zebeb;
    set_win(wp);
    close(fd);
    return(0);
}

/*-----------------------------------------------------------
 * ctrlkw   copy the defined block to the paste file
 *-----------------------------------------------------------
 */
ctrlkw(wp)
struct OBJ *wp;
{
   int fd;
   struct OBJ errwin;

    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,ERASE);

   if (fflag & FF_BLOCKDEF) {
       if ((fd = open(pastefile, O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE)) == -1) {
            pushPop_MESS(cpyPasteStr, ERRMSG);
            return(-1);
       }
       push_MESS(writeblk, &errwin);
       write(fd, &bigbuf[topBlock], bottomBlock - topBlock);
       pop_MESS();
       close(fd);
   }
}

/*---------------------------------------------------------------
 * getdebln - retourne l'offset du debut ligne dans le big buffer
 *---------------------------------------------------------------
 */
unsigned getdebln(wp,pt)
struct OBJ *wp;
unsigned pt;
{
   int i = 0;

   while ((bigbuf[pt] != EOS) && (pt != 0)) {
        i++;
        pt--;
   }
   if (pt != 0) {
       if (i > 0) return(pt+1);
       else       return(pt);
   }
   return(0);
}

/*-----------------------------------------------------------------------
 * 2 - write_page_att - getstion des attributs block/paste au niveau page
 *-----------------------------------------------------------------------
 */
write_page_att(wp)
struct OBJ *wp;
{
   int i;
   unsigned pt;
   pt = topPage;

   for (i=0; i <= wp->nline - 1; i++) {
        put_att_BP(wp,pt,i,lnlen(wp,pt));
        if ((pt = lnnext(wp,pt)) == NIL_LN) break;
   }
}


/*--------------------------------------------------------------------
 * 2 - put_att_BP - gestion des attributs bloc et paste sur une ligne
 *--------------------------------------------------------------------
 */
put_att_BP(wp,pt,line,len)
struct OBJ *wp;
unsigned pt;
{
     int fcb, lcb; /* len */
     unsigned firstCh, lastCh, topB, botB;

     topB    = topBlock;
     botB    = bottomBlock;
     fcb     = 0;
     lcb     = wp->ncol;
/*     len     = lnlen(wp,pt);*/

     /* si il existe un bloc defini, verifier si la
      * ligne appartient au bloc
      */
     if (fflag & FF_BLOCKDEF) {
         if ((pt <= topB && len >= topB - pt) ||
             (pt >= topB && pt <= botB)) {
             /* la ligne est dans le bloc :
              * ajuster son attribut
              */

             /* A) cas de la 1ere ligne du bloc */
             if (pt < topB && len >= (firstCh = topB - pt))  {
                 if ((fcb = firstCh - wp->leftCh) > wp->ncol)
                      fcb = wp->ncol;
                 if (fcb > 0)  /* remettre attribut Normaux */
                     _brighten( wp->ul_x + 1,
                                wp->ul_y + 1 + line,
                                fcb,
                                wp->ink|wp->paper);
             }

             /* B) cas de la derniere ligne du bloc */
             if (pt <= botB && len >= (lastCh = botB - pt)) {
                 if ((lcb = lastCh - wp->leftCh) > wp->ncol)
                      lcb = wp->ncol;
                 if (lcb >= 0 && lcb < wp->ncol) /* remettre attribut normaux */
                     _brighten( wp->ul_x + 1 + lcb,
                                wp->ul_y + 1 + line,
                                wp->ncol - lcb,
                                wp->ink|wp->paper);
                 else
                     _brighten( wp->ul_x + 1,
                                wp->ul_y + 1 + line,
                                wp->ncol,
                                wp->ink|wp->paper);

             }
             fcb = (fcb > 0 ? fcb : 0);
             lcb = (lcb > 0 ? lcb : 0);
             if (lcb - fcb == 0)
                 return;

             _brighten(wp->ul_x + 1 + fcb,
                       wp->ul_y + 1 + line,
                       lcb - fcb,
                       wp->blockAtt); /*WINblock);*/
         }
         else
             _brighten(wp->ul_x + 1,/* + fcb,*/
                       wp->ul_y + 1 + line,
                       wp->ncol,
                       wp->ink|wp->paper);

     }
     else {
             _brighten(wp->ul_x + 1,
                       wp->ul_y + 1 + line,
                       wp->ncol,
                       wp->ink|wp->paper);
     }
}

