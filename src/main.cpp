//////////////////////////////////////////////////////////////////////////////
//
//  Sphere.cpp
//  1. ����Ļ��ƣ�������������еĵ㣩
//  2. ������Ƭ�Ĺ���
//  3. ����ͳһ�����������ݴ���
//////////////////////////////////////////////////////////////////////////////

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vmath.h>
#include <vector>

//���ڳߴ����
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float aspact = (float)4.0 / (float)3.0;

//������ת����
static GLfloat xRot = 20.0f;
static GLfloat yRot = 20.0f;

//�������
GLuint vertex_array_object; // == VAO���
GLuint vertex_buffer_object; // == VBO���
GLuint element_buffer_object;//==EBO���
int shader_program;//��ɫ��������

//���涥������
std::vector<float> sphereVertices;
std::vector<int> sphereIndices;
const int Y_SEGMENTS = 10;
const int X_SEGMENTS = 10;
const float Radio = 2.0;
const GLfloat  PI = 3.14159265358979323846f;


void initial(void)
{
	//�������嶥���������Ƭ�ļ���
	// ������Ķ���
	for (int y = 0; y <= Y_SEGMENTS; y++)
	{
		for (int x = 0; x <= X_SEGMENTS; x++)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * Radio * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * Radio * PI) * std::sin(ySegment * PI);

			sphereVertices.push_back(xPos);
			sphereVertices.push_back(yPos);
			sphereVertices.push_back(zPos);
		}
	}

	// ������Ķ�������
	for (int i = 0; i < Y_SEGMENTS; i++)
	{
		for (int j = 0; j < X_SEGMENTS; j++)
		{

			sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);

			sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
			sphereIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
		}
	}

	// ��
	glGenVertexArrays(1, &vertex_array_object);
	glGenBuffers(1, &vertex_buffer_object);
	//���ɲ��������VAO��VBO
	glBindVertexArray(vertex_array_object);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	// ���������ݰ�����ǰĬ�ϵĻ�����
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &element_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

	// ���ö�������ָ��
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// ���VAO��VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ������ɫ����Ƭ����ɫ��Դ��
	const char* vertex_shader_source =
		"#version 330 core\n"
		"layout (location = 0) in vec3 vPos;\n"           // λ�ñ���������λ��ֵΪ0
		"out vec4 vColor;\n"           // λ�ñ���������λ��ֵΪ0
		"uniform mat4 transform;\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = transform*vec4(vPos, 1.0);\n"
		"    vColor = color;\n"
		"}\n\0";
	const char* fragment_shader_source =
		"#version 330 core\n"
		"in vec4 vColor;\n"                   // �������ɫ����
		"out vec4 FragColor;\n"           // �������ɫ����
		"void main()\n"
		"{\n"
		"    FragColor = vColor;\n"
		"}\n\0";

	// ���ɲ�������ɫ��
	// ������ɫ��
	int success;
	char info_log[512];
	int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	// �����ɫ���Ƿ�ɹ����룬�������ʧ�ܣ���ӡ������Ϣ
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// Ƭ����ɫ��
	int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	// �����ɫ���Ƿ�ɹ����룬�������ʧ�ܣ���ӡ������Ϣ
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// ���Ӷ����Ƭ����ɫ����һ����ɫ������
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	// �����ɫ���Ƿ�ɹ����ӣ��������ʧ�ܣ���ӡ������Ϣ
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	}

	// ɾ����ɫ��
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glUseProgram(shader_program);

	//�趨�����������
	glPointSize(3);//���õ�Ĵ�С
	glLineWidth(1);//�����߿�

	glEnable(GL_CULL_FACE);//�����޳�
	glCullFace(GL_BACK);//�޳�����α���
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//ָ�������ģʽΪ����
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_UP:
		xRot -= 5.0f;
		break;
	case GLFW_KEY_DOWN:
		xRot += 5.0f;
		break;
	case GLFW_KEY_LEFT:
		yRot -= 5.0f;
		break;
	case GLFW_KEY_RIGHT:
		yRot += 5.0f;
		break;
	case GLFW_KEY_1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case GLFW_KEY_2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case GLFW_KEY_3:
		glEnable(GL_CULL_FACE);    //�򿪱����޳�
		glCullFace(GL_BACK);          //�޳�����εı���
		break;
	case GLFW_KEY_4:
		glDisable(GL_CULL_FACE);     //�رձ����޳�
		break;
	default:
		break;
	}
}

