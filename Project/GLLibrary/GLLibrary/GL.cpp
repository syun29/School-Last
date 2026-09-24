#include "GL.h"

//GPUŽg—p
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
GLFWwindow* GL::window;
RECT GL::window_rect;
int GL::window_x;
int GL::window_y;
int GL::window_width;
int GL::window_height;
int GL::focus = GLFW_TRUE;
HWND GL::hWnd;
bool GL::full_screen = false;
bool GL::isQuit = false;
void GL::UpdateWindowRect(int x, int y, int w, int h) {
	window_x = x; window_y = y;
	window_width = w; window_height = h;
	window_rect.top = window_y + 32;
	window_rect.left = window_x + 16;
	window_rect.right = window_x + window_width - 16;
	window_rect.bottom = window_x + window_height - 24;
}
void GL::PrintDeviceInformation() {
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint va;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &va);
	GLint vv;
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &vv);
	GLint vuv;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &vuv);
	GLint fuv;
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &fuv);
	GLint vtiu;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vtiu);
	GLint tiu;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tiu);
	GLint ts;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &ts);
	GLint rs;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &rs);
	GLint vd[2];
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, vd);
	printf("GL_MAX_VERTEX_ATTRIBS %d\n", va);
	printf("GL_MAX_VARYING_VECTORS %d\n", va);
	printf("GL_MAX_VERTEX_UNIFORM_VECTORS %d\n", vuv);
	printf("GL_MAX_FRAGMENT_UNIFORM_VECTORS %d\n", fuv);
	printf("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS %d\n", vtiu);
	printf("GL_MAX_TEXTURE_IMAGE_UNITS %d\n", tiu);
	printf("GL_MAX_TEXTURE_SIZE %d\n", ts);
	printf("GL_MAX_RENDERBUFFER_SIZE %d\n", rs);
	printf("GL_MAX_VIEWPORT_DIMS %d %d\n", vd[0], vd[1]);
}

void GL::ChangeFullScreen(bool full,int monitor_no)
{
	if (full == GL::full_screen) return;
	//GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	if (monitor_no >= count) monitor_no = 0;
	
	GLFWmonitor* monitor = monitors[monitor_no];
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (!GL::full_screen) {
		glfwGetWindowPos(GL::window, &window_x, &window_y);
		glfwSetWindowMonitor(GL::window, monitor, 0, 0, GL::window_width, GL::window_height, mode->refreshRate);
		GL::full_screen = true;
	}
	else {
		glfwSetWindowMonitor(GL::window, nullptr, window_x, window_y, GL::window_width, GL::window_height, mode->refreshRate);
		GL::full_screen = false;
	}
}
