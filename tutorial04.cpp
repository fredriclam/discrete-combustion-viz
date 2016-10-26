// OpenGL 3-D Temperature field visualization based on voxels
// Using public license code from:
// http://www.opengl-tutorial.org/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Include I/O
#include <iostream>
#include <fstream>
#include <string>

// Include vector
#include <vector>

#include <sstream>

#include <common/shader.hpp>

#include <FreeImage.h>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

const int x_res = 61;
const int y_res = 91;
const int z_res = 91;
const float stretch = 0.2;
const float width_x = stretch * 45;// 0;
const float width_y = stretch * 15;// 0;
const float width_z = stretch * 15;// 0;
const float dx = width_x / (x_res - 1);
const float dy = width_y / (y_res - 1);
const float dz = width_z / (z_res - 1);

std::string file_name = "TFLDn6s0.dat";
int frame_max = 200;

const float cmax = 1.2;
const float luminance_factor = 0.1 * 500;

float campos[] = { 12, 2, -5 };
float oripos[] = { 6, 0, 0 };

struct Color{
	GLfloat R;
	GLfloat G;
	GLfloat B;
};

// Color mapping
Color cmap(float t){
	Color c;

	// Select colour from gamut based on parameter t
	c.R = max(min(8./3. * t, 1.), 0.);
	c.G = max(min(8./3. * (t - 3./8.), 1.), 0.);
	c.B = max(min(4. * (t - 3./4.), 1.), 0.);

	//c.R = max(min(1. / 24. * t, 1.),0.);
	//c.G = max(min(1. / 24. * (t - 24.), 1.), 0.);
	//c.B = max(min(1. / 16. * (t - 48.), 1.), 0.);

	// Luminance * density factor uniformly
	float f = dx*dy*dz * luminance_factor;
	c.R *= f;
	c.G *= f;
	c.B *= f;

	return c;
}

// Emittance function
float kernel(float temperature){
	//float q = 0; //= temperature / cmax;
	//if (temperature >= 0.2)
	return temperature*temperature / (cmax*cmax);
	//return sqrt(temperature) / sqrt(cmax);
}


