//g++ space.cpp -o space -lGL -lglut -lGLEW

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct vertex {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 normal;
} typedef Vertex;

struct transform {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
} typedef Transform;

struct ship {
	long begin, end;
	float speed;
	glm::vec3 position, direction, up;
	glm::mat4 rotation;
} typedef Ship;

struct station {
	long begin, end;
	float speed;
	glm::vec3 position, front, top;
	glm::mat4 rotation;
} typedef Station;

struct planet {
	float radiusC, radiusI, radiusO, thetaX, thetaY;
	glm::vec3 center;
	glm::vec4 color;
} typedef Planet;

float width, height;
int state, mode, keyW, keyS, keyA, keyD, keyR, keyF, keyUp, keyDown, keyLeft, keyRight;

GLuint VAO, VBO, EBO, UBO, shaderProgram, planetBegin, planetEnd, starBegin, starEnd;
std::vector<Vertex> vertices;
std::vector<GLuint> indices;

Ship ship;
Station station;
std::vector <Planet> planets;
std::vector <Vertex> stars;
Transform transform;

void reshapeCallback(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);
	glutPostRedisplay();
}

void specialCallback(int k, int x, int y)
{
	if(k == GLUT_KEY_UP)
		keyUp = 1;
	else if(k == GLUT_KEY_DOWN)
		keyDown = 1;
	else if(k == GLUT_KEY_LEFT)
		keyLeft = 1;
	else if(k == GLUT_KEY_RIGHT)
		keyRight = 1;
	glutPostRedisplay();
}

void specialUpCallback(int k, int x, int y)
{
	if(k == GLUT_KEY_UP)
		keyUp = 0;
	else if(k == GLUT_KEY_DOWN)
		keyDown = 0;
	else if(k == GLUT_KEY_LEFT)
		keyLeft = 0;
	else if(k == GLUT_KEY_RIGHT)
		keyRight = 0;
}

void keyboardCallback(unsigned char c, int x, int y)
{
	if(c == '1')
		mode = 0;
	else if(c == '2')
		mode = 1;
	else if(c == '3')
		mode = 2;
	else if(c == '4')
		mode = 3;
	else if(c == 'w' || c == 'W')
		keyW = 1;
	else if(c == 's' || c == 'S')
		keyS = 1;
	else if(c == 'a' || c == 'A')
		keyA = 1;
	else if(c == 'd' || c == 'D')
		keyD = 1;
	else if(c == 'r' || c == 'R')
		keyR = 1;
	else if(c == 'f' || c == 'F')
		keyF = 1;
	else if(c == ' ')
		state = !state;
	else if(c == 27)
		glutLeaveMainLoop();
	if(c != 27)
		glutPostRedisplay();
}

void keyboardUpCallback(unsigned char c, int x, int y)
{
	if(c == 'w' || c == 'W')
		keyW = 0;
	else if(c == 's' || c == 'S')
		keyS = 0;
	else if(c == 'a' || c == 'A')
		keyA = 0;
	else if(c == 'd' || c == 'D')
		keyD = 0;
	else if(c == 'r' || c == 'R')
		keyR = 0;
	else if(c == 'f' || c == 'F')
		keyF = 0;
}

void mouseCallback(int b, int s, int x, int y)
{
	if(s == GLUT_DOWN)
		glutPostRedisplay();
}

void passiveMotionCallback(int x, int y)
{
}

void timerCallback(int id)
{
	if(state == 0)
		glutPostRedisplay();
	glutTimerFunc(16, timerCallback, 0);
}

GLuint createShader(std::string path, GLenum type)
{
	std::ifstream file;
	file.open(path.c_str());
	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	auto source = stream.str();
	auto code = source.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);

	GLint result;
	GLchar log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if(!result)
	{
		glGetShaderInfoLog(shader, 512, NULL, log);
		std::cout << log << std::endl;
	}

	return shader;
}

GLuint createProgram(GLuint vertex, GLuint fragment)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	int result;
	char log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &result);

	if(!result)
	{
		glGetProgramInfoLog(program, 512, NULL, log);
		std::cout << log << std::endl;
	}

	return program;
}

