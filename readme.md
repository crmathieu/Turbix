# Turbix
A Dos-based C library implementing Unix system calls through a built-in multitasked kernel. Largely inspired by the book "The Xinu Operating System" authored by Douglas Comer.

## PACKAGE COMPONENTS
This package is designed to work with the BORLAND C compiler but doesn't include it. The LARGE memory model is the only one supported, therefore you'll have to install the LARGE TURBO C library.

## DISK COMPONENTS
- install.exe (the installation program).
- \TX\KERNEL directory which contains the TURBIX kernel libraries.
- \TX\TI directory which contains executable and overlay development tools files.
- \TX\TI\HELP directory which contains help files
- \TX\USR directory which contains the kernel header files (.H) and where the user will have to create his program and header files

To start the Multitasking Kernel for DOS application Development Interface, type "TI".

## LIBRARIES
- TXSML:   All system calls except SPY and Windows management.
- TXSPY:   SPY library
- TXWIN:   Windows library

## Installing TURBIX is easy:
- Insert the diskette in any floppy and type "INSTALL.EXE"; this program creates the following directories on the hard disk:
                \TX\KERNEL
                \TX\USR
                \TX\TI
                \TX\TI\HELP

- The installation program then copies the files in their appropriate directory and updates a PROFILE file according to the chosen options.

- After the installation program is finished,  type "TI" to start the development interface. You will have to select the Options menu to check whether you accept the default values or not.

        Default values are:
        - Compiler and Linker path  -> C:\TC
        - C Libraries path          -> C:\TC\LIB
        - C Include path            -> C:\TC\INCLUDE
        - User path                 -> C:\TX\USR
        - TURBIX header files path  -> C:\TX\USR
        - TURBIX kernel             -> C:\TX\KERNEL
        - Help files path           -> C:\TI\TI\HELP


  Now, TURBIX is ready to be used.

## TURBIX PROGRAMMING OVERVIEW


### INTRODUCTION
A - Terminology
B - Package contents
C - Kernel initialization
D - Return to DOS
E - Creating an application
F - System processes

### PROCESS MANAGEMENT
A - Introduction
B - Process states
C - Process scheduling
D - Interprocess communication
E - Process synchronization

### INPUT/OUTPUT
A - Terminology
B - Overview

### WINDOWS
A - Introduction
B - Enter Windows mode
C - Window creation
D - Window focus
E - Window Refresh
F - Window deletion
G - Standard screen
H - Exit Windows mode

----------------------------------------------------------------------------------


  _Introduction_

  TURBIX is a library including a multitasking kernel. The concepts provided are the same as those available on multitasking operating systems. This package enables the writing of multitasking applications in "C" language and binds them to the provided multitasking library. The result is a DOS executable that can run parallel tasks.

  A) TERMINOLOGY

  The KERNEL is a set of procedures that control CPU sharing,
  contexts swapping, process creation, process deletion, files
  access, semaphores, signals, messages, and pipe mechanisms.

  The Multitasking Kernel for Dos application (TURBIX) is composed
  of a kernel and system calls that can be used by a process.

  A PROCESS is a program that has begun to run. It is composed
  of an executable code and a stack where local variables are
  stored. All processes share the same global variables, including
  the kernel variables.

  There is no dynamic link mechanism with the kernel. In other
  words, the kernel cannot load and execute separated processes.
  The available processes are those linked with the kernel.


  B) PACKAGE COMPONENTS

  This package doesn't include a compiler; the BORLAND C compiler
  will have to be used. There is only ONE memory model supported:

        the LARGE memory model.

  So LARGE TURBO C library has to be installed in order to use
  the kernel development tools.


  C) KERNEL INITIALIZATION

  When a TURBIX application starts, the kernel gets all the available
  DOS memory initializes its own memory management and builds the
  first process.

  This first process has to create system tables, initialize the File-System, 
  start other system processes, and finally start YOUR application. After doing 
  that, the first process enters an infinite loop that will keep CPU resources 
  while no other process will be able to run.



  D) RETURN TO DOS

  To stop the kernel and return to the DOS interpreter, the user
  has to type CTRL+RETURN.
  It is possible to return to DOS by program with the "m_Shutdown"
  system call.



  E) CREATE AN APPLICATION

  The user application entry point is called "umain" (User Main).
  if you forget it and use "main", a link error will occur.

  The programmer has TWO possibilities

  1)    Create his application in the user entry point "umain". In
        such a case, there is no possibility to control what the
        process does.
