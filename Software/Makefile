# Command-line client
ifeq ($(OS),Windows_NT)
	EXE_EXT = .exe
else
	EXE_EXT =
endif
CMDLINE = flash2verify_v1.0_am29f016b$(EXE_EXT)

# Timestamp fix for Linux
ifeq ($(OS),Windows_NT)
	TIMESTAMP_FIX = 
else
	UNAME_S := $(shell uname -s)
   ifeq ($(UNAME_S),Linux)
		TIMESTAMP_FIX = -D_POSIX_C_SOURCE=199309L
   endif
endif

# By default, build the firmware and command-line client
all: $(CMDLINE) $(CMDLINE2)

# One-liner to compile the command-line client
$(CMDLINE): flash2verify_v1.0_am29f016b.c setup.c rs232/rs232.c
	gcc -O -std=c99 -Wall $(TIMESTAMP_FIX) $^ -o $@
	
# Housekeeping if you want it
clean:
	$(RM) $(CMDLINE)
