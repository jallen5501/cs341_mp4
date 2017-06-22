/*ear::q:q******************************************************************
*
*   file:     cmds.c
*   bob wilson: modified for mp3 on tutor VM 5/24/2016
*   author:   betty o'neil
*   date:     dim dark past
*
*/

/* the Makefile arranges that #include <..> searches in the right
   places for these headers-- */
#include <stdio.h>
#include <serial.h>
#include <lp.h>
#include "slex.h"
#include "tickpack.h"

/*#define DEBUGLPT1*/
/*#define DEBUGCOM1*/

unsigned char timeout=1;
/*===================================================================*
*
*   Command table for tutor program - an array of structures of type cmd
*   - for each command provide the token, the function to call when 
*   that token is found, and the help message.
*
*   slex.h contains the typdef for struct cmd, and declares the 
*   cmds array as extern to all the other parts of the program.
*   Code in slex.c parses user input command line and calls the
*   requested semantic action, passing a pointer to the cmd struct
*   and any arguments the user may have entered.
*
*===================================================================*/

PROTOTYPE int quit(Cmd *cp, char *arguments);
PROTOTYPE int mem_display(Cmd *cp, char *arguments);
PROTOTYPE int mem_set(Cmd *cp, char *arguments);
PROTOTYPE int help(Cmd *cp, char *arguments);
PROTOTYPE int test(Cmd *cp, char *arguments);
PROTOTYPE int timeon ( Cmd *cp, char *arguments);
PROTOTYPE int timeoff ( Cmd *cp, char *arguments);
PROTOTYPE void tick_print(void);
PROTOTYPE static void callback(void);
/* command table */

Cmd cmds[] = {	{"test", test, "test <LPT1> or test <COM1>"}, 
	     {"timeon", timeon, "Timer On: timeon <interval in secs>"},
             {"timeoff", timeoff, "Timer Off: timeoff"},
             {"md",  mem_display, "Memory display: md <addr>"},
             {"ms", mem_set, "Memory set: ms <addr>"},
             {"h", help, "Help: h <cmd> or h (for all)"},
             {"q",  quit, "Quit" },
             {NULL, NULL,  NULL}};   /* null cmd to flag end of table */

static int timer_on;   		/* flag for timer int on */
static int ticks;               /* tick count to display */
static int i;			/* used in callback function */
/*===================================================================*
*               command                 routines
*
*   Each command routine is called with 2 args, the remaining
*   part of the line to parse and a pointer to the struct cmd for this
*   command. Each returns 0 for continue or 1 for all-done.
*
*===================================================================*/

/*===================================================================
*     timeon: timer interrupts every interval seconds and displays count
*
*====================================================================*/

