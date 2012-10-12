/*
#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Utility.hpp"
using namespace std;

static const double MY_PI = 3.14159265358979323846264338327;


class PyramidNode : public SceneNode {
	vector<Vtx> vertices;
	GLfloat height;
public:
	PyramidNode(GLfloat radius, GLfloat height, GLuint sides, GLuint tipColor, GLuint color1, GLuint color2) : vertices(2 + max(sides,3u)), height(height) {
		sides = max(sides,3u);
		vertices.front().x = vertices.front().y = 0;
		vertices.front().z = height;
		vertices.front().color = tipColor;
		
		for ( size_t i = 0; i < sides; ++i ) {
			const double angle = 2.0 * i * MY_PI/(double)sides;
			vertices[i + 1].x = radius * cos(angle);
			vertices[i + 1].y = radius * sin(angle);
			vertices[i + 1].z = 0;
			if ( (i + 1) % 2 == 0 )
				vertices[i + 1].color = color1;
			else
				vertices[i + 1].color = color2;
		}
		vertices.back().x = radius;
		vertices.back().y = 0;
		vertices.back().z = 0;
		vertices.back().color = ((vertices.size() % 2 == 0) ? color1 : color2);
	}
	
	virtual void draw(const GLMatrix4 &parentTransform) {
		const GLMatrix4 &t = parentTransform * transform;
		glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), &vertices[0].x);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &vertices[0].color);
		glUniformMatrix4fv(UNIFORM_transfromationMatrix, 1, false, t.mat);
		
		vertices[0].z = height;
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
		vertices[0].z = 0;
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());		
		
		drawChildren(t);
	}
};


class CoordinateFrameNode : public SceneNode {
	vector<Vtx> vertices;
public:
	CoordinateFrameNode(GLuint xColor, GLuint yColor) : vertices(9 * 2) {
		const GLfloat lineWidth = 0.03f;
		//Y-axis
		//the arrowhead
		vertices[0].x = 0;
		vertices[0].y = 1.0f;
		vertices[0].color = yColor;
		vertices[1].x = -0.1f;
		vertices[1].y = 0.9f;
		vertices[1].color = yColor;
		vertices[2].x = 0.1f;
		vertices[2].y = 0.9f;
		vertices[2].color = yColor;
		
		//the line itself (which is a Rect)
		vertices[3].x = lineWidth;
		vertices[3].y = 0.9f;
		vertices[3].color = yColor;
		vertices[4].x = -lineWidth;
		vertices[4].y = 0.9f;
		vertices[4].color = yColor;
		vertices[5].x = -lineWidth;
		vertices[5].y = 0;
		vertices[5].color = yColor;
		vertices[6].x = lineWidth;
		vertices[6].y = 0.9f;
		vertices[6].color = yColor;
		vertices[7].x = -lineWidth;
		vertices[7].y = 0;
		vertices[7].color = yColor;
		vertices[8].x = lineWidth;
		vertices[8].y = 0;
		vertices[8].color = yColor;
		
		//X-axis
		//the arrowhead
		vertices[9].y = 0;
		vertices[9].x = 1.0f;
		vertices[9].color = xColor;
		vertices[10].y = 0.1f;
		vertices[10].x = 0.9f;
		vertices[10].color = xColor;
		vertices[11].y = -0.1f;
		vertices[11].x = 0.9f;
		vertices[11].color = xColor;
		
		//the line itself (which is a Rect)
		vertices[12].y = -lineWidth;
		vertices[12].x = 0.9f;
		vertices[12].color = xColor;
		vertices[13].y = lineWidth;
		vertices[13].x = 0.9f;
		vertices[13].color = xColor;
		vertices[14].y = lineWidth;
		vertices[14].x = 0;
		vertices[14].color = xColor;
		vertices[15].y = -lineWidth;
		vertices[15].x = 0.9f;
		vertices[15].color = xColor;
		vertices[16].y = lineWidth;
		vertices[16].x = 0;
		vertices[16].color = xColor;
		vertices[17].y = -lineWidth;
		vertices[17].x = 0;
		vertices[17].color = xColor;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform) {
		const GLMatrix4 &t = parentTransform * transform;
		glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), &vertices[0].x);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &vertices[0].color);
		glUniformMatrix4fv(UNIFORM_transfromationMatrix, 1, false, t.mat);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		
		drawChildren(t);
	}
		
		
};

/********************
 *
 * The usual main loop.
 *
 ********************/
/*
int main() {
	if ( !glfwInit() ) {
		cerr << "Unable to initialize OpenGL!\n";
		return -1;
	}
	if ( !glfwOpenWindow(640,640,
				8,8,8,8,
				16,0,
				GLFW_WINDOW) ) {
		cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		return -1;
	}
	
	if ( glewInit() != GLEW_OK ) {
		cerr << "Unable to hook OpenGL extensions!\n";
		return -1;
	}
	glfwSetWindowTitle("3D Transformations");
	
	glfwEnable(GLFW_STICKY_KEYS);
	glfwSwapInterval(1);

	GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER),
	       fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	if ( !loadShaderSource(vtxShader, "3d.vsh") ) return -1;
	if ( !loadShaderSource(fragShader, "3d.fsh") ) return -1;

	GLuint program = glCreateProgram();
	glAttachShader(program, vtxShader);
	glAttachShader(program, fragShader);

	glBindAttribLocation(program, ATTRIB_POS, "pos");
	glBindAttribLocation(program, ATTRIB_COLOR, "color");

	glLinkProgram(program);
	

	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if ( logLength > 0 ) {
			GLchar *log = new GLchar[logLength];
			glGetProgramInfoLog(program, logLength, &logLength, log);
			cout << "Program Compile Log:\n" << log << endl;
			delete [] log;
		}
	}
	glDeleteShader(fragShader);
	glDeleteShader(vtxShader);
	fragShader = 0;
	vtxShader = 0;

	UNIFORM_transfromationMatrix = glGetUniformLocation(program, "modelTransform");
	glUseProgram(program);

	glEnable(GL_DEPTH_TEST);
	
	SceneNode root;
	PyramidNode pyramid(0.3, 0.4, 6, 0xFFFFFF00, 0xFFFF0000, 0xFFFFFFFF);
	root.children.push_back(&pyramid);
	
	glEnableVertexAttribArray(ATTRIB_POS);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	double t = 0;
	
	GLfloat camX = 0, camY = 0, camRot = 0, camS = 1;
	do {
		//update the camera
		
		bool alt = (glfwGetKey(GLFW_KEY_LSHIFT) == GLFW_PRESS)|| (glfwGetKey(GLFW_KEY_RSHIFT) == GLFW_PRESS);
		//The order for the camera is scale->rotate->translate
		//so the order for the view is translate^-1 -> rotate^-1 -> scale^-1
		if ( glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS ) {
			if ( alt )
				camS += 0.005;
			else
				camY += 0.01;
		} else if ( glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS ) {
			if ( alt ) {
				if ( camS > 0.0051 )
					camS -= 0.005;
			} else
				camY -= 0.01;
		} else if ( glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS ) {
			//if ( alt )
				camRot += 0.01;
			//else
			//	camX -= 0.01;
		} else if ( glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS ) {
			if ( alt )
				camRot -= 0.1;
			//else
			//	camX += 0.01;
		}
		

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLMatrix4 baseTransform;
		baseTransform.setRotation(1, 1, 0, t);
		root.draw(baseTransform);
		
		glfwSwapBuffers();
		t += 0.02f;
	} while ( glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED) );
	
	
	glfwTerminate();

	return 0;
}
*/