void addTetra(glm::vec4 color, glm::vec3 center, glm::vec3 axis, float theta, float length)
{
	int offset = vertices.size();
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), theta, axis);
	glm::vec4 normal[4];
	normal[0] = rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	normal[1] = rotation * glm::vec4(0.0f, -1.0f / sqrtf(2.0f), 1.0f / sqrtf(2.0f), 0.0f);
	normal[2] = rotation * glm::vec4(1.0f / sqrtf(3.0f), -1.0f / sqrtf(3.0f), -1.0f / sqrtf(3.0f), 0.0f);
	normal[3] = rotation * glm::vec4(-1.0f / sqrtf(3.0f), -1.0f / sqrtf(3.0f), -1.0f / sqrtf(3.0f), 0.0f);

	for(glm::vec4 vector : normal)
		vertices.push_back(Vertex{glm::vec4(center + glm::vec3(length * vector), 1.0f), color, vector});

	indices.push_back(offset + 0);
	indices.push_back(offset + 1);
	indices.push_back(offset + 2);

	indices.push_back(offset + 0);
	indices.push_back(offset + 2);
	indices.push_back(offset + 3);

	indices.push_back(offset + 0);
	indices.push_back(offset + 3);
	indices.push_back(offset + 1);

	indices.push_back(offset + 1);
	indices.push_back(offset + 3);
	indices.push_back(offset + 2);
}

void addCircle(glm::vec4 color, glm::vec3 center, float thetaX, float thetaY, float radiusI, float radiusO)
{
	int index = 0, sample = 24, offset = vertices.size();
	glm::mat4 translationC = glm::translate(glm::mat4(1.0f), center);
	glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), thetaX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), thetaY, glm::vec3(0.0f, 1.0f, 0.0f));

	for(int i = 0; i < sample; i++)
	{
		float theta = 2 * M_PI * i / sample;

		glm::vec4 positionI = translationC * rotationY * rotationX * glm::vec4(radiusI * cosf(theta), radiusI * sinf(theta), 0.0f, 1.0f);
		glm::vec4 positionO = translationC * rotationY * rotationX * glm::vec4(radiusO * cosf(theta), radiusO * sinf(theta), 0.0f, 1.0f);
		glm::vec4 normalT = glm::normalize(rotationY * rotationX * glm::vec4(0.0f, 0.0f,  1.0f, 0.0f));
		glm::vec4 normalB = glm::normalize(rotationY * rotationX * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));

		vertices.push_back(Vertex{positionI, color, normalT});
		vertices.push_back(Vertex{positionO, color, normalT});
		vertices.push_back(Vertex{positionI, color, normalB});
		vertices.push_back(Vertex{positionO, color, normalB});

		int index0 = index + offset;
		int index1 = index0 + 1;
		int index2 = i != sample - 1 ? index1 + 3 : offset;
		int index3 = index2 + 1;

		indices.push_back(index0);
		indices.push_back(index1);
		indices.push_back(index3);

		indices.push_back(index0);
		indices.push_back(index3);
		indices.push_back(index2);

		indices.push_back(index0 + 2);
		indices.push_back(index3 + 2);
		indices.push_back(index1 + 2);

		indices.push_back(index0 + 2);
		indices.push_back(index2 + 2);
		indices.push_back(index3 + 2);

		index += 4;
	}
}