int test (Cmd *cp, char *arguments) {

	char entry[50];
	sscanf(arguments, "%s", entry);				/* entry-> argument */

	if (!strcmp(entry, "LPT1")) {

		byte OGcontrol, OGportdata, portdata;
		OGportdata=inpt(LPT1_BASE+LP_DATA);		/* saves original port data */
		OGcontrol=inpt(LPT1_BASE+LP_CNTRL);		/* saves original control data */

		printf("Normal Read/Write mode\n");
		outpt(LPT1_BASE+LP_CNTRL, LP_PINITP);		/* no device is connected to LPT1 */		
		outpt(LPT1_BASE+LP_DATA, 0xff);			/* write ff to data register */	
		portdata=inpt(LPT1_BASE+LP_DATA);
#ifdef DEBUGLPT1
                printf("%c\n", portdata);                       /* should print test char */
#endif
		portdata==0xff ? printf("DATA: %x OK\n", portdata): printf("ERROR in LPT1\n");

		outpt(LPT1_BASE+LP_DATA, 0x00);			/* write 00 to data register */  
		portdata=inpt(LPT1_BASE+LP_DATA);

		portdata==0x00 ? printf("DATA: 00 OK\n\n\n") : printf("ERROR in LPT1\n");

		printf("Set in Bi-directional mode now\n");
		outpt(LPT1_BASE+LP_CNTRL, LP_PDIR);		/* enables bidirectional */

		portdata=inpt(LPT1_BASE+LP_DATA);		/* reading from data port when bidirection is on */
#ifdef DEBUGLPT1
                printf("%x\n", portdata);               	/* should print "ff" */
#endif
		portdata==0xff ? printf("DATA: %x OK\n\n\n", portdata) : printf("ERROR in LPT1\n");

		outpt(LPT1_BASE+LP_CNTRL, OGcontrol);		/* return to original control status */
		outpt(LPT1_BASE+LP_DATA, OGportdata);		/* return original port data */
	}	

	else if (!strcmp(entry, "COM1")) {

		byte OGmsr, OGmcr;				/* used to save original values in register */
		byte testmsrbefore, testmsrafter, testmsr, testmcr, testmsragain;

		OGmsr=inpt(COM1_BASE+UART_MSR);			/* original status of MSR register */
		OGmcr=inpt(COM1_BASE+UART_MCR);			/* original status of MCR register */ 

		outpt(COM1_BASE+UART_MCR, 0x00);		/* ensures loopback mode is not on */
		printf("Not in loopback\n");

		testmsrbefore=inpt(COM1_BASE+UART_MSR);
#ifdef DEBUGCOM1
		printf("%x\n",testmsrbefore);
		printf("%x\n",inpt(COM1_BASE+UART_MSR));
#endif
		printf("Write 0x0f to MCR\n");
		outpt(COM1_BASE+UART_MCR, 0x0f);	/* write 0x0f to mcr */
		testmsrbefore&=0xf0;			/* only the first 4 bits are important for diagnostic, last 4 bits is hardware dependent */
		testmsrafter=inpt(COM1_BASE+UART_MSR);	
#ifdef DEBUGCOM1	
		printf("%x\n",testmsrafter);
#endif

		testmsrbefore==testmsrafter ?  printf("MSR: %x OK\n\n\n",testmsrafter) : printf("ERROR in COM1\n\n\n");  

		outpt(COM1_BASE+UART_MCR, 0x1f);	/* Turn on loopback mode */
		printf("In loopback now.\n");

		testmcr=inpt(COM1_BASE+UART_MCR);

		testmcr==0x1f ? printf("MCR: %x OK\n",testmcr) : printf("ERROR\n");	

#ifdef DEBUGCOM1
		printf("%x\n", testmcr);
		printf("%x\n", testmsr);
#endif

		testmsr=testmsragain=inpt(COM1_BASE+UART_MSR);
		testmcr=inpt(COM1_BASE+UART_MCR);
#ifdef DEBUGCOM1
		printf("%x %x\n",testmcr, testmsr); 
#endif
		testmcr&=0x0f;
		testmsr=testmsr>>4;

		testmcr==testmsr ? printf("MSR: %x OK\n",testmsragain) : printf("ERROR\n");

		testmsr=inpt(COM1_BASE+UART_MSR);
#ifdef DEBUGCOM1
		printf("%x\n",testmsr);
#endif 

		testmsr==0xf0 ? printf("MSR: %x OK\n",testmsr) : printf("ERROR\n");


		outpt(COM1_BASE+UART_MCR, UART_MCR_LOOP);	/* Turn on loopback mode with a slightly different byte */
								/* nearly the same behavior should be observed */
		inpt(COM1_BASE+UART_MCR)==UART_MCR_LOOP ? printf("MCR: 10 OK\n") : printf("ERROR\n");

		testmsr=testmsragain=inpt(COM1_BASE+UART_MSR);
		testmcr=inpt(COM1_BASE+UART_MCR);
#ifdef DEBUGCOM1
		printf("%x %x\n",testmcr, testmsr); 
#endif 
		testmcr&=0x0f;
		testmsr=testmsr>>4;

		testmcr==testmsr ? printf("MSR: 0f OK\n") : printf("ERROR\n");

		testmsr=inpt(COM1_BASE+UART_MSR);
#ifdef DEBUGCOM1
		printf("%x\n",testmsr); 
#endif
		testmsr==0x00 ? printf("MSR: 00 OK\n") : printf("ERROR\n");

  		while (timeout && !set_timer(&callback, 5, 5)) {  
			outpt(COM1_BASE+UART_MSR, 0xaa);	/* if COM1 is not working correctly, could get stuck in loop */
			printf("Loop Test Data Passes.\n");
			break;
		 }
		printf("Not in loopback now.\n");

		outpt(COM1_BASE+UART_MSR, OGmsr);		/* restore original msr register status */
		outpt(COM1_BASE+UART_MCR, OGmcr);		/* restore original mcr register status */
	}	
	else	printf("bad input\n");
	return 0;
}


