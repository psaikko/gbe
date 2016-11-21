GLFWDIR = glfw
GLFWLIBS = -lGL -lGLU -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXcursor -lXinerama

CXX = g++
CXXFLAGS = -std=c++11 -ggdb -Wall -Wextra -MMD -Wno-format-security
CPPFLAGS = -I$(GLFWDIR)/include -L$(GLFWDIR)/lib $(GLFWLIBS)

SRCS=gpu.cpp window.cpp mem.cpp timer.cpp cpu.cpp serial.cpp gbe.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(OBJS)
	@echo "Linking gbe"
	@g++ $(OBJS) $(CXXFLAGS) $(CPPFLAGS) -o gbe

clean: 
	rm -f *.o *.d

%.o: %.cpp
	@echo "Compiling "$@""
	@g++ $(CXXFLAGS) $(CPPFLAGS) -c $<

-include $(OBJS:.o=.d)