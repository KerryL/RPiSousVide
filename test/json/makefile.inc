# makefile.inc (RPiSousVide JSON Test)
# This file contains all of the environment definitions
# common to each of the makefiles within the project. 
#

# Include directories that are not already on the path
# DO NOT include the -I prefix to these paths - it will
# be added automatically
INCDIRS_TEMP = \
	/usr/local/include \
	$(CURDIR)/../../src

INCDIRS = $(addprefix -I,$(INCDIRS_TEMP))

# Library directories that are not already on the path
# DO NOT include the -L prefix to these paths - it will
# be added automatically
LIBDIRS_TEMP = \
	/usr/local/lib

LIBDIRS = $(addprefix -L,$(LIBDIRS_TEMP))

# Libraries to link against
# DO NOT include the -l prefix to these libraries - it
# will be added automatically
LIBS_TEMP = \
	

LIBS = $(addprefix -l,$(LIBS_TEMP))

# Static libraries to be build before the executable
# MUST be listed in order of dependence (i.e. first
# library must not be needed by other libraries and
# it must need information contained in the following
# libraries).
PSLIB = \
	

# Compiler to use
CC = arm-unknown-linux-gnueabi-g++
#CC = g++

# Archiver to use
AR = arm-unknown-linux-gnueabi-ar rcs
RANLIB = arm-unknown-linux-gnueabi-ranlib

# Compiler flags
CFLAGS = -g -Wall -Wextra -Werror -pedantic $(INCDIRS)

# Linker flags
LDFLAGS = $(LIBDIRS) $(LIBS)

# Object file output directory
OBJDIR = $(CURDIR)/.obj/

# Binary file output directory
BINDIR = $(CURDIR)/../bin/

# Library output directory
LIBOUTDIR = $(CURDIR)/.lib/

# Method for creating directories
MKDIR = mkdir -p

# Method for removing files
RM = rm -f