```text
                   .-------------------.
                   |    APPLICATION    |
                   |-------------------|
                   |    K E R N E L    |
                   `-------------------'
```

```C
        umain()
        {
              .
              -     Application
              .
        }
```

  2)    Create his application through the spy interpreter.
        In this case, he can easily check how his application
        works, controlling its STATE, I/O, and EVENT which causes
        suspension etc...

        To do so, he just has to call the spy interpreter
        in interactive mode from the user entry point.

        This spy provides a set of commands which are useful
        to see resource states and system tools. When the spy
        prompt ('#') appears, type "help" to get the available
        commands.

        In this command set, the "user_app" command is the
        user entry point through the spy interpreter.

        So, creating an application and testing it through the
        SPY consists of:

            1) Creating the application under the name "user_app"
            2) Calling the spy interpreter in the user entry "umain"
            3) Typing the "session" command to create a spy session
            4) Typing "user_app" to start the user application
            5) At any time, swapping from one session to another and
               using spy command in the spy session to check how the
               application works.

```text
              .---------------------------------------. available
              |user_app |spy tool1|spy tool2|spy toolN| commands
              |---------------------------------------|
              |                 S P Y                 |
              |---------------------------------------|
              |              K E R N E L              |
              `---------------------------------------'
```

```C
          #include "const.h"
          umain(argc, argv)
          int argc;
          char *argv[];
          {
              extern int m_Spy(); /* spy declaration */
              char *p[3];
              int noUse;

              /* ... start the spy interpreter in interactive mode */
              if (m_Fork() == 0) {
                  p[0] = "spy";
                  p[1] = "-i";
                  p[2] = NULL;
                  m_Exec(m_Spy, p);
              }
              m_Wait(&noUse);
          }

          /* user application that will be
           * called by the SPY
           */
          user_app(argc, argv)
          int argc;
          char *argv[];
          {

                /* User program */
                        .
                        .
                        .

                m_Exit(0);     /* return to Spy */
          }
```


  F) SYSTEM PROCESSES

  The kernel creates through the first process several processes
  (called system processes). These processes are used to control
  resource access and manage hardware interrupts.


----------------------------------------------------------------------------------


  PROCESS MANAGEMENT
  ------------------

  A) INTRODUCTION

  Each process is known through an integer value called Process
  IDentifier (or PID). This value is used by the kernel to update
  internal process variables (opened files, Parent process PID,
  state, etc...). A process works with TWO stacks: a USER stack
  on which local variables are defined and a KERNEL stack used
  with some system calls. The kernel doesn't check stack overflow,
  so the programmer will have to be careful with the local variable
  sizes. (Stack size = 4096 bytes)

  Because some jobs are more important than others, each process owns
  a given priority level. The processes created by the user have the
  same priority level, but this can be modified with the "m_Nice"
  System-Call that reduces the priority to a given process.


  B) PROCESS STATES

  Each process behavior is defined by an internal process variable; its
  content describes the Process state.

        There are THREE possible values:

        -  READY        The process is able to run but another
                        Process is using the CPU

        -  RUNNING      The process uses the CPU

        -  SLEEP        The process is waiting for an event, a
                        resource availability or a delay

  Note:
  There is a fourth state that corresponds to an uncreated process.



  A process shifts from one state to another according to
  this diagram:

```text
                              System call or
             .--------------. clock interrupt   .--------------.
             |  R E A D Y   |--------->---------| R U N N I N G|
             |              |---------<---------|              |
             `--------------'                   `--------------'
                  |    | Event receipt               |    |
                  |    |                             |    |
                  |    |    .---------------         |    |
       m_Fork     |    `-<--|  S L E E P   |----<----'    | m_Exit
                  |         |              |              |
                  |         `--------------' Waiting for  |
                  |                           an  Event   |
                  |         .---------------              |
                  `------<--| U N U S E D  |----<---------'
                            |              |
                            `--------------'
