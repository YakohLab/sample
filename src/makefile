CXX=g++
LD=g++
#CXXFLAGS=-std=c++0x `pkg-config --cflags gtkmm-3.0`
#LDFLAGS=`pkg-config --libs gtkmm-3.0`
CXXFLAGS=-std=c++11 -Wall -Wconversion `pkg-config --cflags gtkmm-2.4 gtkglext-1.0`
LDFLAGS=`pkg-config --libs gtkmm-2.4 gtkglext-1.0` -lglut
PROGRAM=sample
SRCS:=$(wildcard *.cpp)
OBJS:=$(SRCS:.cpp=.o)
.SUFFIXES: .o .cpp
.PHONY: all clean

all: $(PROGRAM)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

$(PROGRAM): $(OBJS)
	$(LD) -o $(PROGRAM) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(OBJS)
