        umain()
        {
                int newpid, newSession;

                /* create a new process */
                if ((newpid = m_Fork()) == 0) {
                        /* child process code : print a
                         * message in a new session
                         */
                        newpid = m_Getpid();
                        m_GetSession();
                        while (1)
                           m_Printf("Child ID = %d\n", newpid);
                }
                /* parent process code */
                m_Printf("Parent ID = %d - Child ID = %d\n", m_Getpid(), newpid);
                m_getch();
        }



