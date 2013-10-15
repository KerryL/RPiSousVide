# makefile (RPISousVide Test Applications)
#

# Locations of test applicaton makefiles
TESTDIRS = \
	gpio \
	sockets/server \
	sockets/client \
	logger \
	config \
	derivativeFilter \
	autoTuner \
	json \
	gnuPlot \
	uartTempSensor
#	tempSensor

.PHONY: all clean

all: $(TARGET)
	set -e; for dir in $(TESTDIRS) ; do $(MAKE) -C $$dir all; done

clean:
	set -e; for dir in $(TESTDIRS) ; do $(MAKE) -C $$dir clean; done
