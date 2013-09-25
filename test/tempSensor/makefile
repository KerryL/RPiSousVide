# makefile (RPISousVide Temp Sensor Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = tempSensorTest

# Directories in which to search for source files
DIRS = \
	. \
	.src/

# Source files
#SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
SRC=.src/temperatureSensor.cpp \
	tempSensorTest.cpp

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean

all: $(TARGET)

copy:
	$(MKDIR) .src/
	cp ../../src/temperatureSensor.cpp .src/

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
