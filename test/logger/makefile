# makefile (RPISousVide Logger Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = loggerTest

# Directories in which to search for source files
DIRS = \
	.

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp)) \
	.src/logger.cpp \
	.src/combinedLogger.cpp \
	.src/timeHistoryLog.cpp

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean

all: $(TARGET)

copy:
	$(MKDIR) .src/
	cp ../../src/logger.cpp .src/
	cp ../../src/combinedLogger.cpp .src/
	cp ../../src/timeHistoryLog.cpp .src/

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
