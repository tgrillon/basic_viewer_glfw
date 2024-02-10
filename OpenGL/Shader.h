#include <glad/glad.h>
#include <map>

class Shader { 
public:
    Shader(): program(0) {}
    Shader(int program): program(program) {}

    void use(){
        glUseProgram(program);
    }

    int getUniform(char* name) {
        int loc = uniforms[name];
        if (loc != 0){
            return loc;
        }

        loc = glGetUniformLocation(program, name);
        uniforms[name] = loc;
        return loc;
    }

    void setMatrix4f(char* name, GLfloat* data, GLboolean transpose = false){
        glUniformMatrix4fv(getUniform(name), 1, transpose, data);
    }

    void setVec4f(char* name, GLfloat* data){
        glUniform4fv(getUniform(name), 1, data);
    }
    
    void setFloat(char* name, float data){
        glUniform1f(getUniform(name), data);
    }

    static Shader loadShader(std::string src_vertex, std::string src_fragment, std::string name="") {
      unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
      const char* source_ = src_vertex.c_str();  
      glShaderSource(vshader, 1, &source_, NULL);
      glCompileShader(vshader);
      Shader::checkCompileErrors(vshader, "VERTEX", name);
      
      source_ = src_fragment.c_str();
      unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fshader, 1, &source_, NULL);
      glCompileShader(fshader);
      Shader::checkCompileErrors(fshader, "FRAGMENT", name);

      unsigned int program = glCreateProgram();
      glAttachShader(program, vshader);
      glAttachShader(program, fshader);

      glLinkProgram(program);
      Shader::checkCompileErrors(program, "PROGRAM", name);

      glDeleteShader(vshader);
      glDeleteShader(fshader);

      return Shader(program);
    }

private:
    std::unordered_map<char*, int> uniforms;
    int program;

    static void checkCompileErrors(GLuint shader, std::string type, std::string name) {
        GLint success;
        GLchar infoLog[1024];
        std::cout << "Checking shader : " << shader << " " << type << "_" << name << "\n";

        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            std::cout << "success : " << success << "\n";

            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "_" << name << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            std::cout << "success : " << success << "\n";

            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "_" << name << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

    int getUniformLocation(char* name)  {
        if (uniforms.find(name) != uniforms.end())
            return uniforms[name];

        int location = glGetUniformLocation(program, name);
        if (location == -1)
            std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
        uniforms[name] = location;
        return location;
    }
};