```

  A process will enter a SLEEP state when waiting for one of
  the following events:


  EV_SEM          Resource availability
  EV_ZOM          Parent process acknowledgment
  EV_MESS         Message receipt
  EV_TMESS        Message receipt with time out
  EV_PAUSE        Signal receipt
  EV_CLOCK        End of delay
  EV_PIPE         Pipe availability
  EV_WAIT         End of child signal
  EV_LOCK         File unlocking


  C) PROCESS SCHEDULING

  The kernel updates a linked list made of all the READY processes.
  These processes are put on the list from the highest priority
  level down to the lowest.

  Because priorities are static, the kernel doesn't guarantee that
  all the READY processes will be able to get the CPU after a given
  amount of time. A process with a low priority level won't run until
  there will be no higher priority level processes on the list.

  The kernel is PRE-EMPTIVE, which means it is able to interrupt the
  running process and resume another process even though processes
  don't make system calls. When a process starts running, it owns
  the CPU for a given slice of time called TIMESLICE. Between two
  timeslices, the kernel doesn't schedule any process.


  The kernel changes the running process when


  1)      The running process makes a system call
          that causes a wait on an event
          (ex: I/O operation)

  2)      The running process reaches the end of a
          timeslice and there are other READY processes
          with an equal or higher priority level than
          the current process.


  For a given priority level, processes are scheduled according to
  the ROUND ROBIN algorithm.

  Note:
  The first process is always in a READY state, but its priority level
  is the lowest possible. So, the first process will get the CPU when
  no other process will be ready to run.


  D) INTER-PROCESS COMMUNICATION


  If a process can be considered an independent thread of code, it must be able to exchange 
  information with other processes. Depending on the nature of the data that needs to be exchanged, 
  the programmer will use MESSAGES, SIGNALS, or PIPES to create an Inter-Process Communication.


  1)    Messages

  The Messages mechanism is a simple, easy-to-use tool. It consists of
  sending and receiving ONE BYTE message. Receiving processes will
  wait for a message for a given amount of time or indefinitely. This
  is a synchronous IPC.

  2)    Signals

  The Signals mechanism relates to the way interrupts work. When a process
  initializes an action to a given signal (with the m_Signal system-call), 
  this action will be able to start at any time during the process execution 
  as soon as the signal will be received by the process. 
  This is an asynchronous IPC.

```text
     .---------.            .---------.
     |Process 1|            |Process 2|
     |    -    |            |    -    |
     |    _    |            |Init sig |
     |    _    |            | ACTION  |
     |    _    |            |    _    |
     |    _    |            |    _    |                      .----------.
     |       ===Send Signal====>     ======Soft interrupt===>|  Signal  |
     |    _    |            |    <-------------------.       |  action  |
     |    _    |            |    _    |              |       |    __    |
     |    _    |            |    _    |              |       |    __    |
     `---------'            `---------'              |       `----------'
                                                     |             |
                                                     `-------------'	
```


  3)    Pipes

  A pipe is a buffer where DATA can be transferred in FIFO order,
  in an unidirectional way. Access to the pipe is performed exactly
  in the same way as for file access except that the calling process
  can be suspended (on an empty pipe for "m_Read" or on a full pipe for
  "m_Write"). Although the kernel checks pipe operations, the programmer
  should close unused handles.



  E) PROCESSES SYNCHRONIZATION

  A process can synchronize its execution with other processes. There
  are several ways to realize this.

  The first and simplest method is to wait for a process completion. A
  process creates a child process and waits for its death with the
  "m_Wait" system call. The parent process remains suspended during
  this time.

  The second method uses semaphores. Here, there are no resources to protect
  but just processes to synchronize. A process makes a "m_Waitsem" system-call 
  on a semaphore previously created with an initial count value equal to ZERO. 
  Consequently, the "m_Wait" system call suspends the calling process. This 
  process will be resumed when a "m_Sigsem" system call will be made in another process.

  The third method uses messages. The processes are synchronized with message receipts.
  Pipes could be used too.

  Generally speaking, each system call that is able to suspend or resume a process
  could be used as a synchronization mechanism.


