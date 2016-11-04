GLFWDIR = glfw
GLFWLIBS = -lGL -lGLU -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXcursor -lXinerama
CCOPTS = -std=c++11 -O3 -Wall -Wextra # -pedantic

NO_BOTHERSOME_WARNINGS = -Wno-format-security -Wno-unused-result -Wno-maybe-uninitialized

all:
	g++ gbe.cpp -I$(GLFWDIR)/include -L$(GLFWDIR)/lib $(CCOPTS) $(GLFWLIBS) $(NO_BOTHERSOME_WARNINGS) -o gbe
