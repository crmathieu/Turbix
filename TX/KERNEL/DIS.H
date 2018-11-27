/* Dos Internal Structures - D.I.S. 

INT 21 - DOS 2+ internal - GET LIST OF LISTS
	AH = 52h
	Return: ES:BX -> DOS list of lists
Note:	partially supported by OS/2 v1.1 compatibility box (however, most
	pointers are FFFFh:FFFFh, LASTDRIVE is FFh, and the NUL header "next"
	pointer is FFFFh:FFFFh).

Format of List of Lists:
Offset	Size	Description
 -12	WORD	(DOS 3.1-3.3) sharing retry count (see AX=440Bh)
 -10	WORD	(DOS 3.1-3.3) sharing retry delay (see AX=440Bh)
 -8	DWORD	(DOS 3.x) pointer to current disk buffer
 -4	WORD	(DOS 3.x) pointer in DOS code segment of unread CON input
		when CON is read via a handle, DOS reads an entire line,
		  and returns the requested portion, buffering the rest
		  for the next read.  0000h indicates no unread input
 -2	WORD	segment of first memory control block
 00h	DWORD	pointer to first DOS Drive Parameter Block (see AH=32h)
 04h	DWORD	pointer to list of DOS file tables (see below)
 08h	DWORD	pointer to CLOCK$ device driver, resident or installable
 0Ch	DWORD	pointer to actual CON device driver, resident or installable

---DOS 2.x---
 10h	BYTE	number of logical drives in system
 11h	WORD	maximum bytes/block of any block device
 13h	DWORD	pointer to first disk buffer (see below)
 17h 18 BYTEs	actual NUL device driver header (not a pointer!)
		This is the first device on DOS's linked list of device
		drivers. (see below for format)
---DOS 3.0---
 10h	BYTE	number of block devices
 11h	WORD	maximum bytes/block of any block device
 13h	DWORD	pointer to first disk buffer (see below)
 17h	DWORD	pointer to array of current directory structures (see below)
 1Bh	BYTE	value of LASTDRIVE command in CONFIG.SYS (default 5)
 1Ch	DWORD	pointer to STRING= workspace area
 20h	WORD	size of STRING area (the x in STRING=x from CONFIG.SYS)
 22h	DWORD	pointer to FCB table
 26h	WORD	the y in FCBS=x,y from CONFIG.SYS
 28h 18 BYTEs	actual NUL device driver header (not a pointer!)
		This is the first device on DOS's linked list of device
		drivers. (see below for format)
---DOS 3.1-3.3---
 10h	WORD	maximum bytes/block of any block device
 12h	DWORD	pointer to first disk buffer (see below)
 16h	DWORD	pointer to array of current directory structures (see below)
 1Ah	DWORD	pointer to FCB table (if CONFIG.SYS contains FCBS=)
 1Eh	WORD	number of protected FCBs (the y in FCBS=x,y)
 20h	BYTE	number of block devices
 21h	BYTE	value of LASTDRIVE command in CONFIG.SYS (default 5)
 22h 18 BYTEs	actual NUL device driver header (not a pointer!)
		This is the first device on DOS's linked list of device
		drivers. (see below for format) (see also INT 2F/AX=122Ch)
 34h	BYTE	number of JOIN'ed drives
---DOS 4.x---
 10h	WORD	maximum bytes/block of any block device
 12h	DWORD	pointer to disk buffer info (see below)
 16h	DWORD	pointer to array of current directory structures (see below)
 1Ah	DWORD	pointer to FCB table (if CONFIG.SYS contains FCBS=)
 1Eh	WORD	number of protected FCBs (the y in FCBS=x,y)
 20h	BYTE	number of block devices
 21h	BYTE	value of LASTDRIVE command in CONFIG.SYS (default 5)
 22h 18 BYTEs	actual NUL device driver header (not a pointer!)
		This is the first device on DOS's linked list of device 
		drivers. (see below for format) (see also INT 2F/AX=122Ch)
 34h	BYTE	number of JOIN'ed drives???
 35h	WORD	pointer within IBMDOS code segment to list of special program
		names (see below)
 37h	DWORD	pointer to FAR routine for resident IFS utility functions
		(see below)
		may be called by any IFS driver which does not wish to
		service functions 20h or 24h-28h itself
 3Bh	DWORD	pointer to chain of IFS (installable file system) drivers
 3Fh	WORD	the x in BUFFERS x,y (rounded up to multiple of 30 if in EMS)
 41h	WORD	the y in BUFFERS x,y
 43h	BYTE	boot drive (1=A:)
 44h	BYTE	01h if 80386+, 00h otherwise???
 45h	WORD	extended memory size in K

Format of memory control block (see also below):
Offset	Size	Description
 00h	BYTE	block type: 5Ah if last block in chain, otherwise 4Dh
 01h	WORD	PSP segment of owner, 0000h if free, 0008h if belongs to DOS
 03h	WORD	size of memory block in paragraphs
 05h  3 BYTEs	unused
---DOS 2.x,3.x---
 08h  8 BYTEs	unused
---DOS 4.x---
 08h  8 BYTEs	ASCII program name if PSP memory block, else garbage
		null-terminated if less than 8 characters

Notes:	under DOS 3.1+, the first memory block is the DOS data segment,
	containing installable drivers, buffers, etc.
	under DOS 4.x it is divided into subsegments, each with its own memory
	control block (see below), the first of which is at offset 0000h

Format of STARLITE memory control block:
Offset	Size	Description
 00h	BYTE	block type: 5Ah if last block in chain, otherwise 4Dh
 01h	WORD	PSP segment of owner, 0000h if free, 0008h if belongs to DOS
 03h	WORD	size of memory block in paragraphs
 05h	BYTE	unused
 06h	WORD	segment address of next memory control block (0000h if last)
 08h	WORD	segment address of previous memory control block or 0000h
 0Ah  6 BYTEs	reserved

Format of DOS 4.x data segment subsegment control blocks:
Offset	Size	Description
 00h	BYTE	subsegment type (blocks typically appear in this order)
		"D"  device driver
		"E"  device driver appendage
		"I"  IFS (Installable File System) driver
		"F"  FILES=  control block storage area (for FILES>5)
		"X"  FCBS=   control block storage area, if present
		"C"  BUFFERS EMS workspace area (if BUFFERS /X option used)
		"B"  BUFFERS=  storage area
		"L"  LASTDRIVE=  current directory structure array storage area
		"S"  STACKS=  code and data area, if present (see below)
 01h	WORD	paragraph of subsegment start (usually the next paragraph)
 03h	WORD	size of subsegment in paragraphs
 05h  3 BYTEs	unused
 08h  8 BYTEs	for types "D" and "I", base name of file from which the driver
		  was loaded (unused for other types)

Format of data at start of STACKS code segment (if present):
Offset	Size	Description
 00h	WORD	???
 02h	WORD	number of stacks (the x in STACKS=x,y)
 04h	WORD	size of stack control block array (should be 8*x)
 06h	WORD	size of each stack (the y in STACKS=x,y)
 08h	DWORD	ptr to STACKS data segment
 0Ch	WORD	offset in STACKS data segment of stack control block array
 0Eh	WORD	offset in STACKS data segment of last element of that array
 10h	WORD	offset in STACKS data segment of the entry in that array for
		the next stack to be allocated (initially same as value in 0Eh
		and works its way down in steps of 8 to the value in 0Ch as
		hardware interrupts pre-empt each other)
Note:	the STACKS code segment data may, if present, be located as follows:
    DOS 3.2:	The code segment data is at a paragraph boundary fairly early
		in the IBMBIO segment (seen at 0070:0190h)
    DOS 3.3:	The code segment is at a paragraph boundary in the DOS data
		segment, which may be determined by inspecting the segment
		pointers of the vectors for those of interrupts 02h, 08h-0Eh,
		70h, 72-77h which have not been redirected by device drivers or
		TSRs.
    DOS 4.x:	Identified by sub-segment control block type "S" within the DOS
		data segment.

Format of array elements in STACKS data segment:
Offset	Size	Description
 00h	BYTE	status: 00h=free, 01h=in use, 03h=corrupted by overflow of
		higher stack.
 01h	BYTE	not used
 02h	WORD	previous SP
 04h	WORD	previous SS
 06h	WORD	ptr to word at top of stack (new value for SP). The word at the
		top of the stack is preset to point back to this control block.

*/

