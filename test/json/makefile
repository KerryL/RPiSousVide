# makefile (RPISousVide JSON Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = jsonTest

# Directories in which to search for source files
DIRS = \
	.

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp)) \
	$(foreach dir, $(DIRS), $(wildcard $(dir)/*.c)) \
	.src/cJSON.c

# Object files
CPPOBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))
#OBJS = $(CPPOBJS:.c=.o)
OBJS = $(CPPOBJS:.c=__C.o)

.PHONY: all clean copy

all: $(TARGET)

copy:
	$(MKDIR) .src/
	cp ../../src/cJSON.cpp .src/cJSON.c

$(TARGET): copy $(OBJS)
	$(MKDIR) $(BINDIR)
	$(CC) $(OBJS) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

$(OBJDIR)%.o: %.cpp copy
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)%__C.o: %.c copy
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(BINDIR)$(TARGET)
	$(RM) -r .src/
