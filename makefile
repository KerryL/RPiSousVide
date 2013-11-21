# makefile (Raspberry Pi Sous Vide Controller)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = sousVide

# Directories in which to search for source files
DIRS = \
	src \
	src/utilities \
	src/logging \
	src/rpi

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(MKDIR) $(BINDIR)
	$(CC) $(OBJS) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

$(OBJDIR)%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(BINDIR)$(TARGET)