void addTorus(glm::vec4 color, glm::vec3 center, glm::vec3 axis, float theta, float radiusO, float radiusI)
{
	int index = 0, sampleO = 32, sampleI = 16, offset = vertices.size();
	glm::vec4 vectorO(radiusO, 0.0f, 0.0f, 1.0f), vectorI(radiusI, 0.0f, 0.0f, 1.0f);
	glm::mat4 rotationA = glm::rotate(glm::mat4(1.0f), theta, axis);

	for(int i = 0; i < sampleO; i++)
	{
		float alpha = glm::radians(360.0f) * i / sampleO;
		glm::mat4 rotationO = glm::rotate(glm::mat4(1.0f), alpha, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 point = center + glm::vec3(rotationA * rotationO * vectorO);

		for(int j = 0; j < sampleI; j++)
		{
			float beta = glm::radians(360.0f) * j / sampleI;
			glm::mat4 rotationI = glm::rotate(glm::mat4(1.0f), beta, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 position = glm::vec4(point + glm::vec3(rotationA * rotationO * rotationI * vectorI), 1.0f);
			glm::vec4 normal = glm::vec4(normalize(glm::vec3(position) - point), 0.0f);

			vertices.push_back(Vertex{position, color, normal});

			int index0 = index + offset;
			int index1 = i != sampleO - 1 ? index0 + sampleI : offset + j;
			int index2 = j != sampleI - 1 ? index0 + 1 : index0 - sampleI + 1;
			int index3 = j != sampleI - 1 ? index1 + 1 : index1 - sampleI + 1;

			indices.push_back(index0);
			indices.push_back(index2);
			indices.push_back(index3);

			indices.push_back(index0);
			indices.push_back(index3);
			indices.push_back(index1);

			index++;
		}
	}
}

void addSphere(glm::vec4 color, glm::vec3 center, float radius)
{
	int index0, index1, stackCount = 24, sectorCount = 24, offset = vertices.size();
	float sectorAngle, stackAngle, lengthInv = 1.0f / radius, sectorStep = 2.0f * M_PI / sectorCount, stackStep = M_PI / stackCount;
	glm::mat4 translationC = glm::translate(glm::mat4(1.0f), center);

	for(int i = 0; i <= stackCount; i++)
	{
		stackAngle = M_PI / 2.0f - i * stackStep;
		float z = radius * sinf(stackAngle), t = radius * cosf(stackAngle);

		for(int j = 0; j <= sectorCount; j++)
		{
			sectorAngle = j * sectorStep;
			glm::vec4 position = translationC * glm::vec4(t * cosf(sectorAngle), t * sinf(sectorAngle), z, 1.0f);
			glm::vec4 normal = glm::vec4(glm::normalize(glm::vec3(position - glm::vec4(center, 1.0f))), 0.0f);
			vertices.push_back(Vertex{position, color, normal});
		}
	}

	for(int i = 0; i < stackCount; i++)
	{
		index0 = i * (sectorCount + 1);
		index1 = index0 + sectorCount + 1;

		for(int j = 0; j < sectorCount; j++)
		{
			if(i != 0)
			{
				indices.push_back(index0 + offset);
				indices.push_back(index1 + offset);
				indices.push_back(index0 + 1 + offset);
			}

			if(i != (stackCount-1))
			{
				indices.push_back(index0 + 1 + offset);
				indices.push_back(index1 + offset);
				indices.push_back(index1 + 1 + offset);
			}

			index0++;
			index1++;
		}
	}
}

void createShip(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	ship.position = position;
	ship.direction = glm::normalize(direction);
	ship.up = glm::normalize(up);
	ship.rotation = glm::mat4(1.0f);
	ship.speed = 2.0f;

	ship.begin = indices.size();
	addTetra(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -0.06f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), M_PI, 0.02f);
	addTorus(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.04f, 0.016f);
	addTorus(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), M_PI / 2, 0.04f, 0.016f);
	ship.end = indices.size();
}

void createStation(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	station.position = position;
	station.front = glm::normalize(direction);
	station.top = glm::normalize(up);
	station.rotation = glm::mat4(1.0f);
	station.speed = 0.0f;

	station.begin = indices.size();
	addTetra(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 3.6f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.8f);
	addSphere(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 3.2f);
	station.end = indices.size();
}

