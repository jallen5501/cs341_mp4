/******************************************************************
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
#include "slex.h"
#include "tickpack.h"

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
PROTOTYPE int timeon ( Cmd *cp, char *arguments);
PROTOTYPE int timeoff ( Cmd *cp, char *arguments);
PROTOTYPE void tick_print(void);

/* command table */

Cmd cmds[] = { {"timeon", timeon, "Timer On: timeon <interval in secs>"},
             {"timeoff", timeoff, "Timer Off: timeoff"},
             {"md",  mem_display, "Memory display: md <addr>"},
             {"ms", mem_set, "Memory set: ms <addr>"},
             {"h", help, "Help: h <cmd> or h (for all)"},
             {"q",  quit, "Quit" },
             {NULL, NULL,  NULL}};   /* null cmd to flag end of table */

static int timer_on;   		/* flag for timer int on */
static int ticks;               /* tick count to display */

/*===================================================================*
*               command                 routines
*
*   Each command routine is called with 2 args, the remaining
*   part of the line to parse and a pointer to the struct cmd for this
*   command. Each returns 0 for continue or 1 for all-done.
*
*===================================================================*/

/*===================================================================
*
*     timeon: timer interrupts every interval seconds and displays count
*
*====================================================================*/

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
