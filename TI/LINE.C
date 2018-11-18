   /* creer le fichier de redirection */
   if ((fpred = fopen(fError,"w+")) == NULL)
   {
      sprintf(work, builtStr);
      pushPop_MESS(work, ERRMSG);
      return(FUNC|F10);
   }
   fclose(fpred);
	Xsave_previous_file(absfname);
        push_pick(absfname);

        /* sauvegarder la configuration */
        memmove(&conf.reload[0], &picktab[0], PICKSIZE * 17);
        doSconfig(mp, 0);

        strcpy(vl->fError, fError);
        strcpy(vl->cdirname, cdirname);
        strcpy(vl->homedir, homedir);
        strcpy(vl->relname, relname);
        strcpy(vl->absfname, absfname);
        strcpy(vl->exename, exename);

        vl->WINSYSink           = WINSYSink;
        vl->WINSYSpaper         = WINSYSpaper;
        vl->WINSYSborder        = WINSYSborder;
        vl->WINSYSblock         = WINSYSblock;
        vl->WINmessInk          = WINmessInk;
        vl->WINmessPaper        = WINmessPaper;
        vl->WINmessBorder       = WINmessBorder;
        vl->WINDIRink           = WINDIRink;
        vl->WINDIRpaper         = WINDIRpaper;

        vl->segVideo            = SEGvideo;
        vl->adapter             = adapter;
        vl->retCode             = 0;

        vl->mkd_shell           = conf.mkd_shell;
        vl->mkd_FS              = conf.mkd_FS;
        vl->mkd_AutoAsk         = conf.mkd_AutoAsk;
        vl->mkd_masm            = conf.mkd_masm;
        vl->mkd_tasm            = conf.mkd_tasm;

        sdg
        sdg
        sdg
        sd
        gsdg
        sdfg
        dsg
        g
        sd
        gsdg
        sd
        gsdgdsg
        sdgdg
        dfg
        dfg
        dfg
        dfg
        dsfg
        dfg
        sdfg
        sdfg
        sdg

        gd
        gsdg
        sdg
        dsg
        dg
        g
        ds
        gsdg

        memcpy(&vl->conf, &conf, sizeof(struct configuration));

        /* liberer BIGBUF */
        free(bigbuf);

        /* inhiber la souris */
        inhibe_mouse();