/* memory management macros */
#define FP2_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP2_OFF(fp) (*((unsigned *)&(fp)))

struct	ExecStateInfo {
    unsigned int	Reserved;		/*  00h 		*/
    unsigned int	Flags;			/*  02h 		*/
    unsigned int	NameOff;		/*  04h 		*/
    unsigned int	NameSeg;		/*  06h 		*/
    unsigned int	psp;			/*  08h 		*/
    char		    *Start;		    /*  0Ah 		*/
    unsigned long	Size;			/*  0Eh 		*/
};						/*  12h 		*/

struct	LoadInfo {
    unsigned int Environement;		/*  00h 		*/
    char		 *CommandTail;	    /*  02h 		*/
    char		 *Fcb1;		        /*  06h 		*/
    char		 *Fcb2;		        /*  0Ah 		*/
    char		 *Stack;		    /*  0Eh 		*/
    char		 *Start;		    /*  12h 		*/
};						/*  16h 		*/

struct	LoadExecInfo {
    unsigned int Environement;		/*  00h 		*/
    char		 *CommandTail;	    /*  02h 		*/
    char		 *Fcb1;		        /*  06h 		*/
    char		 *Fcb2;		        /*  0Ah 		*/
};						/*  0Eh 		*/

struct	arena {
    unsigned char	signature;		/*  00h 		*/
    unsigned int	owner;			/*  01h 		*/
    unsigned int	size;			/*  03h 		*/
    unsigned char	reserved[3];	/*  05h 		*/
    unsigned char	name[8];		/*  08h 		*/
};						/*  10h 		*/

