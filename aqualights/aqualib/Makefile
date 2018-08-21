SOURCES := $(shell find -name '*.cc')
OBJECTS := $(SOURCES:%.cc=%.o)
LIBRARY := aqualib.a

CXX = g++
AR = ar
RANLIB = ranlib

$(LIBRARY): $(OBJECTS)
	$(AR) crf $(LIBRARY) $(OBJECTS)
	$(RANLIB) $(LIBRARY)

%.o: %.cc %.h
	$(CXX) -o $@ -c $<

clean:
	rm -f $(OBJECTS) $(LIBRARY)

.PHONY: clean
