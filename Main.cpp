#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.84f, 0.0f, 1.0f);\n"
"}\n\0";

int main() 
{
	glfwInit(); //librería para ventanas.

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Solo pone la version de opengl que usamos
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creamos la ventana, le ponemos tamaño y si es fullscreen o no y tal.
	GLFWwindow* window = glfwCreateWindow(800, 800, "Ventana", NULL, NULL);

	if (window == NULL) // Si hubo un error y la ventana no se pudo crear
	{
		std::cout << "No se pudo crear la ventana" << std::endl; 
		glfwTerminate(); // Apagamos la librería de ventanas.
		return -1; 
	}
	glfwMakeContextCurrent(window); 

	gladLoadGL(); 

	glViewport(0, 0, 800, 800); // Le decimos a la gráfica qué pedazo de la ventana usar 

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); 
	glCompileShader(vertexShader); 

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // color
	glCompileShader(fragmentShader); 

	GLuint shaderProgram = glCreateProgram(); //aca se juntan los fragments y vertices.
	glAttachShader(shaderProgram, vertexShader); 
	glAttachShader(shaderProgram, fragmentShader); // Le pegamos el color.
	glLinkProgram(shaderProgram); 

	glDeleteShader(vertexShader); // Borramos el ocupar memoria.
	glDeleteShader(fragmentShader); 

	
	GLfloat vertices[] = // Aquí hacemos una lista con los 4 puntos (esquinas) de nuestro cuadrado.
	{
		 0.5f,  0.5f, 0.0f, // Arriba a la derecha (Índice 0)
		 0.5f, -0.5f, 0.0f, // Abajo a la derecha (Índice 1)
		-0.5f, -0.5f, 0.0f, // Abajo a la izquierda (Índice 2)
		-0.5f,  0.5f, 0.0f  // Arriba a la izquierda (Índice 3)
	};

	
	GLuint indices[] = // La gráfica solo dibuja triángulos. Aquí le decimos cómo unir los puntos de arriba.
	{
		0, 1, 3, // Primer triángulo (mitad derecha superior del cuadrado)
		1, 2, 3  // Segundo triángulo (mitad izquierda inferior del cuadrado)
	};

	GLuint VAO, VBO, EBO; // Creamos 3 "cajas" para guardar datos en la tarjeta gráfica.

	glGenVertexArrays(1, &VAO); // Creamos la caja VAO (guarda TODA la configuración del dibujo).
	glGenBuffers(1, &VBO); // Creamos la caja VBO (guarda las coordenadas de las esquinas).
	glGenBuffers(1, &EBO); // Creamos la caja EBO (guarda el orden para hacer los triángulos).

	glBindVertexArray(VAO); 

	glBindBuffer(GL_ARRAY_BUFFER, VBO); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Metemos nuestras esquinas adentro.

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Metemos el orden de los triángulos adentro.

	// Le explicamos a la gráfica cómo entender las esquinas, es una vaina ahi rara de 3 en 3 para las coordenadas X, Y, Z).
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 

	//while para mantener la imagen en la pantalla
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f); //color de fondo
		glClear(GL_COLOR_BUFFER_BIT); //le pone el color a la pantalla

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // aqui dibuja los triangulos en 6 puntos de orden.
	
		glfwSwapBuffers(window); 
		glfwPollEvents(); 
	}
	//limpiamos la memoria:
	glDeleteVertexArrays(1, &VAO); 
	glDeleteBuffers(1, &VBO); 
	glDeleteBuffers(1, &EBO); 
	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window); 
	glfwTerminate();

	return 0; 
}