static void callback() {
	if (++i==5) {	
		timeout=0;
		printf("TIMEOUT\n");	
	}
} 


int timeon (Cmd *cp, char *arguments)
{
  unsigned int interval;

  if ((sscanf(arguments, "%d", &interval))!=1) {
    printf ("Proper usage : %s\n", cp->help);
    return 0; 
  }

  if(timer_on) {
    printf("%s", "timer already on\n");
    return 0;
  }
  set_timer(&tick_print, interval, interval);   /* continuous */
  timer_on = 1;
  ticks = 0;
  printf("%s", "timer on\n");
  return 0;
}

void tick_print(void)
{
  ticks++;
  printf("(%d)\n", ticks);
}

/*=====================================================================
*
*     timeoff: shut down timer and stop printing of count
*
*======================================================================*/

int timeoff (Cmd *cp, char *arguments)
{
  stop_timer(&tick_print);
  timer_on = 0;
  ticks = 0;
  printf("%s", "timer off\n");
  return 0;
}


/*===================================================================*
*		command			routines		     
*
*   Each command routine is called with 2 args, the remaining 
*   part of the line to parse and a pointer to the struct cmd for this 
*   command. Each returns 0 for continue or 1 for all-done.  
*
*===================================================================*/

int quit(Cmd *cp, char *arguments)
{
  if (timer_on) {
     shutdown_ticks();
     timer_on = 0;
  }
  return 1;			/* all done flag */
}

/*===================================================================*
*
*   mem_display: display contents of a single byte in hex
*
*/

int mem_display(Cmd *cp, char *arguments)
{
  unsigned int address, i;

  if (sscanf(arguments, "%x", &address) == 1) {
    /* print the address in hex */
    printf("%08x    ", address);
    /* print 16 locations as two hex digits per memory byte */ 
    for (i = 0; i < 16; i++)
      printf("%02x ", *(unsigned char *)(address + i));
    /* print 16 locations as one ascii coded character per byte */
    for (i = 0; i < 16; i++)
      printf("%c",
      (*(unsigned char *)(address + i) >= ' ' &&
       *(unsigned char *)(address + i) <= '~') ?
       *(unsigned char *)(address + i) : '.');
    /* and print end of line */
    printf("\n");
  } else
    printf("        help message: %s\n", cp->help);

  return 0;                   /* not done */
}

/********************************************************/

/* mem_set : ms <hexaddress> <new_val> 
 *     stores byte new_val at address, both are given 
 *     in 'arguments'
*/

int mem_set(Cmd *cp, char *arguments) 
{
  unsigned int address, value;

  if (sscanf(arguments, "%x %x", &address, &value) == 2) {
    if(value < 0x100)
       *(unsigned char *)address = value;
    else
       *(unsigned int *)address = value;

    printf("OK\n");
  } else
    printf("        help message: %s\n", cp->help);

  return 0;
}

/****************************************************/

/* help : display a help message of the 'arguments'
 *      command, or list all commands if no arg.
*/

int help(Cmd *cp, char *arguments) 
{
  int stat, cmd_num, pos;

  /* reuse code that matches command token-- */
  stat = slex(arguments, cmds, &cmd_num, &pos);
  if (stat < 0) {		/* if no command token found in args */
    printf("     cmd    help message\n");
    printf("     ---    ------------\n");
    for (cp = cmds; cp->cmdtoken; cp++)   /* show all commands */
      printf("%8s    %s\n",cp->cmdtoken, cp->help);
  } else {			/* found command token, just print that one */
    printf("%s\n", cmds[cmd_num].help);
    return 0;
  }
  return 0;
}
