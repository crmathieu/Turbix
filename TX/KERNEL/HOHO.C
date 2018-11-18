    if (strlen(vl->exename) == 0) {
        strcpy(wkfile,vl->absfname);
        strcpy(vl->exename, wkfile);
    }
