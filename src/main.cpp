//////////////////////////////////////////////////////////////////////////////
//
//  Sphere.cpp
//  1. 球体的绘制（求出球面上所有的点）
//  2. 三角面片的构造
//  3. 利用统一变量进行数据传递
//////////////////////////////////////////////////////////////////////////////

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vmath.h>
#include <vector>
#include <Windows.h>

//日期
static GLfloat day = 0.0;

//窗口尺寸参数
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//视角参数
/*
* perspective(Fovy, Aspact, ZNear, ZFar)：视角改变
* Fovy是眼睛上下睁开的幅度，角度值，值越小，视野范围越狭小（眯眼），值越大，视野范围越宽阔（睁开铜铃般的大眼）；
* Aspact表示裁剪面的宽w高h比，这个影响到视野的截面有多大（这里设置成和显示区域的宽高比一致即可，比如800*600，则设置成4/3）；
* ZNear表示近裁剪面到眼睛的距离，ZFar表示远裁剪面到眼睛的距离。注意zNear和zFar不能设置设置为负值（你怎么看到眼睛后面的东西）。
*/
const float fovy = 80;
float aspact = (float)SCR_WIDTH / (float)SCR_HEIGHT;
const float znear = 1;
const float zfar = 800;

//增加旋转参数
static GLfloat xRot = 20.0f;
static GLfloat yRot = 20.0f;

//句柄参数
GLuint vertex_array_object; // == VAO句柄
GLuint vertex_buffer_object; // == VBO句柄
GLuint element_buffer_object;//==EBO句柄
int shader_program;//着色器程序句柄

//球面顶点数据
std::vector<float> sphereVertices;
std::vector<int> sphereIndices;
const int Y_SEGMENTS = 10;
const int X_SEGMENTS = 10;
const float Radio = 2.0;
const GLfloat  PI = 3.14159265358979323846f;
GLfloat radius = 1.0;


void initial(void)
{
	//进行球体顶点和三角面片的计算
	// 生成球的顶点
	for (int y = 0; y <= Y_SEGMENTS; y++)
	{
		for (int x = 0; x <= X_SEGMENTS; x++)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = radius * std::cos(xSegment * Radio * PI) * std::sin(ySegment * PI);
			float yPos = radius * std::cos(ySegment * PI);
			float zPos = radius * std::sin(xSegment * Radio * PI) * std::sin(ySegment * PI);

			sphereVertices.push_back(xPos);
			sphereVertices.push_back(yPos);
			sphereVertices.push_back(zPos);
		}
	}

	// 生成球的顶点索引
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

	// 球
	glGenVertexArrays(1, &vertex_array_object);
	glGenBuffers(1, &vertex_buffer_object);
	//生成并绑定球体的VAO和VBO
	glBindVertexArray(vertex_array_object);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	// 将顶点数据绑定至当前默认的缓冲中
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &element_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

	// 设置顶点属性指针
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 解绑VAO和VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 顶点着色器和片段着色器源码
	const char* vertex_shader_source =
		"#version 330 core\n"
		"layout (location = 0) in vec3 vPos;\n"           // 位置变量的属性位置值为0
		"out vec4 vColor;\n"           // 位置变量的属性位置值为0
		"uniform mat4 transform;\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = transform*vec4(vPos, 1.0);\n"
		"    vColor = color;\n"
		"}\n\0";
	const char* fragment_shader_source =
		"#version 330 core\n"
		"in vec4 vColor;\n"                   // 输入的颜色向量
		"out vec4 FragColor;\n"           // 输出的颜色向量
		"void main()\n"
		"{\n"
		"    FragColor = vColor;\n"
		"}\n\0";

	// 生成并编译着色器
	// 顶点着色器
	int success;
	char info_log[512];
	int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	// 检查着色器是否成功编译，如果编译失败，打印错误信息
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// 片段着色器
	int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	// 检查着色器是否成功编译，如果编译失败，打印错误信息
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// 链接顶点和片段着色器至一个着色器程序
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	// 检查着色器是否成功链接，如果链接失败，打印错误信息
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	}

	// 删除着色器
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glUseProgram(shader_program);

	//设定点线面的属性
	glPointSize(3);//设置点的大小
	glLineWidth(1);//设置线宽

	glEnable(GL_CULL_FACE);//启用剔除
	glCullFace(GL_BACK);//剔除多边形背面
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//指定多边形模式为线条

	//启用深度测试
	glEnable(GL_DEPTH_TEST);
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
		glEnable(GL_CULL_FACE);    //打开背面剔除
		glCullFace(GL_BACK);          //剔除多边形的背面
		break;
	case GLFW_KEY_4:
		glDisable(GL_CULL_FACE);     //关闭背面剔除
		break;
	default:
		break;
	}
}