void add_voxel(float x, float y, float z, float temperature, std::vector<GLfloat>& g_vertex_buffer_data, std::vector<GLfloat>& g_color_buffer_data){
	GLfloat temporary_array[108] = {
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
	for (int i = 0; i < 36; ++i){
		temporary_array[3 * i] = temporary_array[3 * i]*0.5*dx + x;
		temporary_array[3 * i + 1] = temporary_array[3 * i + 1]*0.5*dy + y;
		temporary_array[3 * i + 2] = temporary_array[3 * i + 2]*0.5*dz + z;
	}
	// Insert into vertex buffer
	g_vertex_buffer_data.insert(g_vertex_buffer_data.end(), temporary_array, temporary_array + sizeof(temporary_array) / sizeof(temporary_array[0]));

	// Interpret colour
	//float light_level = kernel(temperature);
	//int i_RGB = min(max(int(64 * light_level), 0),63);
	// Color (light level) as function of temperature
	Color c = cmap(kernel(temperature));
	// Fill colour on all sides of voxel
	for (int i = 0; i < 36; ++i){
		temporary_array[3 * i] = c.R;
		temporary_array[3 * i + 1] = c.G;
		temporary_array[3 * i + 2] = c.B;
	}
	// Insert into color buffer
	g_color_buffer_data.insert(g_color_buffer_data.end(), temporary_array, temporary_array + sizeof(temporary_array) / sizeof(temporary_array[0]));
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// ! Enalbe alpha blending
	glEnable(GL_BLEND);
	// Custom blending function
	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	//glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	//glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(campos[0], campos[1], campos[2]),
		//glm::vec3(6.35, 6.1, 5.25), // (6, 8, 8), //(8, 6, -6), // Camera is at (4,3,-3), in World Space
		glm::vec3(oripos[0], oripos[1], oripos[2]), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// Temperature field streaming
	std::ifstream istream(file_name);


	for (int frame_num = 1; frame_num <= frame_max; ++frame_num){
		// Voxel rendering
		int N = 0;
		std::vector<GLfloat> v_buffer, c_buffer;
		float temperature;

		// Skip
		//for (int i = 0; i < (frame_num - 1)*x_res*y_res*z_res; ++i){
		//	istream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		//}

		for (int k = 0; k < z_res; ++k)
			for (int j = 0; j < y_res; ++j)
				for (int i = 0; i < x_res; ++i){
					istream >> temperature;
					//std::cout << temperature;

					// Render nothing(-1 = > outside cylinder)
					if (temperature > 0){

						// Cylindrical problem
						float x = width_x * i / (x_res - 1);
						float y = -0.5*width_y + width_y * j / (y_res - 1);
						float z = -0.5*width_z + width_z * k / (z_res - 1);

						// Add voxel
						add_voxel(x, y, z, temperature, v_buffer, c_buffer);
						// Increment voxel count for rendering later
						++N;
					}

				}

		// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
		// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices

		///*static const */GLfloat g_vertex_buffer_data[] = {
		//	-1.0f,-1.0f,-1.0f,
		//	-1.0f,-1.0f, 1.0f,
		//	-1.0f, 1.0f, 1.0f,
		//	 1.0f, 1.0f,-1.0f,
		//	-1.0f,-1.0f,-1.0f,
		//	-1.0f, 1.0f,-1.0f,
		//	 1.0f,-1.0f, 1.0f,
		//	-1.0f,-1.0f,-1.0f,
		//	 1.0f,-1.0f,-1.0f,
		//	 1.0f, 1.0f,-1.0f,
		//	 1.0f,-1.0f,-1.0f,
		//	-1.0f,-1.0f,-1.0f,
		//	-1.0f,-1.0f,-1.0f,
		//	-1.0f, 1.0f, 1.0f,
		//	-1.0f, 1.0f,-1.0f,
		//	 1.0f,-1.0f, 1.0f,
		//	-1.0f,-1.0f, 1.0f,
		//	-1.0f,-1.0f,-1.0f,
		//	-1.0f, 1.0f, 1.0f,
		//	-1.0f,-1.0f, 1.0f,
		//	 1.0f,-1.0f, 1.0f,
		//	 1.0f, 1.0f, 1.0f,
		//	 1.0f,-1.0f,-1.0f,
		//	 1.0f, 1.0f,-1.0f,
		//	 1.0f,-1.0f,-1.0f,
		//	 1.0f, 1.0f, 1.0f,
		//	 1.0f,-1.0f, 1.0f,
		//	 1.0f, 1.0f, 1.0f,
		//	 1.0f, 1.0f,-1.0f,
		//	-1.0f, 1.0f,-1.0f,
		//	 1.0f, 1.0f, 1.0f,
		//	-1.0f, 1.0f,-1.0f,
		//	-1.0f, 1.0f, 1.0f,
		//	 1.0f, 1.0f, 1.0f,
		//	-1.0f, 1.0f, 1.0f,
		//	 1.0f,-1.0f, 1.0f,
		//
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -1.0f, 1.0f, -1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -1.0f, -1.0f, 1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -1.0f, -1.0f, -1.0f,
		//	 -1.0f, 1.0f, -1.0f,
		//	 -1.0f, -1.0f, -1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -1.0f, -1.0f, 1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -1.0f, -1.0f, 1.0f,
		//	 -1.0f, 1.0f, 1.0f,
		//	 -1.0f, -1.0f, -1.0f,
		//	 -1.0f, 1.0f, -1.0f,
		//	 -1.0f, -1.0f, -1.0f,
		//	 -1.0f, 1.0f, 1.0f,
		//	 -1.0f, -1.0f, 1.0f,
		//	 -1.0f, 1.0f, 1.0f,
		//	 -1.0f, 1.0f, -1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -1.0f, 1.0f, 1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -1.0f, 1.0f, 1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -1.0f, -1.0f, 1.0f,
		//
		//	 -3.0f, -1.0f, -1.0f,
		//	 -3.0f, -1.0f, 1.0f,
		//	 -3.0f, 1.0f, 1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -3.0f, -1.0f, -1.0f,
		//	 -3.0f, 1.0f, -1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -3.0f, -1.0f, -1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -3.0f, -1.0f, -1.0f,
		//	 -3.0f, -1.0f, -1.0f,
		//	 -3.0f, 1.0f, 1.0f,
		//	 -3.0f, 1.0f, -1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -3.0f, -1.0f, 1.0f,
		//	 -3.0f, -1.0f, -1.0f,
		//	 -3.0f, 1.0f, 1.0f,
		//	 -3.0f, -1.0f, 1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -2.0f, -1.0f, -1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -2.0f, 1.0f, -1.0f,
		//	 -3.0f, 1.0f, -1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -3.0f, 1.0f, -1.0f,
		//	 -3.0f, 1.0f, 1.0f,
		//	 -2.0f, 1.0f, 1.0f,
		//	 -3.0f, 1.0f, 1.0f,
		//	 -2.0f, -1.0f, 1.0f,
		//};

		//static const float R = 1, G = 1, B = 1;

		// One color for each vertex. They were generated randomly.
		///*static const */GLfloat g_color_buffer_data[] = {
		//	/*R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,*/
		//
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B,
		//	R, G, B
		//};

		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, v_buffer.size() * sizeof(GLfloat), &v_buffer[0], GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		GLuint colorbuffer;
		glGenBuffers(1, &colorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, c_buffer.size() * sizeof(GLfloat), &c_buffer[0], GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

		do{

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(programID);

			// Send our transformation to the currently bound shader,
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

			// 2nd attribute buffer : colors
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, N * 12 * 3); // 12*3 indices starting at 0 -> 12 triangles //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			// FreeImage Bitmap Output Block
			// Make the BYTE array, factor of 3 because it's RBG.
			BYTE* pixels = new BYTE[3 * SCREEN_WIDTH * SCREEN_HEIGHT];
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, pixels);
			// Convert to FreeImage format & save to file
			// modified FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0x0000FF, 0x00FF00, 0xFF0000, false);
			FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, 3 * SCREEN_WIDTH, 24, 0x0000FF, 0xFF0000, 0x00FF00, false);

			std::stringstream ss;
			ss << "frame" << frame_num << ".bmp";
			FreeImage_Save(FIF_BMP, image, ss.str().c_str(), 0);
			// Free resources
			FreeImage_Unload(image);
			delete[] pixels;

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();

		} // Check if the ESC key was pressed or the window was closed
		/*while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			   glfwWindowShouldClose(window) == 0 );*/
		while (false);

		// Cleanup VBO and shader (copy)
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &colorbuffer);
	}

	//

	int n_n;
	std::cin >> n_n;

	// Cleanup VBO and shader
	//glDeleteBuffers(1, &vertexbuffer);
	//glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

