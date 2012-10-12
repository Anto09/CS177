#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <cassert>
#include <vector>

using namespace std;

const GLfloat PI = 3.14159265358979323846264338327f;
enum { ATTRIB_POS, ATTRIB_COLOR };
GLuint UNIFORM_transfromationMatrix;

struct Vtx {
	GLfloat x, y, z;
	GLuint color;
};

struct GLMatrix4 {
	GLfloat mat[16];

	void create_rotation_matrix_4x4(GLfloat x, GLfloat y, GLfloat z, GLfloat theta, GLfloat mat[]){

	}
	
	void setIdentity() {
		mat[0] = 1, mat[4] = 0, mat[8] = 0, mat[12] = 0;
		mat[1] = 0, mat[5] = 1, mat[9] = 0, mat[13] = 0;
		mat[2] = 0, mat[6] = 0, mat[10] = 1, mat[14] = 0;
		mat[3] = 0, mat[7] = 0, mat[11] = 0, mat[15] = 1;
	}
	
	void setRotation(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) {
		create_rotation_matrix_4x4(x, y, z, theta, mat);
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
		
		for(int i = 0; i < 4; ++i){
			ret.mat[0+i] = mat[0+i]*rhs.mat[0]+mat[4+i]*rhs.mat[1]+mat[8+i]*rhs.mat[2]+mat[12+i]*rhs.mat[3];
		}
		for(int i = 0; i < 4; ++i){
			ret.mat[4+i] = mat[0+i]*rhs.mat[4]+mat[4+i]*rhs.mat[5]+mat[8+i]*rhs.mat[6]+mat[12+i]*rhs.mat[7];
		}
		for(int i = 0; i < 4; ++i){
			ret.mat[8+i] = mat[0+i]*rhs.mat[8]+mat[4+i]*rhs.mat[9]+mat[8+i]*rhs.mat[10]+mat[12+i]*rhs.mat[11];
		}
		for(int i = 0; i < 4; ++i){
			ret.mat[12+i] = mat[0+i]*rhs.mat[12]+mat[4+i]*rhs.mat[13]+mat[8+i]*rhs.mat[14]+mat[12+i]*rhs.mat[15];
		}
		
		return ret;
	}
	
	GLMatrix4& operator*=(const GLMatrix4 &rhs) {
		GLfloat tmp[16];
		for(int i = 0; i < 4; ++i){
			tmp[0+i] = mat[0+i]*rhs.mat[0]+mat[4+i]*rhs.mat[1]+mat[8+i]*rhs.mat[2]+mat[12+i]*rhs.mat[3];
		}
		for(int i = 0; i < 4; ++i){
			tmp[4+i] = mat[0+i]*rhs.mat[4]+mat[4+i]*rhs.mat[5]+mat[8+i]*rhs.mat[6]+mat[12+i]*rhs.mat[7];
		}
		for(int i = 0; i < 4; ++i){
			tmp[8+i] = mat[0+i]*rhs.mat[8]+mat[4+i]*rhs.mat[9]+mat[8+i]*rhs.mat[10]+mat[12+i]*rhs.mat[11];
		}
		for(int i = 0; i < 4; ++i){
			tmp[12+i] = mat[0+i]*rhs.mat[12]+mat[4+i]*rhs.mat[13]+mat[8+i]*rhs.mat[14]+mat[12+i]*rhs.mat[15];
		}
		
		memcpy(mat, tmp, sizeof(mat));
		return *this;
	}
};

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

class RectNode : public SceneNode {
	Vtx vtx[4];
	GLfloat lineWidth;
public:
	RectNode(void){
	
	}
	RectNode(GLfloat width, GLfloat height, GLuint color, GLfloat lineWidth) : lineWidth(lineWidth) {
		vtx[0].x = -width/2;
		vtx[0].y = height/2;
		vtx[0].z = 0;
		vtx[0].color = color;

		vtx[1].x = -width/2;
		vtx[1].y = -height/2;
		vtx[1].z = 0;
		vtx[1].color = color;
		
		vtx[2].x = width/2;
		vtx[2].y = -height/2;
		vtx[2].z = 0;
		vtx[2].color = color;
		
		vtx[3].x = width/2;
		vtx[3].y = height/2;
		vtx[3].z = 0;
		vtx[3].color = color;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform) {

		const GLMatrix4 &t = parentTransform * transform;
		glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), &vtx[0].x);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &vtx[0].color);
		glUniformMatrix4fv(UNIFORM_transfromationMatrix, 1, false, t.mat);
		
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(lineWidth);
		//glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		drawChildren(t);
	}
};

class HandNode : public SceneNode {
	GLfloat lineWidth;
	RectNode base1,base2,side1,side2,side3,side4;
public:
	HandNode(GLfloat width, GLfloat height, GLfloat length, GLuint color, GLfloat lineWidth) : lineWidth(lineWidth) {
		base1 = RectNode(width,length,color,lineWidth);
		base2 = RectNode(width,length,color,lineWidth);
		base2.transform.translate(-1,0,0);
		base2.transform.scale(2,0,0);
		side1 = RectNode(width,height,color,lineWidth);
		side1.transform.translate(.5,0,0);
		side2 = RectNode(width,height,color,lineWidth);
		side2.transform.translate(-.5,0,0);
		side3 = RectNode(width,height,color,lineWidth);
		side3.transform.translate(0,.5,0);
		side4 = RectNode(width,height,color,lineWidth);
		side4.transform.translate(0,-.5,0);
		children.push_back(&base1);
		children.push_back(&base2);
		children.push_back(&side1);
		children.push_back(&side2);
		children.push_back(&side3);
		children.push_back(&side4);
	}
	
	virtual void draw(const GLMatrix4 &parentTransform) {
		const GLMatrix4 &t = parentTransform * transform;
		drawChildren(t);
	}
};

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
