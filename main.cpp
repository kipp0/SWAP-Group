#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/glm/gtx/transform.hpp"

#include "ShaderProgram.h"
#include "ObjMesh.h"

#define SCALE_FACTOR 2.0f

int width, height;

GLuint programId;
GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;
GLuint positions_vbo = 0;
GLuint textureCoords_vbo = 0;
GLuint normals_vbo = 0;
GLuint colours_vbo = 0;

unsigned int numVertices;

float angle = 0.0f;
float lightOffsetY = 0.0f;
glm::vec3 eyePosition(40, 30, 30);
bool animateLight = true;
bool rotateObject = true;
float scaleFactor = 2.0f;
float lastX = std::numeric_limits<float>::infinity();
float lastY = std::numeric_limits<float>::infinity();


// skybox stuff


static void createGeometry(void) {
  ObjMesh mesh;
  ObjMesh mesh2;
  mesh.load("logo.obj", true, true);
  mesh2.load("logo.obj", true, true);

  numVertices = mesh.getNumIndexedVertices();
  Vector3* vertexPositions = mesh.getIndexedPositions();
  Vector2* vertexTextureCoords = mesh.getIndexedTextureCoords();
  Vector3* vertexNormals = mesh.getIndexedNormals();
  
  numVertices = mesh2.getNumIndexedVertices();
  Vector3* vertexPositions2 = mesh2.getIndexedPositions();
  Vector2* vertexTextureCoords2 = mesh2.getIndexedTextureCoords();
  Vector3* vertexNormals2 = mesh2.getIndexedNormals();

  glGenBuffers(1, &positions_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexPositions, GL_STATIC_DRAW);

  glGenBuffers(1, &textureCoords_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector2), vertexTextureCoords, GL_STATIC_DRAW);

  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexNormals, GL_STATIC_DRAW);
  
  glGenBuffers(1, &positions_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexPositions2, GL_STATIC_DRAW);

  glGenBuffers(1, &textureCoords_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector2), vertexTextureCoords2, GL_STATIC_DRAW);

  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexNormals2, GL_STATIC_DRAW);

  unsigned int* indexData = mesh.getTriangleIndices();
  int numTriangles = mesh.getNumTriangles();
  
  unsigned int* indexData2 = mesh2.getTriangleIndices();
  int numTriangles2 = mesh2.getNumTriangles();

  glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numTriangles * 3 * 2, indexData, GL_STATIC_DRAW);
}

static void update(void) {
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    
    if (angle >= 320) {
        rotateObject = false;
    }
    // rotate the shape about the y-axis so that we can see the shading
    if (rotateObject) {
        
        
      float degrees = (float)milliseconds / 20.0f;
      angle = degrees;
    }

    // move the light position over time along the x-axis, so we can see how it affects the shading
    if (animateLight) {
      float t = milliseconds / 1000.0f;
      lightOffsetY = sinf(t) * 100;
    }
    
    if (scaleFactor <= 35.0f) {
      scaleFactor = 5.0f + (float)milliseconds / 80.0f;
    }
    
    if (eyePosition.y >= -5.0f) {
        eyePosition.y = 50.0f - (float)milliseconds / 80.0f;
    }
    
    
    glutPostRedisplay();
}


