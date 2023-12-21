#include <CGAL/Graphics_scene.h>
#include <CGAL/Basic_shaders.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

const char vertexShaderMVP[] = R"DELIM(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
)DELIM";

const char fragment_source_color2[]=R"DELIM(
#version 150
in highp vec4 fP;
in highp vec3 fN;
in highp vec4 fColor;
in highp vec4 m_vertex;

uniform highp vec4 light_pos;
uniform highp vec4 light_diff;
uniform highp vec4 light_spec;
uniform highp vec4 light_amb;
uniform highp float spec_power;

uniform highp vec4 clipPlane;
uniform highp vec4 pointPlane;
uniform highp float rendering_mode;
uniform highp float rendering_transparency;

out highp vec4 out_color;

void main(void)
{
  highp vec3 L = light_pos.xyz - fP.xyz;
  highp vec3 V = -fP.xyz;

  highp vec3 N = normalize(fN);
  L = normalize(L);
  V = normalize(V);

  highp vec3 R = reflect(-L, N);
  highp vec4 diffuse = vec4(max(dot(N,L), 0.0) * light_diff.rgb * fColor.rgb, 1.0);
  highp vec4 ambient = vec4(light_amb.rgb * fColor.rgb, 1.0);
  highp vec4 specular = pow(max(dot(R,V), 0.0), spec_power) * light_spec;

  // onPlane == 1: inside clipping plane, should be solid;
  // onPlane == -1: outside clipping plane, should be transparent;
  // onPlane == 0: on clipping plane, whatever;
  
  /*
  float onPlane = sign(dot((m_vertex.xyz-pointPlane.xyz), clipPlane.xyz));

  // rendering_mode == -1: draw all solid;
  // rendering_mode == 0: draw solid only;
  // rendering_mode == 1: draw transparent only;
  if (rendering_mode == (onPlane+1)/2) {
    // discard other than the corresponding half when rendering
    discard;
  }
  */

  // draw corresponding part
  out_color = diffuse + ambient;
}
)DELIM";


namespace CGAL {
namespace OpenGL{

  const int windowWidth = 1024;
  const int windowHeight = 768;
  const int windowSamples = 4;

  void glfwErrorCallback(int error, const char *description)
  {
    fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
  }

  GLFWwindow *initialise(const char *title)
  {
    // Initialise GLFW
    if (!glfwInit())
    {
      fprintf(stderr, "Could not start GLFW\n");
      exit(EXIT_FAILURE);
    }

    // OpenGL 2.1 with compatibilty
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Enable the GLFW runtime error callback function defined previously.
    glfwSetErrorCallback(glfwErrorCallback);

    // Set additional window options
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, windowSamples); // MSAA

      // Create window using GLFW
    GLFWwindow *window = glfwCreateWindow(windowWidth,
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
    //glfwSetFramebufferSizeCallback(window, glViewport(0, 0, windowWidth, windowHeight));
    
    // Initialized GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cerr << "Failed to initialized GLAD!" << std::endl;
      exit(EXIT_FAILURE);
    }
    // gladLoadGL();

    // White background