----------------------------------------------------------------------------------


  INPUT / OUTPUT
  -------------
  A) TERMINOLOGY

        SESSION

        a session is a virtual screen and keyboard. When a process
        is created, it is always attached to a given session. The
        kernel manages 10 sessions, but at a given instant, only
        one session is active, that is, owns physical screen and
        keyboard resources.

```text
                  .-----------.
               .-----------.3 |
            .-----------.2 |  |
            |         1 |  |  |
            |  Session  |  |--'
            |           |--'         .-----------.
            `-----------'            |   Active  |
                                     |           |
                                     |  Session  |
                                     `-----------'
```


        This session mechanism has been implemented in order to
        clearly separate processes' standard Input/Output. A session
        remains valid while there is at least one process running
        in this session. When the last process terminates in a given
        session, the session becomes invalid and all necessary resources
        are released.


        There are THREE session system calls:
        "m_GetSession", "m_GetProcSessionHandle", "m_ChgActivSession".
        they are used respectively to attach a new session to a given
        process, to get a session handle attached to a given process, and
        to change the active session.


        The User can change the active session from the keyboard
        by typing SHIFT+Fi, where "i" is the chosen session number
        (called session handle too).


        This allows to start a user application through the spy
        interpreter in a given session and to switch to another
        session in order to control how the application works.



        FILE SYSTEM

        TURBIX allows to use up to 255 different files at the same time
        (set "files = 255" in the config.sys). The file system manages
        concurrent access to a same file from several processes. In order
        to prevent multi-access when critical system calls are used (write
        operations), processes will lock and unlock files.


  B) OVERVIEW

        Input/Output are managed on THREE levels

        1 - Application
        2 - Stream
        3 - Device

  1)    The application level corresponds to system calls. These system
        calls are the same whatever the kind of object (pipe, file, or session)

  2)    The stream level is an internal level that switches the user operation
        to a specific code that manages a class of objects (called STREAM). The
        kernel manages 3 stream classes: PIPE, FILE, and SESSION. A stream connects
        a process to a given object.

  5)    The device level performs I/O requests for an object belonging
        to a given stream class.

  Each process owns an internal table used to manage Input/Output.
  A valid entry in this table corresponds to an active stream; when
  a process wants to use open, pipe, or create I/O system calls, the
  file system initializes internal variables and returns an index
  (or handle - or file descriptor) in the table. This handle is the
  link between the process and the object to use.

  The 3 first slots in the table (handle 0, 1, 2) are automatically
  initialized when the process is created.

      handle 0 refers to the STANDARD INPUT
      handle 1 refers to the STANDARD OUTPUT
      handle 2 refers to the STANDARD ERROR


  When the kernel calls the user application through the "umain"
  entry point, the standard input, output, and error refer to the
  initial session: S0

  All child processes inherit objects opened by the parent process.
  The kernel updates internal object use counters; an object will
  remain available while its corresponding counter will be positive.


  Note: When a child process inherits an open object, both processes
  (child and parent) use the same stream, in other words, they share
  not only the object but the object reference variable too (for
  example: if the object is a file, child and parent processes will
  share the same file pointer)