static void render(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // activate our shader program
	glUseProgram(programId);

  // turn on depth buffering
  glEnable(GL_DEPTH_TEST);

  // projection matrix - perspective projection
  // FOV:           45°
  // Aspect ratio:  4:3 ratio
  // Z range:       between 0.1 and 100.0
  float aspectRatio = (float)width / (float)height;
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

  // projection matrix - orthographic (non-perspective) projection
  // Note:  These are in world coordinates
  // xMin:          -10
  // xMax:          +10
  // yMin:          -10
  // yMax:          +10
  // zMin:           0
  // zMax:          +100
  //glm::mat4 projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);

  // view matrix - orient everything around our preferred view
  glm::mat4 view = glm::lookAt(
      eyePosition,
      glm::vec3(0,0,0),    // where to look
      glm::vec3(0,1,0)     // up
  );

  // model matrix: translate, scale, and rotate the model
  glm::vec3 rotationAxis(0,1,0);
  glm::mat4 model = glm::mat4(1.0f);
  
  model = glm::translate(model, glm::vec3(3.0f, -2.0f, 1.0));
  model = glm::translateB(model, glm::vec3(3.0f, -2.0f, 1.0));
  
  model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0)); // rotate about the y-axis
  model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));

  // model-view-projection matrix
  glm::mat4 mvp = projection * view * model;
  GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVPMatrix");
  glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

  // model-view matrix
  glm::mat4 mv = view * model;
  GLuint mvMatrixId = glGetUniformLocation(programId, "u_MVMatrix");
  glUniformMatrix4fv(mvMatrixId, 1, GL_FALSE, &mv[0][0]);

  // the position of our camera/eye
  GLuint eyePosId = glGetUniformLocation(programId, "u_EyePosition");
  glUniform3f(eyePosId, eyePosition.x, eyePosition.y, eyePosition.z);

  // the position of our light
  GLuint lightPosId = glGetUniformLocation(programId, "u_LightPos");
  glUniform3f(lightPosId, 1, 8 + lightOffsetY, -2);

  // the colour of our object
  GLuint diffuseColourId = glGetUniformLocation(programId, "u_DiffuseColour");
  glUniform4f(diffuseColourId, 0.3, 0.2, 0.8, 1.0);

  // the shininess of the object's surface
  GLuint shininessId = glGetUniformLocation(programId, "u_Shininess");
  glUniform1f(shininessId, 10000);

  // find the names (ids) of each vertex attribute
  GLint positionAttribId = glGetAttribLocation(programId, "position");
  GLint textureCoordsAttribId = glGetAttribLocation(programId, "textureCoords");
  GLint normalAttribId = glGetAttribLocation(programId, "normal");

  // provide the vertex positions to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glEnableVertexAttribArray(positionAttribId);
  glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // provide the vertex texture coordinates to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
  glEnableVertexAttribArray(textureCoordsAttribId);
  glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // provide the vertex normals to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glEnableVertexAttribArray(normalAttribId);
  glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// draw the triangles
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);

  // for testing purposes
  //glutSolidTorus(0.5f, 1.5f, 12, 10);

	// disable the attribute arrays
  glDisableVertexAttribArray(positionAttribId);
  glDisableVertexAttribArray(textureCoordsAttribId);
  glDisableVertexAttribArray(normalAttribId);

	// make the draw buffer to display buffer (i.e. display what we have drawn)
	glutSwapBuffers();
}

static void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    width = w;
    height = h;
}

static void drag(int x, int y) {
  if (!isinf(lastX) && !isinf(lastY)) {
    float dx = lastX - (float)x;
    float dy = lastY - (float)y;
    float distance = sqrt(dx * dx + dy * dy);

    if (dy > 0.0f) {
      scaleFactor = SCALE_FACTOR / distance;
    } else {
      scaleFactor = distance / SCALE_FACTOR;
    }
  } else {
    lastX = (float)x;
    lastY = (float)y;
  }
}

static void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
      lastX = std::numeric_limits<float>::infinity();
      lastY = std::numeric_limits<float>::infinity();
    }
}

static void keyboard(unsigned char key, int x, int y) {
    std::cout << "Key pressed: " << key << std::endl;
    if (key == 'l') {
      animateLight = !animateLight;
    } else if (key == 'r') {
      rotateObject = !rotateObject;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CSCI 3090u Shading in OpenGL");
    glutIdleFunc(&update);
    glutDisplayFunc(&render);
    glutReshapeFunc(&reshape);
    glutMotionFunc(&drag);
    glutMouseFunc(&mouse);
    glutKeyboardFunc(&keyboard);

    glewInit();
    if (!GLEW_VERSION_2_0) {
        std::cerr << "OpenGL 2.0 not available" << std::endl;
        return 1;
    }
    std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
		std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;

    createGeometry();
    ShaderProgram program;
    
    program.loadShaders("shaders/proc_marble_vertex.glsl", "shaders/proc_marble_fragment.glsl");
    //program.loadShaders("shaders/phong_vertex.glsl", "shaders/phong_fragment.glsl");
  	programId = program.getProgramId();

    glutMainLoop();

    return 0;
}