    // Print various OpenGL information to stdout
    printf("%s: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("GLFW\t %s\n", glfwGetVersionString());
    printf("OpenGL\t %s\n", glGetString(GL_VERSION));
    printf("GLSL\t %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return window;
  }

  class Basic_Viewer
  {
  public:
    Basic_Viewer(const Graphics_scene &graphics_scene,
                  const char *title,
                  bool draw_vertices = false,
                  bool draw_edges = true,
                  bool draw_faces = true,
                  bool draw_rays = true,
                  bool draw_lines = true) : 
    m_scene(graphics_scene),
    m_title(title),
    m_draw_vertices(draw_vertices),
    m_draw_edges(draw_edges),
    m_draw_rays(draw_rays),
    m_draw_lines(draw_lines),
    m_draw_faces(draw_faces),
    m_are_buffers_initialized(false),
    m_size_points(7.),
    m_ambient_color(0.6f, 0.5f, 0.5f, 1.f)
    {
      // modelView = glm::translate(glm::mat4(1.f), glm::vec3(1,0,0));
      m_faces_mono_color = glm::vec4(1.f,0.5f,1.f,1.0f);

      modelView = glm::lookAt(glm::vec3(0.0f,0.0f,10.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec3(0.0f,1.0f,0.0f));
  
      modelViewProjection = 
        glm::perspective(glm::radians(45.f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f)
        *
        modelView;
    }

    void show()
    {
    m_window = initialise(m_title);
    compileShader();

    float test = 0;
      while (!glfwWindowShouldClose(m_window))
      {
          test += 0.7;
        // Process inputs

        // Rendering
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderScene(test);

        glfwSwapBuffers(m_window);
        glfwPollEvents();

        
      }

      glfwTerminate();
    }

  private:
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        std::cout << "Checking shader : " << shader << " " << type << "\n";

        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            std::cout << "success : " << success << "\n";

            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            std::cout << "success : " << success << "\n";

            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

    void compileShader()
    {
      const char* source_ = vertex_source_color; // vertexShaderMVP;
      unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vshader, 1, &source_, NULL);
      glCompileShader(vshader);
      checkCompileErrors(vshader, "VERTEX");
      
      source_ = fragment_source_color2; // fragmentShaderMVP;
      unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fshader, 1, &source_, NULL);
      glCompileShader(fshader);
      checkCompileErrors(fshader, "FRAGMENT");

      m_render_p1 = glCreateProgram();
      glAttachShader(m_render_p1, vshader);
      glAttachShader(m_render_p1, fshader);

      glLinkProgram(m_render_p1);
      checkCompileErrors(m_render_p1, "PROGRAM");

      glDeleteShader(vshader);
      glDeleteShader(fshader);
    }


    // Stocker buffers
    // void loadAttribute(int index, int dataindex, int attribIndex, int datasize){
    //   if (!m_are_buffers_initialized) { /* glGenBuffers*/}

    //   glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    //   glBufferData(GL_ARRAY_BUFFER,
    //                m_scene.get_size_of_index(dataindex) * sizeof(float),
    //                m_scene.get_array_of_index(dataindex).data(), GL_STATIC_DRAW);

    //   glVertexAttribPointer(0, datasize, GL_FLOAT, GL_FALSE, datasize * sizeof(float), nullptr);

    // }

    void loadBuffer(int i, int location, int gsEnum, int dataCount){ 
      glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

      glBufferData(GL_ARRAY_BUFFER,
                   m_scene.get_size_of_index(gsEnum),
                   m_scene.get_array_of_index(gsEnum).data(), GL_STATIC_DRAW);

      glVertexAttribPointer(location, dataCount, GL_FLOAT, GL_FALSE, dataCount * sizeof(float), nullptr);
    }

    void initialiseBuffers()
    {
      // TODO eviter de tout recharger à chaque update()

      glGenBuffers(NB_GL_BUFFERS, buffers);
      glGenVertexArrays(NB_VAO_BUFFERS, m_vao); 

      // GLuint vbo_normal, vbo_color;
      // glGenBuffers(1, &vbo_normal);
      // glGenBuffers(1, &vbo_color);


      unsigned int bufn = 0;

      // Load mono facess

      glBindVertexArray(m_vao[VAO_MONO_FACES]); 

      loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_FACES, 3);
      loadBuffer(bufn++, 1, Graphics_scene::FLAT_NORMAL_MONO_FACES, 3);
  
      glEnableVertexAttribArray(0); // vertex
      glEnableVertexAttribArray(1); // normals 

      // Load color faces

      glBindVertexArray(m_vao[VAO_COLORED_FACES]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_FACES, 3);
      loadBuffer(bufn++, 1, Graphics_scene::FLAT_NORMAL_COLORED_FACES, 3);
      loadBuffer(bufn++, 2, Graphics_scene::COLOR_FACES, 3);

      glEnableVertexAttribArray(0); // vertex
      glEnableVertexAttribArray(1); // normals 
      glEnableVertexAttribArray(2); // colors

      m_are_buffers_initialized = true;
    }

    // Uniforms mvp
    void attribBuffers(){
      glUseProgram(m_render_p1);
      
      int mvp_location = glGetUniformLocation(m_render_p1, "mvp_matrix");      
      int mv_location = glGetUniformLocation(m_render_p1, "mv_matrix");
      int psize_location = glGetUniformLocation(m_render_p1, "point_size");      

      // int m = glGetUniformLocation(m_render_p1, "model");      
      // int v = glGetUniformLocation(m_render_p1, "view");
      // int p = glGetUniformLocation(m_render_p1, "projection");      


      int light_pos_location = glGetUniformLocation(m_render_p1, "light_pos");
      int light_diff_location = glGetUniformLocation(m_render_p1, "light_diff");
      int light_spec_location = glGetUniformLocation(m_render_p1, "light_spec");
      int light_amb_location = glGetUniformLocation(m_render_p1, "light_amb");
      int shininess_location = glGetUniformLocation(m_render_p1, "spec_power");

      int clipPlane_location = glGetUniformLocation(m_render_p1, "clipPlane");
      int pointPlane_location = glGetUniformLocation(m_render_p1, "pointPlane");

      int rendering_mode_location = glGetUniformLocation(m_render_p1, "rendering_mode");
      int rendering_transparency_location = glGetUniformLocation(m_render_p1, "rendering_transparency");
      
      /**
       * 
       * uniform highp vec4 light_pos;
      uniform highp vec4 light_diff;
      uniform highp vec4 light_spec;
      uniform highp vec4 light_amb;
      uniform highp float spec_power;

      uniform highp vec4 clipPlane;
      uniform highp vec4 pointPlane;
      uniform highp float rendering_mode; 
      uniform highp float rendering_transparency;
      */

      // vertex uniforms
      glm::mat4 model = glm::mat4(1.0f);


      glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
      float radius = 10.0f;

      float camX = static_cast<float>(sin(glfwGetTime()) * radius);
      float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

      view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);

      glm::mat4 mvp_matrix = projection * view; 
      
      // glUniformMatrix4fv(m, 1, GL_FALSE, &model[0][0]);
      // glUniformMatrix4fv(v, 1, GL_FALSE, &view[0][0]);
      // glUniformMatrix4fv(p, 1, GL_FALSE, &projection[0][0]);

      // fragment uniforms
      glm::vec4 light_position = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      glm::vec4 diffuse = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
      glm::vec4 specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      GLfloat shininess = 1.0f;

      enum { // rendering mode
        DRAW_ALL = -1, // draw all
        DRAW_INSIDE_ONLY, // draw only the part inside the clipping plane
        DRAW_OUTSIDE_ONLY // draw only the part outside the clipping plane
      };

      // glm::mat4 clipping_mMatrix = glm::mat4(1.0f);
      // for(int i=0; i< 16 ; i++) { clipping_mMatrix.data()[i] =  m_frame_plane->matrix()[i]; }
      glm::vec4 clipPlane = glm::vec4(0.0, 0.0, 1.0, 0.0);
      glm::vec4 pointPlane = glm::vec4(0,0,100,1);
      float rendering_mode = GLfloat(DRAW_ALL);
      float rendering_transparency = 0.5f;

      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp_matrix[0][0]);
      glUniformMatrix4fv(mv_location, 1, GL_FALSE, &view[0][0]);
      glUniform1f(psize_location, GLfloat(m_size_points));