----------------------------------------------------------------------------------


  WINDOWS
  -------

  TURBIX includes a set of window functions close to the UNIX "curs"
  functions. These functions are useful to build multitasking
  window applications.


  A) INTRODUCTION
  A window is a screen area that can be managed independently
  from other parts of the screen. It is composed of variables which
  define how the window works, its position on the screen, the current
  position in the window, an internal buffer that corresponds to the
  window contents etc...(see "win.h" for further details about the
  window structure).


  B) ENTER WINDOWS MODE
  Before using any Windows functions, the session where windows will be
  hosted has to be initialized in windows mode. The "m_initscr" function
  performs this initialization.

  All sessions can be set to windows mode at the same time.


  C) WINDOW CREATION
  When the session works in windows mode, the "m_newwin" function is
  used to define window parameters. The "m_box" function draws window
  borders when needed. After calling these functions, a window is
  created but it is not yet visible on the screen. To make a window
  visible, the "m_wpush" function has to be called.


  D) WINDOW FOCUS
  When several windows are pushed on the screen, they all work at the
  same time even though they are overlapped. The last pushed window owns
  the FOCUS, which means it is able to get keyboard input characters. If
  other windows are waiting for input characters, processes attached to
  these windows are suspended until the window will get the focus. The
  "m_wselect" function is used to give the focus to a given window. This
  operation can be performed with the keyboard when typing ALT+TAB to
  give the focus in a round-robin way.


  E) WINDOW REFRESH
  All characters written with window functions are stored in an
  internal buffer. The buffer contents are not automatically written
  to the window location on the screen. This operation is done when
  using the "m_wrefresh" function. This function makes a global refresh
  (the whole buffer contents) or a partial refresh (just what has been
  modified into the window buffer since the last refresh) according to
  the control flag contents in the window structure.

        control FLAGS are:

        W_SCROLLON           enables scrolling
        W_AUTOCRLF           CR LF on the right border or truncate
        W_FOREGRND           Sets the focus to a window
        W_VISIBLE            The window is visible
        W_GLOBALR            refreshes the whole window buffer
        W_ECHO               echo on input

  The programmer could change the flag contents himself, but it is better
  to use window functions to perform this job.


  F) WINDOW DELETION
  A window can be deleted with the "m_delwin" function. Yet, if a process
  is attached to such a window, it remains in the system without standard
  I/O. As a general rule to avoid this, all attached processes to future
  deleted windows will have to be killed.


  G) STANDARD SCREEN
  When the session has been initialized in windows mode, the whole
  screen is considered as a default window called "standard screen";
  this default window is a little bit different from other windows in
  the following ways:

        - Drawing borders is not allowed
        - Pushing or popping this window is not allowed

  Because initializing windows mode implicitly creates the default
  window, it is already visible and doesn't need to be pushed.

  If the standard screen is needed, the "m_initStdscrPTR" function will
  be used to get the standard screen window pointer.


  H) EXIT WINDOWS MODE
  To exit from Windows mode and return to full-screen mode,
  the "m_endwin" function has to be called. It releases all resources
  needed for Windows mode and sets the session to full-screen mode.


DEMO

  

https://github.com/crmathieu/Turbix/assets/6110399/bdfd0efb-2b1b-4ade-9d51-20d1aebbf442


----------------------------------------------------------------------------------


  CONCLUSION
  ----------

  TURBIX is an easy-to-use and powerful tool to understand the new
  concepts introduced by multitasking operating systems. TURBIX
  could be used not only as a pedagogic platform to procure skills
  for students in computer science but for professional applications
  too. The user will find some programming examples in the source
  file "sample.c".


----------------------------------------------------------------------------------

API
---



