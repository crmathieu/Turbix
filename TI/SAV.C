   /* recuperer les coordonnees souris (si elle existe) */
   m_getPosAndButtonStatus(&mouX, &mouY, &i);

   /* check mouse */
   if (!m_installed(&bidon)) {
        /* pas de souris, installer
         * l'interruption DUMMY
         */
        BiosInt33 = (unsigned long)_dos_getvect(0x33);
        _dos_setvect(0x33, Dummy_interrupt);
        mouse_present = FALSE;
   }
   else {
        /* positionner l 'interruption souris editeur */
        mouse_present = TRUE;

        m_setPos(mouX, mouY);
        showMouse();

   }
