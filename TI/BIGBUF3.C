/* bigbuf.c */

#include "xed.h"
#include "ext_var.h"

/*--------------------------------------------
 * newline - Adds EOS in bigbuf
 *--------------------------------------------
 */
FUNCTION newline(wp,curpos)
struct OBJ *wp;
{
    int i, pos;
    unsigned n;

/*    if ((pos = wp->curX + wp->leftCh) > lnlen(wp,current))
         pos = lnlen(wp,current);*/
    if (curpos > lnlen(wp,current))   pos = lnlen(wp,current);
    else                                  pos = curpos;

    n = current + pos;
    if ((long)fsize + 1 >= BIGBUF_SIZE - 1) {
        pushPop_MESS(outofmem, ERRMSG);
        return;
    }
    memmove(bigbuf + n + 1,
            bigbuf + n,
            fsize  - n);
    bigbuf[n] = EOS;
    fsize++;
    if (bottom != current)
        bottom++;
    else
        bottom += (pos + 1);
    current += (pos + 1);
/*    printf("C = %d",lnlen(wp,current));
    getch();*/
    bottom_line_no++;
    current_line_no++;
}

/*-------------------------------------------------------
 * line_too_big - takes care of truncating oversized line
 *-------------------------------------------------------
 */
FUNCTION line_too_big(wp)
struct OBJ *wp;
{
    int i, pos;
    unsigned n;

    pushPop_MESS(ltoobig, ERRMSG);
    newline(wp,nb_car_per_line - 1);
}

/*-----------------------------------------------------------------
 * deleteline - deletes current line in big buffer
 *              (without taking into account line buffer)
 *-----------------------------------------------------------------
 */
FUNCTION deleteline(wp)
struct OBJ *wp;
{
    int i, pos;
    unsigned n;

    pos = lnlen(wp,current) + 1;
    n = current;
    memmove(bigbuf + n,
            bigbuf + n + pos,
            fsize  - (n + pos));
    fsize -= pos;
    bottom_line_no--;
    if (bottom != current)
        bottom -= pos;
    else
        current_line_no--;
    fflag &= ~FF_LINE_UPDATE;
}

/*---------------------------------------------------------------------
 * concat(wp) - concatainates current line with previous line in
 *              big buffer
 *---------------------------------------------------------------------
 */
FUNCTION concat(wp,prev)
struct OBJ *wp;
unsigned prev;
{
    int i;
    unsigned n;

    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,ERASE);

    n = current;
    memmove(bigbuf + n - 1,
            bigbuf + n,
            fsize  - n);
    fsize--;
    bottom_line_no--;
    current_line_no--;
    if (bottom != current) {
        bottom--;
        current = prev;
    }
    else
        current = bottom = prev;
}


/*---------------------------------------------------------------
 * insert_lnbuf - Attempts to insert data from line buffer
 *                in big buffer
 *---------------------------------------------------------------
 */
FUNCTION insert_lnbuf(wp,blank)
struct OBJ *wp;
{
    int i, Plen, Nlen, Blen, val, diff;
    unsigned pos;

    Plen = lnlen(wp,current) + 1;
    Blen = _lnlen(linebuf) + 1;

    /* erase trailing space */
    if (blank == ERASE)  Nlen = erase_trailing_blank(wp) + 1;
    else                 Nlen = _lnlen(linebuf) + 1;

    /* block management */
    if ((fflag & FF_BLOCKDEF) && (blank == ERASE)) {
        if ((val = Blen - Nlen))  { /* > 0) {*/
            if (((pos = current + wp->curX + wp->leftCh) > topBlock) &&
                 (pos <= bottomBlock))
                  bottomBlock -= val;
            else
                  if (pos <= topBlock) {
                      topBlock -= val;
                      bottomBlock -= val;
                  }
        }
    }
    if (fflag & FF_LINE_UPDATE)
        fflag |= FF_DIRTY;

    fflag &= ~FF_LINE_UPDATE;

