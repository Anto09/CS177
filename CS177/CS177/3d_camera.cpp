
#include <GL/glew.h>
#include <GL/glfw.h>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

static const double MY_PI = 3.14159265358979323846264338327;

struct Vtx {
	GLfloat x, y, z;
	GLuint color;
};

enum { ATTRIB_POS, ATTRIB_COLOR };

GLuint UNIFORM_transfromationMatrix;


/********************
 *
 * 3x3 OpenGL Matrix class
 *
 ********************/
struct GLMatrix4 {
	GLfloat mat[16];

	void create_rotation_matrix_4x4Y(GLfloat x, GLfloat y, GLfloat z, GLfloat theta, GLfloat *mat){
		const GLfloat c = cos(theta), s = sin(theta);
		mat[0] = c, mat[4] = 0, mat[8] = s,	mat[12] = (x*(1-c))-(z*s);
		mat[1] = 0, mat[5] = 1, mat[9] = 0,	mat[13] = 0;
		mat[2] = -1*s, mat[6] = 0, mat[10] = c, mat[14] = (x*s )+ (z*(1-c));
		mat[3] = 0, mat[7] = 0, mat[11] =0, mat[15] = 1;
	}

	void create_rotation_matrix_4x4X(GLfloat x, GLfloat y, GLfloat z, GLfloat theta, GLfloat *mat) {
		const GLfloat c = cos(theta), s = sin(theta);
		mat[0] = 1, mat[4] = 0, mat[8] = 0,	mat[12] = 0;
		mat[1] = 0, mat[5] = c, mat[9] = -1*s,	mat[13] = (s*z)+(y*(1-c));
		mat[2] = 0, mat[6] = s, mat[10] = c, mat[14] = (-1*y*s)+(z*(1-c));
		mat[3] = 0, mat[7] = 0, mat[11] =0, mat[16] = 1;
	}

	void create_rotation_matrix_4x4Z(GLfloat x, GLfloat y, GLfloat z, GLfloat theta, GLfloat *mat) {
		const GLfloat c = cos(theta), s = sin(theta);
		mat[0] = c, mat[4] = -s, mat[8] = 0,	mat[12] = (x*(1-c))+(y*s);
		mat[1] = s, mat[5] = c, mat[9] = 0,	mat[13] = (y*(1-c))-(-1*s*x);
		mat[2] = 0, mat[6] = 0, mat[10] = 1, mat[14] = 0;
		mat[3] = 0, mat[7] = 0, mat[11] =0, mat[15] = 1;
	}

	void setIdentity() {
		mat[0] = 1, mat[4] = 0, mat[8] = 0, mat[12] = 0;
		mat[1] = 0, mat[5] = 1, mat[9] = 0, mat[13] = 0;
		mat[2] = 0, mat[6] = 0, mat[10] = 1, mat[14] = 0;
		mat[3] = 0, mat[7] = 0, mat[11] = 0, mat[15] = 1;
	}
	
	void setRotationX(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) {
		create_rotation_matrix_4x4X(x, y, z, theta, mat);
	}
	
	void setRotationY(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) {
		create_rotation_matrix_4x4Y(x, y, z, theta, mat);
	}

	void setRotationZ(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) {
		create_rotation_matrix_4x4Z(x, y, z, theta, mat);
	}

	void setTranslation(GLfloat x, GLfloat y, GLfloat z) {
		mat[0] = 1, mat[4] = 0, mat[8] = 0, mat[12] = x;
		mat[1] = 0, mat[5] = 1, mat[9] = 0, mat[13] = y;
		mat[2] = 0, mat[6] = 0, mat[10] = 1, mat[14] = z;
		mat[3] = 0, mat[7] = 0, mat[11] = 0, mat[15] = 1;
	}

	void translate(GLfloat x, GLfloat y, GLfloat z) {
		mat[12] += x;
		mat[13] += y;
		mat[14] += z;
	}
	
	void scale(GLfloat sx, GLfloat sy, GLfloat sz) {
		mat[0] *= sx;
		mat[4] *= sx;
		mat[8] *= sx;
		mat[12] *= sx;
		
		mat[1] *= sy;
		mat[5] *= sy;
		mat[9] *= sy;
		mat[13] *= sy;
		
		mat[2] *= sz;
		mat[6] *= sz;
		mat[10] *= sz;
		mat[14] *= sz;
	}
	
	void transpose() {
		swap(mat[4],mat[1]);
		swap(mat[8],mat[2]);
		swap(mat[12],mat[3]);
		swap(mat[9],mat[6]);
		swap(mat[13],mat[7]);
		swap(mat[14], mat[11]);
	}
	
	GLMatrix4& operator=(const GLMatrix4 &rhs) {
		memcpy(mat, rhs.mat, sizeof(mat));
		return *this;
	}
	
