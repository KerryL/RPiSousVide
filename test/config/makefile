# makefile (RPISousVide Config File Test)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = configTest

# Directories in which to search for source files
DIRS = \
	.

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp)) \
	.src/sousVideConfig.cpp \
	.src/configFile.cpp \
	.src/autoTuner.cpp \
	.src/matrix.cpp

# Object files
OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))

.PHONY: all clean copy

all: $(TARGET)

copy:
	$(MKDIR) .src/
	cp ../../src/sousVideConfig.cpp .src/
	cp ../../src/configFile.cpp .src/
	cp ../../src/autoTuner.cpp .src/
	cp ../../src/matrix.cpp .src/

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