    if ((diff = Nlen - Plen) == 0) {
        /* line has same size: copy without shifting */
        copy_lnbuf_to_bigbuf(wp,Plen);
        return;
    }

    /* la ligne modifiee n'a pas la meme taille que celle dans bigbuf */
    if ((long)fsize + diff >= BIGBUF_SIZE - 1) {
        pushPop_MESS(outofmem, ERRMSG);
        return;
    }
    pos = current + Plen;
    memmove(bigbuf + pos + diff,
           bigbuf + pos,
           fsize - pos);
    copy_lnbuf_to_bigbuf(wp,Nlen);
    fsize += diff;
    if (bottom != current)
        bottom += diff;
}

/*---------------------------------------------------------------------
 * copy_lnbuf_to_bigbuf - copier le buffer ligne a la place de la ligne
 *                        courante dans bigbuf
 *---------------------------------------------------------------------
 */
FUNCTION copy_lnbuf_to_bigbuf(wp,size)
struct OBJ *wp;
{
   char *line;
   char far *pos;
   pos  = bigbuf + current;
   line = linebuf;
   while (size-- > 0)
          *pos++ = *line++;
   if (current == bottom)
          *(--pos) = EOFM;
}

/*---------------------------------------------------------------
 * copy_block -  copier le block defini a partir de la position
 *               courante dans le big buffer
 *---------------------------------------------------------------
 */
FUNCTION copy_block(wp,operation)
struct OBJ *wp;
{
    int zebeb, firstCh;
    unsigned blklen, blklen0, pos, u, h;

    /* prise en compte modifications de la ligne courante */
    if (fflag & FF_LINE_UPDATE) {
        insert_lnbuf(wp,NO_ERASE);

        /* ajuster les marques du bloc */
        if ((fillCh > 0) && (current + wp->curX/**/ + wp->leftCh/**/ < topBlock)) {
             topBlock += fillCh;
             bottomBlock += fillCh;
             fillCh = 0;
        }
    }
    /* calcul de la taille du block */
    blklen = blklen0 = bottomBlock - topBlock;

    /* offset debut de copie */
    pos = current + (firstCh = wp->curX + wp->leftCh);

    /* check size */
    if ((long)fsize + blklen >= BIGBUF_SIZE - 1) {
        pushPop_MESS(outofmem, ERRMSG);
        return;
    }

    /* deplacer le bloc a partir de la position courante */
    memmove(bigbuf + pos + blklen,
            bigbuf + pos,
            fsize - pos);

    /* ajuster les adresses du block si block apres ligne courante */
    if (current + firstCh < topBlock) {
        topBlock += blklen;
        bottomBlock += blklen;
    }

    /* copier le block */
    memmove(bigbuf + pos,
            bigbuf + topBlock,
            blklen);

    fsize += blklen;

    /* mise a jour position du bloc courant */
    topBlock = current + firstCh;
    bottomBlock = topBlock + blklen;

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
        bottom += blklen0;
    else
        bottom = getdebln(wp,bottomBlock);

    /* M A J  numero de la derniere ligne */
/*    for (u = topBlock; u < bottomBlock; u++)
         if (bigbuf[u] == EOS) bottom_line_no++;*/

    bottom_line_no += getnln(wp,topBlock,bottomBlock) - zebeb;

    if (operation == COPY_BLOCK)
        set_win(wp);
}


/*-------------------------------------------
 * delete_block -  supprimer le block defini
 *-------------------------------------------
 */
FUNCTION delete_block(wp,operation)
struct OBJ *wp;
{
    int firstCh,ligdeb,nline,i,k, correction;
    unsigned blklen, blklen0, pos, deb, u;

    /* prise en compte modifications de la ligne courante */
    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp, ERASE);

    /* calcul de la taille du block */
    blklen = blklen0 = bottomBlock - topBlock;

    /* calcul du nb de lignes dans le block */
