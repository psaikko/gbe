GLFWLIBS = -lGL -lGLU -lGLEW -lglfw
ALLIBS = -lalut -lopenal

CXXFLAGS = --std=c++14 -O3 -Wall -Wextra -MMD -Wno-format-security -Wno-reorder
CPPFLAGS = $(GLFWLIBS) $(ALLIBS)

SRCS=gpu.cpp window.cpp mem.cpp timer.cpp cpu.cpp serial.cpp gbe.cpp sound.cpp openal_output.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(OBJS)
	@echo "Linking gbe"
	@$(CXX) $(OBJS) $(CXXFLAGS) $(CPPFLAGS) -o gbe

clean:
	rm -f *.o *.d

%.o: %.cpp
	@echo "Compiling "$@""
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

-include $(OBJS:.o=.d)