struct ExeHeader {
    unsigned int	Signature;		/*  00h 		*/
    unsigned int	ExtraByte;		/*  02h 		*/
    unsigned int	Pages;			/*  04h 		*/
    unsigned int	RelocItems;		/*  06h 		*/
    unsigned int	HeaderSize;		/*  08h 		*/
    unsigned int	MinAlloc;		/*  0Ah 		*/
    unsigned int	MaxAlloc;		/*  0Ch 		*/
    unsigned int	InitSS; 		/*  0Eh 		*/
    unsigned int	InitSP; 		/*  10h 		*/
    unsigned int	CheckSum;		/*  12h 		*/
    unsigned int	InitIP; 		/*  14h 		*/
    unsigned int	InitCS; 		/*  16h 		*/
    unsigned int	RelocTable;		/*  18h 		*/
    unsigned int	Overlay;		/*  1Ah 		*/
};						/*  1Ch 		*/

struct DPBStruct {
    unsigned char	DriveNb;		    /*  00h 		*/
    unsigned char	UnitNb; 		    /*  01h 		*/
    unsigned int	BytePerSector;		/*  02h 		*/
    unsigned char	MaxSectPerClust;	/*  04h 		*/
    unsigned char	ClusterSize;		/*  05h 		*/
    unsigned int	NbResSector;		/*  06h 		*/
    unsigned char	NbOfFat;		    /*  08h 		*/
    unsigned int	NbRootEntry;		/*  09h 		*/
    unsigned int	FirstDataSector;	/*  0Bh 		*/
    unsigned int	ClusterMaxi;		/*  0Dh 		*/
    unsigned int	SectorPerFat;		/*  0Fh 		*/
    unsigned int	FirstRootSector;	/*  11h 		*/
    unsigned char	 *DriverPtr;	    /*  13h 		*/
    unsigned char	MediaDescr;		    /*  17h 		*/
    unsigned char	BlockStatus;		/*  18h 		*/
    unsigned char	 *NxtDriverPtr;	    /*  19h 		*/
    unsigned int	FirstFreeCluster;	/*  1Dh 		*/
    unsigned int	FreeClusterNb;		/*  1Fh 		*/
};						/*  21h 		*/