	GLMatrix4 operator*(const GLMatrix4 &rhs) const {
		GLMatrix4 ret;
		for ( int i = 0; i < 16; ++i ) {
			const int a = i % 4, b = (i / 4) * 4;
			ret.mat[i] = mat[a]*rhs.mat[b] + mat[a+4]*rhs.mat[b+1] + mat[a+8]*rhs.mat[b+2] + mat[a+12]*rhs.mat[b+3];
		}
		return ret;
	}
	
	GLMatrix4& operator*=(const GLMatrix4 &rhs) {
		GLMatrix4 tmp;
		for ( int i = 0; i < 9; ++i ) {
			const int a = i % 3, b = (i / 3) * 3;
			tmp.mat[i] = mat[a]*rhs.mat[b] + mat[a+4]*rhs.mat[b+1] + mat[a+8]*rhs.mat[b+2] + mat[a+12]*rhs.mat[b+3];
		}
		memcpy(mat, tmp.mat, sizeof(mat));
		return *this;
	}
};

/********************
 *
 * Scene Node class used to implement a transformation hierarchy.
 *
 ********************/
class SceneNode {
public:
	GLMatrix4 transform;
	vector<SceneNode*> children;
	SceneNode() {
		transform.setIdentity();
	}
	virtual void draw(const GLMatrix4 &parentTransform) {
		drawChildren(parentTransform * transform);
	}
	
	virtual void update(double t) {
		for ( size_t i = 0; i < children.size(); ++i )
			children[i]->update(t);
	}
	
	void drawChildren(const GLMatrix4 &t) {
		for ( size_t i = 0; i < children.size(); ++i )
			children[i]->draw(t);
	}
	
	virtual ~SceneNode() {
	}
};


class RegularPolygonNode : public SceneNode {
	vector<Vtx> vertices;
public:
	RegularPolygonNode(GLfloat radius, GLuint sides, GLuint color) : vertices(2 + max(sides,3u)) {
		sides = max(sides,3u);
		vertices.front().x = vertices.front().y = 0;
		vertices.front().color = color;
		
		for ( size_t i = 0; i < sides; ++i ) {
			const double angle = 2.0 * i * MY_PI/(double)sides;
			vertices[i + 1].x = radius * cos(angle);
			vertices[i + 1].y = radius * sin(angle);
			vertices[i + 1].color = color;
		}
		vertices.back().x = radius;
		vertices.back().y = 0;
		vertices.back().color = color;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform) {
		const GLMatrix4 &t = parentTransform * transform;
		glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), &vertices[0].x);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &vertices[0].color);
		glUniformMatrix4fv(UNIFORM_transfromationMatrix, 1, false, t.mat);
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

class RectNode : public SceneNode {
	Vtx vtx[4];
	GLfloat lineWidth;
public:
	RectNode(GLfloat width, GLfloat height, GLuint color, GLfloat lineWidth) : lineWidth(lineWidth) {
		vtx[0].x = -width/2;
		vtx[0].y = height/2;
		vtx[0].color = color;

		vtx[1].x = -width/2;
		vtx[1].y = -height/2;
		vtx[1].color = color;
		
		vtx[2].x = width/2;
		vtx[2].y = -height/2;
		vtx[2].color = color;
		
		vtx[3].x = width/2;
		vtx[3].y = height/2;
		vtx[3].color = color;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform) {
		const GLMatrix4 &t = parentTransform * transform;
		glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), &vtx[0].x);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &vtx[0].color);
		glUniformMatrix4fv(UNIFORM_transfromationMatrix, 1, false, t.mat);
		
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(lineWidth);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		
		drawChildren(t);
	}
};


/****************************************
 *
 * The real meat of the program.
 *
 * The nodeList array is there in order to keep track of nodes to delete when we clean up.
 * The cleaning up is not really necessary since we're gonna exit anyway.
 ****************************************/
void createScene(SceneNode &root, vector<SceneNode*> &nodeList) {
	static const GLuint xColor = 0xFF0000FF, yColor = 0xFFFF0000;
	//a simple estimation
	nodeList.resize(32, 0);
	
	root.children.push_back(nodeList[0] = new CoordinateFrameNode(0xFF00FFFF, 0xFFFFFF00));
	
	CoordinateFrameNode *coordinateFrame = new CoordinateFrameNode(xColor, yColor);
	nodeList[1] = coordinateFrame;
	
	coordinateFrame->transform.scale(0.5,0.5,0);
	
	root.children.push_back(nodeList[2] = new RegularPolygonNode(.3, 4, 0xFF00AAAA));
	nodeList[2]->transform.setRotationY(0.5, 0.5, 0,MY_PI/6);
	nodeList[2]->transform.translate(0, 0.5,0);
	nodeList[2]->children.push_back(coordinateFrame);
	
	nodeList[2]->children.push_back(nodeList[3] = new RegularPolygonNode(.05, 16, 0xFFFFFFFF));
	nodeList[3]->transform.translate(.2,.2,0);
	nodeList[3]->children.push_back(coordinateFrame);
	
	nodeList[2]->children.push_back(nodeList[4] = new RegularPolygonNode(.05, 16, 0xFFAAFAFA));
	nodeList[4]->transform.translate(-.2,.2,0);
	nodeList[4]->children.push_back(coordinateFrame);
	
	root.children.push_back(nodeList[5] = new RegularPolygonNode(.2, 5, 0xFFAAFF00));
	nodeList[5]->transform.translate(-.4, .1,0);
	nodeList[5]->children.push_back(coordinateFrame);
	
}
/********************
 *
 * The usual main loop.
 *
 ********************/