void Draw(void)
{
	// �����ɫ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//����ͼ�ε���ת
	//perspective(Fovy, Aspact, ZNear, ZFar)���ӽǸı�
	// Fovy���۾����������ķ��ȣ��Ƕ�ֵ��ֵԽС����Ұ��ΧԽ��С�����ۣ���ֵԽ����Ұ��ΧԽ����������ͭ���Ĵ��ۣ���
	// Aspact��ʾ�ü���Ŀ�w��h�ȣ����Ӱ�쵽��Ұ�Ľ����ж���������óɺ���ʾ����Ŀ�߱�һ�¼��ɣ�����800*600�������ó�4/3����
	// ZNear��ʾ���ü��浽�۾��ľ��룬ZFar��ʾԶ�ü��浽�۾��ľ��롣ע��zNear��zFar������������Ϊ��ֵ������ô�����۾�����Ķ�������

	//translate(x, y, z)��ͼ��ƽ�ƣ��������ʾ������ԣ�
	// �˴�x��y=0��ʾ����Ļ�м䣬z=-5��ʾͼ������Ļ���棨���������5����λ����
	vmath::mat4 trans = vmath::perspective(40, aspact, 1.0f, 800.0f) * vmath::translate(0.0f, 0.0f, -5.0f) * vmath::rotate(xRot, vmath::vec3(1.0, 0.0, 0.0)) * vmath::rotate(yRot, vmath::vec3(0.0, 1.0, 0.0));
	unsigned int transformLoc = glGetUniformLocation(shader_program, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans);

	//����ͼ�ε���ɫ
	GLfloat vColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	unsigned int colorLoc = glGetUniformLocation(shader_program, "color");
	glUniform4fv(colorLoc, 1, vColor);

	// ����������
	glBindVertexArray(vertex_array_object);                                    // ��VAO
	glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);                                          // ����������
	glBindVertexArray(0);                                                      // �����

}

void reshaper(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	if (height == 0)
	{
		aspact = (float)width;
	}
	else
	{
		aspact = (float)width / (float)height;
	}

}

int main()
{
	glfwInit(); // ��ʼ��GLFW

	// OpenGL�汾Ϊ3.3�����ΰ汾�ž���Ϊ3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	// ʹ�ú���ģʽ(������������)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ��������(���ߡ���������)
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sphere", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to Create OpenGL Context" << std::endl;
		glfwTerminate();
		return -1;
	}

	// �����ڵ�����������Ϊ��ǰ�̵߳���������
	glfwMakeContextCurrent(window);

	// ��ʼ��GLAD������OpenGL����ָ���ַ�ĺ���
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	initial();

	//���ڴ�С�ı�ʱ����reshaper����
	glfwSetFramebufferSizeCallback(window, reshaper);

	//�������м��̲���ʱ����key_callback����
	glfwSetKeyCallback(window, key_callback);

	std::cout << "��������Կ���ͼ�ε���ת��" << std::endl;
	std::cout << "���ּ�1��2���ö����ģʽΪ��ģʽ�����ģʽ��" << std::endl;
	std::cout << "���ּ�3���޳�ģʽ�����޳�����εı��档" << std::endl;
	std::cout << "���ּ�4�ر��޳�ģʽ��" << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		Draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ����ɾ��VAO��VBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &vertex_array_object);
	glDeleteBuffers(1, &vertex_buffer_object);

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