struct InternalFile {
    unsigned int	NbHandle;		/*  00h 		*/
    unsigned int	OpenMode;		/*  02h 		*/
    unsigned char	Attributes;		/*  04h 		*/
    unsigned int	DevInfo;		/*  05h 		*/
    unsigned char	 *DriverPtr;	/*  07h 		*/
    unsigned int	StartCluster;	/*  0Bh 		*/
    unsigned int	Time;			/*  0Dh 		*/
    unsigned int	Date;			/*  0Fh 		*/
    unsigned long	size;			/*  11h 		*/
    unsigned long	offset; 		/*  15h 		*/
    unsigned int	RelCluster;		/*  19h 		*/
    unsigned long	DirEntrySec;	/*  1Bh 		*/
    unsigned char	DirEntryNb;		/*  1Fh 		*/
    unsigned char	 * RedirIfsRec;	/*  19h 		*/
    unsigned char	Reseved [3];	/*  1Dh 		*/
    unsigned char	FileName [11];	/*  20h 		*/
    unsigned char	 * SFTNamePtr;	/*  2Bh 		*/
    unsigned int	NetMachineNb;	/*  2Fh 		*/
    unsigned int	PspOwner;		/*  31h 		*/
    unsigned int	ShareOff;		/*  33h 		*/
    unsigned int	LastCluster;	/*  35h 		*/
    unsigned char	 * IFSDriver;	/*  37h 		*/
};						/*  3Bh 		*/

struct FileTable {
    struct FileTable *NextFileTable;    /*  00h 		*/
    unsigned int	 NbFiles;		    /*  04h 		*/
    struct InternalFile file[1];		/*  06h 		*/
};						/*  ??			*/

struct StackDArray {
    unsigned int	status; 		/*  00h 		*/
    unsigned int	PreviousSP;		/*  02h 		*/
    unsigned int	PreviousSS;		/*  04h 		*/
    unsigned int	NewSP;			/*  06h 		*/
};						/*  08h 		*/

struct StacksInfo {
    unsigned int	Reserved;		    /*  00h 		*/
    unsigned int	NbOfStacks;		    /*  02h 		*/
    unsigned int	DArraySize;		    /*  04h 		*/
    unsigned int	StacksSize;		    /*  06h 		*/
    struct StackDArray	 * DArrayPtr;	/*  08h 		*/
    unsigned int	FirstDArrayOff; 	/*  0Ch 		*/
    unsigned int	LastDArrayOff;		/*  0Eh 		*/
    unsigned int	NextDArrayOff;		/*  10h 		*/
};						/*  12h 		*/

struct DirStruct {
    char		CurPath[67];		/*  00h 		*/
    unsigned char	dummy1; 		/*  44h 		*/
    unsigned char	type1;			/*  45h 		*/
    struct DPBStruct *DPBPtr;	    /*  46h 		*/
    unsigned int	StartCluster;	/*  4Ah 		*/
    unsigned long	dummy2; 		/*  4Ch 		*/
    unsigned int	basic;			/*  50h 		*/
    char		dummy3[7];		    /*  54h 		*/
};						/*  5Bh 		*/

struct DiskBuffer {
    unsigned int	FwdLRUBuffer;	/*  00h 		*/
    unsigned int	BwdBuffer;		/*  02h 		*/
    unsigned char	drive;			/*  04h 		*/
    unsigned char	flags;			/*  05h 		*/
    unsigned long	LogSector;		/*  06h 		*/
    unsigned char	NbCopy; 		/*  0Ah 		*/
    unsigned int	SectorOffset;	/*  0Bh 		*/
    unsigned char	 *DosBPB;		/*  0Dh 		*/
    unsigned int	BufferUseCnt;	/*  11h 		*/
    unsigned char	reserved;		/*  13h 		*/
    unsigned char	FirstData;		/*  14h 		*/
};						/*  ??h 		*/

