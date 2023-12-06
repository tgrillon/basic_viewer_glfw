#include <CGAL/Graphics_scene.h>
#include <CGAL/Basic_shaders.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Implémenter camera.h / frame (coordinate) system si nécessaire
 * 
 * set l'uniform mvp une fois par 
 * 
 * voir QGLBuffer
 * 
 * glGenBuffers
 * glBindBuffer
 * glBufferData
 * 
 * vao:
 * enableAttributeArray
 * setAttributeArray
 * 
 * void initialize_buffers
 * 
*/


const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n";

namespace CGAL {
namespace OpenGL {

    const int         windowWidth = 1024;
    const int         windowHeight = 768;
    const int         windowSamples = 4;

    void glfwErrorCallback(int error, const char* description)
    {
        fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
    }

    GLFWwindow* initialise(const char* title)
    {
        // Initialise GLFW
        if (!glfwInit())
        {
            fprintf(stderr, "Could not start GLFW\n");
            exit(EXIT_FAILURE);
        }

        // OpenGL 2.1 with compatibilty 
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

        // Enable the GLFW runtime error callback function defined previously.
        glfwSetErrorCallback(glfwErrorCallback);

        // Set additional window options
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        glfwWindowHint(GLFW_SAMPLES, windowSamples);  // MSAA

        // Create window using GLFW
        GLFWwindow* window = glfwCreateWindow(windowWidth,
            windowHeight,
            title,
            nullptr,
            nullptr);

        // Ensure the window is set up correctly
        if (!window)
        {
            fprintf(stderr, "Could not open GLFW window\n");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        // Let the window be the current OpenGL context and initialise glad
        glfwMakeContextCurrent(window);
        gladLoadGL();

        // White background
        glClearColor(1.f, 1.f, 1.f, 1.f);

        // Print various OpenGL information to stdout
        printf("%s: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
        printf("GLFW\t %s\n", glfwGetVersionString());
        printf("OpenGL\t %s\n", glGetString(GL_VERSION));
        printf("GLSL\t %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

        return window;
    }

    class Basic_Viewer {
    public:

        Basic_Viewer(const Graphics_scene& graphics_scene, const char* title)
        : m_scene(graphics_scene), m_title(title) 
        {

        }

        void show(){
            m_window = initialise(m_title);

            initialiseBuffers();
            
            while (!glfwWindowShouldClose(m_window)){

                // Process inputs

                // Rendering
                renderScene();
                
                glfwSwapBuffers(m_window);
                glfwPollEvents();
            }

            glfwTerminate();
        }
        

    private:
        GLFWwindow* m_window;
        const Graphics_scene& m_scene;
        const char* m_title;

        unsigned int m_vbo, m_vao;

        unsigned int m_render_p1;

        void compileShader(){
            
        }

        void initialiseBuffers(){
            glGenBuffers(1, &m_vbo);
            glGenVertexArrays(1, &m_vao);

            unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
            unsigned int fshader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vshader, 1, &vertexShaderSource, NULL);
            glShaderSource(fshader, 1, &fragmentShaderSource, NULL);
            glCompileShader(vshader);
            glCompileShader(fshader);

            m_render_p1 = glCreateProgram();
            glAttachShader(m_render_p1, vshader);
            glAttachShader(m_render_p1, fshader);

            glLinkProgram(m_render_p1);
        }

        void renderScene(){
            glBindVertexArray(m_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

            glBufferData(GL_ARRAY_BUFFER, 
                m_scene.get_size_of_index(Graphics_scene::POS_MONO_FACES) * sizeof(float), 
                m_scene.get_array_of_index(Graphics_scene::POS_MONO_FACES).data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);  

            glUseProgram(m_render_p1);
            glDrawArrays(GL_TRIANGLES, 0, m_scene.get_size_of_index(Graphics_scene::POS_MONO_FACES));
        }

    };



    // Inside OpenGL Namespace
    inline
    void draw_graphics_scene(const Graphics_scene& graphics_scene,
        const char* title = "CGAL Basic Viewer") {
        
    }
} // End OpenGL Namespace
} // End CGAL Namespace