/*    for (u = topBlock, nline = 0; u < bottomBlock; u++)
         if (bigbuf[u] == EOS)   nline++;*/

    nline = getnln(wp, topBlock, bottomBlock);

    /* offset debut de suppression */
    pos = topBlock;
    deb = getdebln(wp, topBlock); /* 1ere ligne de la suppression */
    ligdeb = getnln(wp, 0, deb);

    /* mise a jour des pointeurs */
    if (bottom <= bottomBlock) { /* last line in block */
        if (bottom > topBlock) {
            bottom = getdebln(wp, topBlock);
            bottom_line_no -= nline;
        } /* sinon : block sur la derniere ligne : ne rien faire */
    }
    else {
        bottom_line_no -= nline;
        bottom -= blklen;
    }
    if (operation == DELETE_BLOCK) {
        /*----------------------------*
         *       DELETE_BLOCK         *
         *----------------------------*/
        current = deb;

/*        if (current <= deb)
            while (current < deb) {
                   if ((current = lnnext(wp,current)) == NIL_LN)
                        break;
            }
        else
            while (current > deb) {
                   if ((current = lnprev(wp,current)) == NIL_LN)
                        break;
            }*/

        /* tester position du block / a la page courante */
        if ((topBlock - topPage >= 0) &&
            (i = getnln(wp,topPage,topBlock)) < wp->nline)
        {
            /* debut block dans la page */
            wgotoxy(wp,topBlock - getdebln(wp,topBlock),i);
            current_line_no = topPage_line_no +
                                  getnln(wp,topPage,topBlock);
        }
        else { /* debut block avant ou apres la page :
                * se positionner en debut de block apres suppression
                */
               topPage = current;
               topPage_line_no = current_line_no = ligdeb;/*getnln(wp,0,current) + 1;*/
               wgotoxy(wp,topBlock - getdebln(wp,topBlock),0);
        }
        /* supprimer le bloc a partir de la position courante */
        memmove(bigbuf + pos,
                bigbuf + pos + blklen,
                fsize - (pos + blklen));

    }
    else { /*--------------------------*
            *       MOVE_BLOCK         *
            *--------------------------*/
        /* tester position du block / a la page courante */
        if ((current >= bottomBlock) || (current == getdebln(wp,bottomBlock))) {
            if (topPage < bottomBlock) {
                /* fin de block dans la page */

                /* calculer # de lignes entre bottomBlock et current */
                k = getnln(wp,bottomBlock,current);

                /* si la fin de block est sur la ligne courante */
/*              if (current == getdebln(wp,bottomBlock))
                        correction = bottomBlock - current;
                else
                        correction = 0;*/

                if (topPage > topBlock) {
                    /* le bas du block est dans la page */
                    topPage         = deb;
                    topPage_line_no = ligdeb;
                    i               = 0;
                }
                else
                    /* block entierement dans la page */
                    i = getnln(wp,topPage,topBlock);

                /* supprimer le bloc a partir de la position courante */
                memmove(bigbuf + pos,
                        bigbuf + pos + blklen,
                        fsize - (pos + blklen));
                current = getnextline(wp,deb,k);
                current_line_no = topPage_line_no +
                                      getnln(wp,topPage,current);
                wgotoxy(wp,wp->curX, i + k);
            }
            else { /* le block est entierement exterieur a la page */
                topPage -= blklen;
                current -= blklen;
                current_line_no -= nline;
                topPage_line_no -= nline;

                /* supprimer le bloc a partir de la position courante */
                memmove(bigbuf + pos,
                        bigbuf + pos + blklen,
                        fsize - (pos + blklen));
            }
        }
        else  /* current <= bottomBlock */
            memmove(bigbuf + pos,
                    bigbuf + pos + blklen,
                    fsize - (pos + blklen));
    }
    topBlock = bottomBlock = NIL_LN;
    fsize -= blklen;

    /* verifier si la 1ere ligne du bloc copi- n'est pas trop longue */
    if (lnlen(wp,pos) > nb_car_per_line - 1) {
        line_too_big(wp);
        current = lnprev(wp,current);
        current_line_no--;
    }
    if (operation == DELETE_BLOCK) {
        fflag &= ~FF_BLOCKDEF;
        set_win(wp);
    }
}