- m_AdjustPTR            : Adjuste the child process HEAP pointer
- m_Alarm                : Create a delay without stopping the calling process
- m_Beep                 : Create a beep with a given frequency
- m_box                  : Draw box
- m_Chdir                : Change the default directory
- m_ChgActiveSession     : Change the current session
- m_Close                : Close a file, device, or pipe
- m_Countsem             : Get semaphore counter current value
- m_Creat                : Create a file
- m_Creatsem             : Create a semaphore
- m_CursorShape          : Modify the cursor shape
- m_DateTime             : Get the time and date
- m_delwin               : Delete window
- m_Delsem               : Delete a semaphore
- m_Dup                  : Duplicate a file handle
- m_Dup2                 : Duplicate a file handle to a specified handle
- m_endwin               : Stop window mode to a given session
- m_Exec                 : Execute a new process
- m_Flush                : Flush the session video buffer attached to the calling process
- m_Fork                 : Create a child process
- m_Fprintf              : Print formatted output to a file, pipe, or device
- m_Free                 : Release a memory block
- m_Fscanf               : Make formatted input from a file, pipe or device
- m_Getc                 : Read a character from a file, pipe, or device
- m_Getch                : Read a character from STDIN
- m_Getche               : Read character with echo from STDIN
- m_Getcwd               : Get the calling process current working directory
- m_Getpos               : Get the cursor position
- m_GetPriority          : Get the calling process priority
- m_GetProcSessionHandle : Get the session handle corresponding to a given process
- m_Getpid               : Get the calling process ID
- m_GetProcName          : Get the calling process name
- m_Getppid              : Get the calling process parent ID
- m_GetSession           : Attach a new session to the calling process
- m_getyx                : Get current window (x,y) coordinates
- m_Gotoxy               : Change the current cursor position
- m_Gsleep               : Suspend the calling process for "n" ticks
- m_initscr              : Initialize window mode to a given session
- m_initStdscrPTR        : Initialize standard screen pointer
- m_Ioctl                : Control a character device
- m_Kill                 : Send a signal to a process
- m_Lock                 : Lock file access
- m_Lseek                : Change file pointer value
- m_Malloc               : Allocate a memory block from the global heap
- m_Mkdir                : Create directory
- m_Msgclr               : Clear message area
- m_Msgrdv               : Make a rendez vous with process(es)
- m_Msgsync              : Send a message
- m_Msgwait              : Wait for a message
- m_mvwin                : Move window
- m_newwin               : Create window
- m_Nice                 : Reduce the calling process priority
- m_Open                 : Open file or device
- m_Pause                : Wait for any signal
- m_Pipe                 : Create a pipe
- m_Printf               : Print formatted output to STDOUT
- m_Putc                 : Write a character to a file, pipe, or device
- m_Pwaitsem             : Wait for a resource with priority
- m_Read                 : Read a character from a file, pipe, or device
- m_Remove               : Remove a specified file
- m_Resetsem             : Reset a semaphore counter
- m_Scanf                : Make formatted input from STDIN
- m_Seprintf             : Print formatted output to SESSION STDOUT
- m_SessionGetch         : Read a character from the STDIN of a given session
- m_Setcva               : Modify default video attribute
- m_Setdrv               : Change default drive
- m_SetErrHandler        : Set the default critical error handle
- m_SetProcName          : Set process name
- m_Sgetc                : Read a character from a file, pipe, or device with priority
- m_Sgetch               : Read a character from STDIN with priority
- m_Sgetche              : Read character with echo from STDIN with priority
- m_Shutdown             : Exit to DOS
- m_Signal               : Initialize action to perform on signal reception
- m_Sigsem               : Release resource to another process
- m_Sleep                : Suspend the calling process for "n" seconds
- m_Sprintf              : Print formatted output to string
- m_Spy                  : SPY interpreter
- m_Sscanf               : Make formatted input from a string
- m_Tell                 : Get the current file pointer value
- m_touchwin             : Global window refresh
- m_Unlock               : Unlock access file
- m_Wait                 : Wait for the child process completion
- m_Waitsem              : Wait for a resource
- m_Wakeup               : Wake up a suspended process
- m_Write                : Write to a file, pipe, or device
- m_Exit                 : Terminate the calling process
- m_waddch               : Write character window buffer
- m_wautocrlf            : Enable automatic return on the window right side
- m_wclrtobot            : Delete characters from the current position to the end of the window buffer
- m_wclrtoeol            : Delete characters from the current position to the end of the line
- m_wdelch               : Delete current position character
- m_wdeleteln            : Delete the current line
- m_wecho                : Enable echo to window
- m_werase               : Purge window buffer
- m_wgetch               : Read a character from STDIN in window mode
- m_winch                : Get window current position character
- m_winsch               : Insert character to window current position
- m_winsertln            : Insert line
- m_wmove                : Move window current position
- m_wpop                 : Pop window (make it invisible)
- m_wprintw              : Write a formatted string to a window
- m_wpush                : Push window (make it visible)
- m_wrefresh             : Refresh just modified part of a window buffer
- m_wresize              : Redefine a window's size and position
- m_wscroll              : Scroll window buffer
- m_wscanw               : Make formatted input from STDIN to window
- m_wselect              : Select a window as ACTIVE (it gets the keyboard focus)
