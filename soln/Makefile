# makefile for building cs341 'tutor' programs using the
# tutor VM  and "make clean"

# system directories needed for compilers, libraries, header files--
# assumes the environment variables SAPC_TOOLS and SAPC_GNUBIN
# have been set up by the ulab module

PC_LIB = $(SAPC_TOOLS)/lib
PC_INC = $(SAPC_TOOLS)/include

# We require warning free ansi compatible C:
#
CFLAGS = -g -Wall -Wstrict-prototypes -Wmissing-prototypes \
		-Wno-uninitialized -Wshadow -ansi \
		-D__USE_FIXED_PROTOTYPES__ 

# for SAPC builds
PC_CC   = $(SAPC_GNUBIN)/i386-gcc
PC_CFLAGS = $(CFLAGS) -I$(PC_INC)
PC_AS   = $(SAPC_GNUBIN)/i386-as
PC_LD   = $(SAPC_GNUBIN)/i386-ld
PC_NM   = $(SAPC_GNUBIN)/i386-nm

# File suffixes:
# .c	C source (often useful both for UNIX and SAPC)
# .s 	assembly language source (gnu as for protected mode 486)
# .o    relocatable machine code, initialized data, etc., for UNIX
# .opc  relocatable machine code, initialized data, etc., for SAPC
# .lnx  executable image (bits as in memory), for SA PC (Linux a.out format)
# .syms text file of .exe's symbols and their values (the "symbol table")
# .usyms text file of UNIX executable's symbols

PC_OBJS = tutor.opc slex.opc tickpack.opc
UNIX_OBJS = tutor.o slex.o


# PC executable--tell ld to start code at 0x1000e0, load special startup
# module, special PC C libraries--
# Code has 0x20 byte header, so use "go 100100"
all: tutor.lnx test_tickpack.lnx

test_tickpack.lnx: test_tickpack.opc tickpack.opc
	$(PC_LD) -N -Ttext 1000e0 -o test_tickpack.lnx \
		$(PC_LIB)/startup0.opc $(PC_LIB)/startup.opc \
		test_tickpack.opc tickpack.opc $(PC_LIB)/libc.a
	rm -f syms;$(PC_NM) -n test_tickpack.lnx>test_tickpack.syms; \
		ln -s test_tickpack.syms syms

tickpack.opc: tickpack.c tickpack.h
	$(PC_CC) $(PC_CFLAGS) -c -o tickpack.opc tickpack.c

slex.opc: slex.c
	$(PC_CC) $(PC_CFLAGS) -c -o slex.opc slex.c
                
test_tickpack.opc: test_tickpack.c tickpack.h
	$(PC_CC) $(PC_CFLAGS) -c -o test_tickpack.opc test_tickpack.c

tutor.lnx: cmds.opc $(PC_OBJS) \
		$(PC_LIB)/startup0.opc $(PC_LIB)/startup.opc $(PC_LIB)/libc.a
	$(PC_LD) -N -Ttext 1000e0 -o tutor.lnx \
		$(PC_LIB)/startup0.opc $(PC_LIB)/startup.opc \
		$(PC_OBJS) cmds.opc $(PC_LIB)/libc.a
	rm -f syms;$(PC_NM) -n tutor.lnx>tutor.syms;ln -s tutor.syms syms

# this rule allows you to build your own cmds.opc--
cmds.opc: cmds.c slex.h
	$(PC_CC) $(PC_CFLAGS) -c -o cmds.opc cmds.c


tutor.opc: tutor.c slex.h
	$(PC_CC) $(PC_CFLAGS) -c -o tutor.opc tutor.c

$(PROJDIR)/slex.opc: slex.c slex.h
	$(PC_CC) $(PC_CFLAGS) -c -o slex.opc slex.c

clean:
	rm -f *.o *.opc *.syms *.usyms *.lnx tutor core syms usyms

spotless: clean
	rm -f *~
