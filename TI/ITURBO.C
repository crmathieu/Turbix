#include "xed.h"

/* interface entre compilateurs microsoft et Borland :
 * Adaptation des fonctions du turbo … la syntaxe
 * Microsoft
 */

#ifdef TURBO
#define  _harderr          harderr
#define  _bios_equiplist   biosequip
#define  _hardretn         hardretn
#define  _hardresume       hardresume

_dos_getdiskfree(drive, diskspace)
unsigned drive;
struct dfree *diskspace;
{
   getdfree(drive, diskspace);
   if (diskspace->df_sclus == -1)
       return(-1);
   return(0);
}

_dos_setvect(num_it, it)
void interrupt (*it)();
{
    setvect(num_it, it);
}

_dos_getvect(num_it)
{
     getvect(num_it);
}


_dos_getdrive(cdrive)
int *cdrive;
{
    *cdrive = getdisk();
}
_dos_setdrive(drive, cdrive)
int drive, *cdrive;
{
    *cdrive = setdisk(drive);
}
#endif

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * chk_devspace - verifie si le device peut recevoir un nouveau fichier
 *                (toujours sur le drive courant)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
chk_devspace(file, reset)
char *file;
{
#define NDRIVE 26
#ifdef TURBO
#define diskfree_t           dfree
#define total_clusters       df_total
#define avail_clusters       df_avail
#define sectors_per_cluster  df_sclus
#define bytes_per_sector     df_bsec
#endif
   static struct diskfree_t dskf[NDRIVE];

   static unsigned  freespace[NDRIVE] = {0,0,0,0,0,0,0,0,0,0,
                                         0,0,0,0,0,0,0,0,0,0,
                                         0,0,0,0,0,0};
   int drive, id;


   strupr(file);
   id = drive = file[0] - 'A' + 1;
   id--;
   if (reset)
        freespace[id] = 0;

   if (freespace[id] == 0) {
       if (_dos_getdiskfree(drive, &dskf[id]) == 0)
           freespace[id] = dskf[id].avail_clusters * dskf[id].sectors_per_cluster;
       else
           return(-2);
   }
   freespace[id] = dskf[id].avail_clusters * dskf[id].sectors_per_cluster;

   if (freespace[id] < ((fsize+bottom_line_no)/dskf[id].bytes_per_sector))
       return(-1);
   else {
       freespace[id] -= ((fsize+bottom_line_no)/dskf[id].bytes_per_sector);
       return(0);
   }
}
