# makefile (RPiSousVide Socket Server Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = serverTest

# Directories in which to search for source files
DIRS = \
	. \
	.src

# Source files
#SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
SRC = serverTest.cpp \
	.src/linuxSocket.cpp

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean

all: $(TARGET)

copy:
	$(MKDIR) .src/
	cp ../../../src/linuxSocket.cpp .src/

$(TARGET): copy $(OBJS)
	$(MKDIR) $(BINDIR)
	$(CC) $(OBJS) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

$(OBJDIR)%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r .src/
	$(RM) -r $(OBJDIR)
	$(RM) $(BINDIR)$(TARGET)