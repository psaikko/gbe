GLFWDIR = glfw
GLFWLIBS = -lGL -lGLU -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXcursor -lXinerama

NO_BOTHERSOME_WARNINGS = -Wno-format-security -Wno-unused-result

CXX = g++
CXXFLAGS = -std=c++11 -ggdb -Wall -Wextra $(NO_BOTHERSOME_WARNINGS)
CPPFLAGS = -I$(GLFWDIR)/include -L$(GLFWDIR)/lib $(GLFWLIBS)

SRCS=$(find . | grep ".cpp$")
SRCS=gpu.cpp window.cpp mem.cpp timer.cpp cpu.cpp serial.cpp gbe.cpp
OBJS=$(subst .cpp,.o,$(SRCS))


all: clean $(OBJS)
	@echo "Linking gbe"
	g++ $(OBJS) $(CXXFLAGS) $(CPPFLAGS) -o gbe

clean: 
	rm -f *.o

%.o: %.cpp
	@echo "Compiling "$@":"
	g++ $(CXXFLAGS) $(CPPFLAGS) -c $<
