# makefile (RPISousVide GPIO Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = gpioTest

# Directories in which to search for source files
DIRS = \
	.

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp)) \
	.src/gpio.cpp \
	.src/pwmOutput.cpp \
	.src/timingUtility.cpp

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean

all: $(TARGET)

copy:
	echo $(SRC)
	echo $(OBJS)
	$(MKDIR) .src/
	cp ../../src/gpio.cpp .src/
	cp ../../src/pwmOutput.cpp .src/
	cp ../../src/timingUtility.cpp .src/

$(TARGET): copy $(OBJS)
	$(MKDIR) $(BINDIR)
	$(CC) $(OBJS) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

$(OBJDIR)%.o: %.cpp copy
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(BINDIR)$(TARGET)
	$(RM) -r .src/