bool loadShaderSource(GLuint shader, const char *filePath) {
	FILE *f = fopen(filePath, "r");
	if ( !f ) {
		cerr << "Cannot find file: " << filePath << '\n';
		return false;
	}
	fseek(f, 0, SEEK_END);
	const size_t sz = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);
	
	GLchar *buffer = new GLchar[sz];
	fread(buffer, 1, sz, f);
	fclose(f);
	buffer[sz-1] = 0;
	glShaderSource(shader, 1, (const GLchar**) &buffer, NULL);
	
	glCompileShader(shader);
	delete [] buffer;
	
	GLint logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if ( logLength > 0 ) {
		GLchar *log = new GLchar[logLength];
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		cout << "Shader Compile Log:\n" << log << endl;
		delete [] log;
	}
	
	return true;
}


int main() {
	if ( !glfwInit() ) {
		cerr << "Unable to initialize OpenGL!\n";
		return -1;
	}
	if ( !glfwOpenWindow(640,640,
				8,8,8,8,
				0,0,
				GLFW_WINDOW) ) {
		cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		return -1;
	}
	
	if ( glewInit() != GLEW_OK ) {
		cerr << "Unable to hook OpenGL extensions!\n";
		return -1;
	}
	glfwSetWindowTitle("2D Transformations");
	
	glfwEnable(GLFW_STICKY_KEYS);
	glfwSwapInterval(1);

	GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER),
	       fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	if ( !loadShaderSource(vtxShader, "2d.vsh") ) return -1;
	if ( !loadShaderSource(fragShader, "2d.fsh") ) return -1;

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

	
	SceneNode root;
	RectNode cameraNode(2, 2, 0xFF00FF00, 4);
	
	RegularPolygonNode bg(sqrt(2.0f), 4, 0xFF000000);
	bg.transform.setRotationY(0,0,0,MY_PI/4);
	
	vector<SceneNode*> nodeList;
	root.transform.setIdentity();
	createScene(root, nodeList);
	root.children.push_back(&cameraNode);

	glEnableVertexAttribArray(ATTRIB_POS);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	double t = 0;
	
	GLfloat camX = 0, camY = 0, camZ = 0, camRot = 0, camS = 1;
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
			if ( alt )
				camRot += 0.01;
			else
				camX -= 0.01;
		} else if ( glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS ) {
			if ( alt )
				camRot -= 0.01;
			else
				camX += 0.01;
		}
		
		cameraNode.transform.setIdentity();
		cameraNode.transform.scale(camS, camS,0);
		GLMatrix4 rotationMatrix;
		rotationMatrix.setRotationY(0, 0, 0, camRot);
		cameraNode.transform = rotationMatrix * cameraNode.transform;
		cameraNode.transform.translate(camX, camY,0);
		
		glClear(GL_COLOR_BUFFER_BIT);

		int windowWidth, windowHeight;
		glfwGetWindowSize(&windowWidth, &windowHeight);
		
	
		GLMatrix4 baseTransform;
		baseTransform.setIdentity();
		baseTransform.translate(-camX, -camY,0);
		rotationMatrix.transpose();
		baseTransform = rotationMatrix * baseTransform;
		baseTransform.scale(1.0/camS, 1.0/camS,0);
		
		
		if ( glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS ) {
			glViewport(0,0,windowWidth, windowHeight);
			GLMatrix4 ident;
			ident.setIdentity();
			root.draw(ident);
			
			glViewport(0,0,windowWidth/4, windowHeight/4);
			bg.draw(ident);
			root.draw(baseTransform);
		} else {
			glViewport(0,0,windowWidth, windowHeight);
			root.draw(baseTransform);
			
			glViewport(0,0,windowWidth/4, windowHeight/4);
			GLMatrix4 ident;
			ident.setIdentity();
			bg.draw(ident);
			root.draw(ident);
		}
		
		glfwSwapBuffers();
		t += 0.02f;
	} while ( glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED) );
	
	for ( size_t i = 0; i < nodeList.size(); ++i )
		delete nodeList[i];
	
	glfwTerminate();

	return 0;
}