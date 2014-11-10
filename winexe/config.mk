#################################
# Start BINARY winexe
[BINARY::winexe]
INSTALLDIR = BINDIR
OBJ_FILES = \
                winexe.o \
		service.o \
		async.o \
		winexesvc/winexesvc32_exe.o \
		winexesvc/winexesvc64_exe.o
PRIVATE_DEPENDENCIES = \
                POPT_SAMBA \
                POPT_CREDENTIALS \
                LIBPOPT \
		RPC_NDR_SVCCTL
# End BINARY winexe
#################################

winexe/winexesvc/winexesvc32_exe.c:
	@$(MAKE) -C winexe/winexesvc

winexe/winexesvc/winexesvc64_exe.c:
	@$(MAKE) -C winexe/winexesvc