void Draw(void)
{
	// 清空颜色缓冲
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned int transformLoc = glGetUniformLocation(shader_program, "transform");
	unsigned int colorLoc = glGetUniformLocation(shader_program, "color");

	// 处理图形的颜色
	GLfloat vColor[3][4] = {
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f } };

	// 绑定VAO
	glBindVertexArray(vertex_array_object);

	/*
	* 
	translate(x, y, z)：图形平移（相对于显示区域而言）
	此处x，y=0表示在屏幕中间，z=-5表示图形在屏幕里面（离摄像机）5个单位距离
	*/
	
	// 创建观察矩阵、投影矩阵
	//vmath::mat4 view = vmath::lookat(vmath::vec3(0.0, 5.0, 0.0), vmath::vec3(0.0, 0.0, -10.0), vmath::vec3(0.0, 1.0, 0.0));
	vmath::mat4 projection = vmath::perspective(fovy, aspact, znear, zfar);
	vmath::mat4 trans = projection;

	// 画太阳
	
	trans *= vmath::translate(0.0f, 0.0f, -10.0f);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans);
	glUniform4fv(colorLoc, 1, vColor[0]);
	glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);                                          // 绘制三角形
	
	// 画地球
	float a_earth = 6.0f;
	float b_earth = 2.0f;
	GLfloat angle_earth = day;
	float x_earth = a_earth * cosf(angle_earth * (float)PI / 180.0f);
	float y_earth = b_earth * sinf(angle_earth * (float)PI / 180.0f);
	float d_earth = sqrtf(x_earth * x_earth + y_earth * y_earth);

	trans *= vmath::rotate(angle_earth, vmath::vec3(0.0f, 0.0f, 1.0f));	// 4.设置旋转轴方向
	
	trans *= vmath::translate(d_earth, 0.0f, 0.0f);				// 3.设置旋转半径
	
	//trans *= vmath::rotate(43.0f * (float)PI / 180.0f, vmath::vec3(0.0f, 0.0f, 1.0f));
	//trans *= vmath::rotate(yRot, vmath::vec3(0.0, 1.0, 0.0));	// 2.自转
	trans *= vmath::scale(0.5f);								// 1.缩放
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans);
	glUniform4fv(colorLoc, 1, vColor[1]);
	glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

	// 画月球
	float a_moon = 6.0f;
	float b_moon = 2.0f;
	//GLfloat angle_moon = day / 30.0 * 360.0 - day / 360.0 * 360.0;
	GLfloat angle_moon = day *12;
	//while (angle_moon > 360) angle_moon -= 360;
	float x_moon = a_moon * cosf(angle_moon * (float)PI / 180.0f);
	float y_moon = b_moon * sinf(angle_moon * (float)PI / 180.0f);
	float d_moon = sqrtf(x_moon * x_moon + y_moon * y_moon);

	//trans *= vmath::rotate(angle_moon, vmath::vec3(1.0f, 1.0f * tanf(23.0f * (float)PI / 180.0f), 0.0f));	// 4.设置旋转轴方向
	trans *= vmath::rotate(angle_moon, vmath::vec3(0.0f, 1.0f, 0.0f));
	trans *= vmath::translate(2.0f, 0.0f, 0.0f);				// 3.设置旋转半径
	trans *= vmath::scale(0.5f);								// 1.缩放
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans);
	glUniform4fv(colorLoc, 1, vColor[2]);
	glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

	// 解除绑定
	glBindVertexArray(0);

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
	glfwInit(); // 初始化GLFW

	// OpenGL版本为3.3，主次版本号均设为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	// 使用核心模式(无需向后兼容性)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口(宽、高、窗口名称)
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LTH I Love You!!!!", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to Create OpenGL Context" << std::endl;
		glfwTerminate();
		return -1;
	}

	// 将窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(window);

	// 初始化GLAD，加载OpenGL函数指针地址的函数
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	initial();

	//窗口大小改变时调用reshaper函数
	glfwSetFramebufferSizeCallback(window, reshaper);

	//窗口中有键盘操作时调用key_callback函数
	glfwSetKeyCallback(window, key_callback);

	std::cout << "方向键可以控制图形的旋转。" << std::endl;
	std::cout << "数字键1，2设置多边形模式为线模式和填充模式。" << std::endl;
	std::cout << "数字键3打开剔除模式并且剔除多边形的背面。" << std::endl;
	std::cout << "数字键4关闭剔除模式。" << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		day++;
		if (day >= 360)
			day = 0;
		Draw();
		Sleep(30);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 解绑和删除VAO和VBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &vertex_array_object);
	glDeleteBuffers(1, &vertex_buffer_object);

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
