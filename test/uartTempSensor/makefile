# makefile (RPISousVide UART Temp Sensor Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = uartTempSensorTest

# Directories in which to search for source files
DIRS = \
	.

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp)) \
	.src/ds18b20UART.cpp \
	.src/uartOneWireInterface.cpp \
	.src/timingUtility.cpp

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean copy

all: $(TARGET)

copy:
	$(MKDIR) .src/
	cp ../../src/ds18b20UART.cpp .src/
	cp ../../src/uartOneWireInterface.cpp .src/
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