      glUniform4fv(light_pos_location, 1, glm::value_ptr(light_position));
      glUniform4fv(light_diff_location, 1, glm::value_ptr(diffuse));
      glUniform4fv(light_spec_location, 1, glm::value_ptr(specular));
      glUniform4fv(light_amb_location, 1, glm::value_ptr(m_ambient_color));
      glUniform1f(shininess_location, shininess);
      glUniform4fv(clipPlane_location, 1, glm::value_ptr(clipPlane));
      glUniform4fv(pointPlane_location, 1, glm::value_ptr(pointPlane));
      glUniform1f(rendering_transparency_location,rendering_transparency);
      glUniform1f(rendering_mode_location,rendering_mode);
    }

    void renderScene(float time)
    {
      if(!m_are_buffers_initialized) { initialiseBuffers(); }
      
      
      if(m_draw_faces){ 

        // glm::mat4 trans = glm::mat4(1.0f);
        // trans = glm::rotate(trans, time, glm::vec3(0.0, 0.0, 1.0));

        // modelView = trans;

        // modelViewProjection = 
        //   glm::perspective(glm::radians(45.f), ((float)windowWidth)/windowHeight, 0.1f, 100.0f)
        //   *
        //   modelView;
        attribBuffers();
        glBindVertexArray(m_vao[VAO_MONO_FACES]);
        glUseProgram(m_render_p1);
        glVertexAttrib4fv(2, glm::value_ptr(m_ambient_color));
        glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_FACES));
      
        glBindVertexArray(m_vao[VAO_COLORED_FACES]);
        glUseProgram(m_render_p1);
        glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_FACES));
      }
    }

  private:
    GLFWwindow *m_window;
    const Graphics_scene &m_scene;
    const char *m_title;
    bool m_draw_vertices;
    bool m_draw_edges;
    bool m_draw_rays;
    bool m_draw_lines;
    bool m_draw_faces;
    bool m_are_buffers_initialized;
    float m_size_points;
    glm::vec4 m_faces_mono_color;
    glm::vec4 m_ambient_color;
    glm::mat4 modelView;
    glm::mat4 modelViewProjection;

    enum
    { VAO_MONO_POINTS=0,
      VAO_COLORED_POINTS,
      VAO_MONO_SEGMENTS,
      VAO_COLORED_SEGMENTS,
      VAO_MONO_RAYS,
      VAO_COLORED_RAYS,
      VAO_MONO_LINES,
      VAO_COLORED_LINES,
      VAO_MONO_FACES,
      VAO_COLORED_FACES,
      VAO_CLIPPING_PLANE,
      NB_VAO_BUFFERS
    };
    GLuint m_vao[NB_VAO_BUFFERS];

    unsigned int m_render_p1;

    static const unsigned int NB_GL_BUFFERS=(Graphics_scene::END_POS-Graphics_scene::BEGIN_POS)+
      (Graphics_scene::END_COLOR-Graphics_scene::BEGIN_COLOR)+3; // +2 for normals (mono and color), +1 for clipping plane

    GLuint buffers[NB_GL_BUFFERS]; // +1 for the vbo buffer of clipping plane
  };

  // Blocking call
  inline void draw_graphics_scene(const Graphics_scene &graphics_scene,
                                  const char *title = "CGAL Basic Viewer")
  {
    Basic_Viewer(graphics_scene, title).show();
  }
} // End OpenGL Namespace
} // End CGAL Namespace