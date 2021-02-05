#include <stdio.h>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

int main() {
	// Init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(640, 460, "Hello Triangle", NULL, NULL);
	glfwMakeContextCurrent(window);
	printf("Renderer: %s\nOpenGL version %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

	// Init glew
	glewExperimental = GL_TRUE;
	glewInit();

	// On fragments overlap, just draw nearest one
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Rendering
	float points[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	const char* vertex_shader =
		"#version 400\n"
		"in vec3 vp;"
		"uniform float a;"
		"void main () {"
		"  gl_Position = vec4 (vp.x, vp.y - sin(a), vp.z, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 400\n"
		"out vec4 frag_colour;"
		"void main () {"
		"  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
		"}";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);


	GLint a = glGetUniformLocation(shader_programme, "a");
	GLfloat b = 0;
	//glUniform1f(a, b);


	while (!glfwWindowShouldClose(window)) {
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_programme);
		glBindVertexArray(vao);

		// draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
		// update other events like input handling 
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_S)) {
			b += .01;
			glUniform1f(a, b);
		}

		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	// Terminate
	glfwTerminate();
	return 0;
}