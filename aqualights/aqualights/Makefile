SOURCES := $(shell find -name '*.cc')
OBJECTS := $(SOURCES:%.cc=%.o)
INCLUDE := ../aqualib/ ../../
AQUALIB_LIBRARY := ../aqualib/aqualib.a
WS2811_LIBRARY := ../../libws2811.a
BINARY := aqualights

CXX := g++
CXXFLAGS := $(INCLUDE:%=-I%)

$(BINARY): $(OBJECTS) $(LIBRARY)
	$(CXX) -o $(BINARY) $(OBJECTS) $(AQUALIB_LIBRARY) $(WS2811_LIBRARY)

%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(AQUALIB_LIBRARY):
	make -C ../aqualib

$(WS2811_LIBRARY):
	scons -C ../..

clean:
	rm -f $(OBJECTS) $(BINARY)

.PHONY: clean $(AQUALIB_LIBRARY) $(WS2811_LIBRARY)
