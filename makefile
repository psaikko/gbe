GLFWDIR = glfw
GLFWLIBS = -lGL -lGLU -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXcursor -lXinerama
CCOPTS = -std=c++11 -O3 -pedantic -Wall -Wextra

all:
	g++ gbe.cpp -I$(GLFWDIR)/include -L$(GLFWDIR)/lib $(CCOPTS) $(GLFWLIBS) -o gbe