/* List Of List */
struct LOL {
    unsigned int      FirstArenaSeg;	/*  00h  -02h		*/
    struct DPBStruct  *FirstDPBPtr;	    /*  02h   00h		*/
    struct FileTable  *FileTablePtr;	/*  06h   04h		*/
    char		      *ClockDriver;	    /*  0Ah   08h		*/
    char		      *ConDriver;	    /*  0Eh   0Ch		*/
    unsigned int	  MaxBlockSize;		/*  12h   10h		*/
    struct DiskBuffer *DiskBufferPtr;	/*  14h   12h		*/
    struct DirStruct  *DirStructPtr;	/*  18h   16h		*/
    char		      *FCBTablePtr;	    /*  1Ch   1Ah		*/
    unsigned int	  NbProtFCB;		/*  20h   1Eh		*/
    unsigned char	  NbBlkDev;		    /*  22h   20h		*/
    unsigned char	  LastDrive;		/*  23h   21h		*/
    unsigned char	  NulDriver[18];	/*  24h   22h		*/
    unsigned char	  NbJoinDrive;		/*  36h   34h		*/
};						/*  37h 		*/

struct	PSP {
	unsigned int	Int20;		    /* 00h		*/
	unsigned int	MemSize;	    /* 02h		*/
	unsigned char	dummy1; 	    /* 04h		*/
	unsigned char	cpm[5]; 	    /* 05h		*/
	char 			*TerminateAdr;	/* 0Ah		*/
	char 			*BreakAdr;	    /* 0Eh		*/
	char 			*CriticAdr;	    /* 12h		*/
	unsigned int	ParentPspSeg;	/* 16h		*/
	unsigned char	FileOpen[20];	/* 18h		*/
	unsigned int	EnvSeg; 	    /* 2Ch		*/
	unsigned long	SSSP;		    /* 2Eh		*/
	unsigned int	MaxOpenFile;	/* 32h		*/
	char 			*OpenFileTable; /* 34h		*/
	char 			*PreviousPsp;	/* 38h		*/
	char			dummy2[20];	    /* 3Ch		*/
	char			dispatcher[3];	/* 50h		*/
	char			dummy3[9];	    /* 53h		*/
	char			Fcb1[36];	    /* 5Ch		*/
	char			Fcb2[20];	    /* 6Ch		*/
	char			CmdTail[127];	/* 80h		*/
};

/* Memory Control Block Header */
struct ARENA {
	unsigned char 	Id;
	/*
	 *	"D"  device driver
	 *	"E"  device driver appendage
	 *	"I"  IFS (Installable File System) driver
	 *	"F"  FILES=  control block storage area (for FILES>5)
	 *	"X"  FCBS=   control block storage area, if present
	 *	"C"  BUFFERS EMS workspace area (if BUFFERS /X option used)
	 *	"B"  BUFFERS=  storage area
	 *	"L"  LASTDRIVE=  current directory structure array storage area
	 *	"S"  STACKS=  code and data area, if present (see below)
	 *	"M"  code ou data
	 *	"Z"  dernier bloc
	 */
	unsigned 	owner;      /* adresse subsegment correspondant */
	unsigned 	nPara;      /* taille du block en PARAGRAPHES */
	unsigned char 	res[3];
	unsigned char   progN[8];
};


/* Notes:

    we obtain the DIB using the function 52h of INT 21h. Th DIB address
    is read as ES:(BX-4)
    
    2 ways to obtain the DPB:
     - from the DIB and following the DPB linked list
     - by using function 1Fh and 32h. these 2 functions provide a DPB
     address in DS:BX

		1Fh provides the DPB of the current unit
		32h provides the DPB of the unit specified in the AL register, 
        with the following convention:
		    AL = 0  --> current unit
            AL = 1  --> A, etc ...

    it is advised to use function 32h to retrieve various DPB because this
    interrupt will force a disk access, hence we ensure that all fields will
    be properly initialized (not guaranteed when using the linked list from the DIB)
*/