void createScene()
{
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(30.0f,  30.0f,  30.0f), glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(30.0f,  170.0f, 15.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(80.0f,  110.0f, 25.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(70.0f,  60.0f,  12.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(90.0f,  150.0f, 13.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(120.0f, 80.0f,  17.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(150.0f, 40.0f,  15.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)});
	planets.push_back(Planet{1.0f, 1.6f, 3.2f, 0.0f, 0.0f, glm::vec3(160.0f, 170.0f, 22.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)});

	for(int i = 0; i <40; i++)
	{
		float radiusC = rand() % 80 / 40.0f + 0.4f;
		float radiusI = radiusC + rand() % 40 / 40.0f + 0.8f;
		float radiusO = radiusI + rand() % 40 / 40.0f + 1.6f;

		planets.push_back(
			Planet {
				radiusC, radiusI, radiusO,
				(float) M_PI * (rand() % 40 / 120.0f),
				(float) M_PI * (rand() % 40 / 120.0f),
				glm::vec3(rand() % 360 - 180.0f, rand() % 360 - 180.0f, rand() % 240 - 120.0f),
				glm::vec4(rand() % 12 / 11.0f, rand() % 12 / 11.0f, rand() % 12 / 11.0f, 1.0f)
			}
		);
	}

	planetBegin = indices.size();
	for(auto planet : planets)
	{
		addSphere(planet.color, planet.center, planet.radiusC);
		addCircle(planet.color / 2.0f, planet.center, planet.thetaX, planet.thetaY, planet.radiusI, planet.radiusO);
	}
	planetEnd = indices.size();

	int offset = vertices.size();
	starBegin = indices.size();
	for(int i = 0; i < 120; i++)
	{
		int face = rand() % 2 * 2 - 1;

		stars.push_back(
			Vertex {
				glm::vec4(face * 640, rand() % 1280 - 640, rand() % 1280 - 640, 1.0f),
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				glm::vec4(-face, 0.0f, 0.0f, 0.0f)
			}
		);
		vertices.push_back(stars.back());

		stars.push_back(
			Vertex {
				glm::vec4(rand() % 1280 - 640, face * 640, rand() % 1280 - 640, 1.0f),
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				glm::vec4(0.0f, -face, 0.0f, 0.0f)
			}
		);
		vertices.push_back(stars.back());

		stars.push_back(
			Vertex {
				glm::vec4(rand() % 1280 - 640, rand() % 1280 - 640, face * 640, 1.0f),
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				glm::vec4(0.0f, 0.0f, -face, 0.0f)
			}
		);
		vertices.push_back(stars.back());

		indices.push_back(offset + i * 3);
		indices.push_back(offset + i * 3 + 1);
		indices.push_back(offset + i * 3 + 2);
	}
	starEnd = indices.size();
}

void gameInit()
{
	srand(time(NULL));

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	createShip(glm::vec3(105.0f, 0.0f, 15.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	createStation(glm::vec3(100.0f, 10.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	createScene();

	GLuint vertexShader = createShader("shader.vert", GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader("shader.frag", GL_FRAGMENT_SHADER);
	shaderProgram = createProgram(vertexShader, fragmentShader);

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(shaderProgram);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) sizeof(glm::vec4));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) (2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glUniformBlockBinding(shaderProgram, 0, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Transform), &transform, GL_DYNAMIC_DRAW);
}

void processInput()
{
	float timeD = 0.016f;
	float moveD = timeD / 0.8f, rotateD = timeD / 1.6f;

	glm::mat4 rotation;
	glm::vec3 left = glm::normalize(glm::cross(ship.up, ship.direction));

	if(keyLeft || keyRight)
	{
		rotation = glm::rotate(glm::mat4(1.0f), (keyRight - keyLeft) * rotateD, ship.direction);
		ship.up = glm::normalize(glm::vec3(rotation * glm::vec4(ship.up, 1.0f)));
		ship.rotation = rotation * ship.rotation;
		left = glm::normalize(glm::vec4(glm::cross(ship.up, ship.direction), 0.0f));
	}

	if(keyUp || keyDown)
	{
		rotation = glm::rotate(glm::mat4(1.0f), (keyUp - keyDown) * rotateD, left);
		ship.up = glm::normalize(glm::vec3(rotation * glm::vec4(ship.up, 1.0f)));
		ship.rotation = rotation * ship.rotation;
		ship.direction = glm::normalize(glm::vec4(glm::cross(left, ship.up), 0.0f));
	}

	if(keyA || keyD)
	{
		rotation = glm::rotate(glm::mat4(1.0f), (keyA - keyD) * rotateD, ship.up);
		ship.direction = glm::normalize(glm::vec3(rotation * glm::vec4(ship.direction, 1.0f)));
		ship.rotation = rotation * ship.rotation;
		left = glm::normalize(glm::vec4(glm::cross(ship.up, ship.direction), 0.0f));
	}

	if(keyW || keyS)
		ship.speed = glm::clamp(ship.speed + (keyW - keyS) * moveD, 0.0f, 2.4f);

	if(keyR || keyF)
		station.speed = glm::clamp(station.speed + (keyR - keyF) * rotateD, 0.0f, 1.2f);

	rotation = glm::rotate(glm::mat4(1.0f), rotateD * station.speed, station.top);
	station.front = glm::normalize(glm::vec3(rotation * glm::vec4(station.front, 1.0f)));
	station.rotation = rotation * station.rotation;
	ship.position = ship.position + moveD * ship.speed * ship.direction;
}

void updateCamera()
{
	glm::vec3 eye, target, up;

	if(mode == 0)
	{
		eye = ship.position + ship.up * 0.2f - ship.direction * 0.4f;
		target = ship.position;
		up = ship.up;
	}

	else if(mode == 1)
	{
		eye = ship.position + ship.direction * 0.4f;
		target = ship.position + ship.direction;
		up = ship.up;
	}

	else if(mode == 2)
	{
		eye = station.position + station.front * 4.8f;
		target = eye + station.front;
		up = station.top;
	}

	else if(mode == 3)
	{
		eye = glm::vec3(0.0f, 0.0f, 480.0f);
		target = glm::vec3(0.0f, 0.0f, 478.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	transform.view = glm::lookAt(eye, target, up);
	transform.projection = glm::perspective((float) M_PI / 3.0f, width / height, 0.1f, 1000.0f);

	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), 2 * sizeof(glm::mat4), &transform.view);
}

void drawShip()
{
	transform.model = glm::translate(glm::mat4(1.0f), ship.position) * ship.rotation;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &transform.model);
	glDrawElements(GL_TRIANGLES, ship.end - ship.begin, GL_UNSIGNED_INT, (GLvoid*) (ship.begin * sizeof(GLuint)));
}

void drawStation()
{
	transform.model = glm::translate(glm::mat4(1.0f), station.position) * station.rotation;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &transform.model);
	glDrawElements(GL_TRIANGLES, station.end - station.begin, GL_UNSIGNED_INT, (GLvoid*) (station.begin * sizeof(GLuint)));
}

void drawScene()
{
	transform.model = glm::mat4(1.0f);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &transform.model);
	glDrawElements(GL_TRIANGLES, planetEnd - planetBegin, GL_UNSIGNED_INT, (GLvoid*) (planetBegin * sizeof(GLuint)));
	glDrawElements(GL_POINTS, starEnd - starBegin, GL_UNSIGNED_INT, (GLvoid*) (starBegin * sizeof(GLuint)));
}

void printLog()
{
	printf("Camera Mode: %d\n", mode + 1);

	printf("Ship:\n\tPosition: %g %g %g\n\tDirection: %g %g %g\n\tUp: %g %g %g\n\tSpeed: %g\n",
		ship.position.x, ship.position.y, ship.position.z, ship.direction.x, ship.direction.y, ship.direction.z,
		ship.up.x, ship.up.y, ship.up.z, ship.speed
	);

	printf("Station:\n\tPosition: %g %g %g\n\tDirection: %g %g %g\n\tRotational Speed: %g\n",
		station.position.x, station.position.y, station.position.z,
		station.front.x, station.front.y, station.front.z, station.speed
	);

	//for(int i = 0; i < planets.size(); i++)
	//	printf("Planet %d Position: %g %g %g\n", planets.at(i).center.x, planets.at(i).center.y, planets.at(i).center.z);

	printf("\n");
}

void displayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processInput();
	updateCamera();
	drawShip();
	drawStation();
	drawScene();

	if(state == 1)
		printLog();

	glutSwapBuffers();
}

void gameClean()
{
	glDeleteProgram(shaderProgram);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &UBO);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 6);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1366, 768);
	glutCreateWindow("HW3");

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	//glutSetCursor(GLUT_CURSOR_NONE);

	glutReshapeFunc(reshapeCallback);
	glutSpecialFunc(specialCallback);
	glutSpecialUpFunc(specialUpCallback);
	glutKeyboardFunc(keyboardCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutMouseFunc(mouseCallback);
	glutPassiveMotionFunc(passiveMotionCallback);
	glutDisplayFunc(displayCallback);
	glutTimerFunc(16, timerCallback, 0);

	glewExperimental = GL_TRUE;
	glewInit();
	gameInit();
	glutMainLoop();
	gameClean();
}
