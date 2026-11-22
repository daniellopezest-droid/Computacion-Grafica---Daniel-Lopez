#include "shaderClass.h"

std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw(errno);
}

static GLuint compilarShader(GLenum tipoShader, const char* fuente)
{
	GLuint shader = glCreateShader(tipoShader);
	glShaderSource(shader, 1, &fuente, NULL);
	glCompileShader(shader);

	int exito;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &exito);
	if (!exito)
	{
		char registroInfo[512];
		glGetShaderInfoLog(shader, 512, NULL, registroInfo);
		std::cout << "Error de compilacion en el Shader: " << registroInfo << "\n";
	}
	return shader;
}

Shader::Shader(const char* vertexfile, const char* fragmentfile)
{
	std::string vertexCode = get_file_contents(vertexfile);
	std::string fragmentCode = get_file_contents(fragmentfile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint shaderVert = compilarShader(GL_VERTEX_SHADER, vertexSource);
	GLuint shaderFrag = compilarShader(GL_FRAGMENT_SHADER, fragmentSource);

	ID = glCreateProgram();
	glAttachShader(ID, shaderVert);
	glAttachShader(ID, shaderFrag);
	glLinkProgram(ID);

	int exito;
	glGetProgramiv(ID, GL_LINK_STATUS, &exito);
	if (!exito)
	{
		char registroInfo[512];
		glGetProgramInfoLog(ID, 512, NULL, registroInfo);
		std::cout << "Error al enlazar el programa de Shaders: " << registroInfo << "\n";
	}

	glDeleteShader(shaderVert);
	glDeleteShader(shaderFrag);
}

void Shader::Activate()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}