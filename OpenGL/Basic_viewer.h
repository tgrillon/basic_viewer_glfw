#include <CGAL/Graphics_scene.h>
#include <CGAL/Basic_shaders.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "Shader.h"

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
                                   "in highp vec4 fColor;"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "    FragColor = fColor;\n"
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
  enum RenderMode{ // rendering mode
        DRAW_ALL = -1, // draw all
        DRAW_INSIDE_ONLY, // draw only the part inside the clipping plane
        DRAW_OUTSIDE_ONLY // draw only the part outside the clipping plane
      };

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
                  bool draw_vertices = true,
                  bool draw_edges = true,
                  bool draw_faces = true,
                  bool draw_rays = true,
                  bool draw_text = true,
                  bool draw_lines = true) : 
    m_scene(graphics_scene),
    m_title(title),
    m_draw_vertices(draw_vertices),
    m_draw_edges(draw_edges),
    m_draw_rays(draw_rays),
    m_draw_lines(draw_lines),
    m_draw_faces(draw_faces),
    m_draw_text(draw_text),
    
    m_size_points(2.),
    m_size_edges(4.),
    m_size_rays(2.),
    m_size_lines(2.),

    m_faces_mono_color(1.f,0,0,1.0f),

    m_light_position(0, 0, 0, 0),
    m_ambient(0.6f, 0.5f, 0.5f, 1.f),
    m_shininess(0.5f),
    m_diffuse(0.9f, 0.9f, 0.9f, 1.0f),
    m_specular(0.0f, 0.0f, 0.0f, 1.0f),

    point_plane(0, 100, 0, 0),
    clip_plane(0, 1, 0, 0),

    m_are_buffers_initialized(false),
    m_is_opengl_4_3(false),
    rendering_mode(DRAW_ALL)
    {
      // modelView = glm::translate(glm::mat4(1.f), glm::vec3(1,0,0));
      modelView = glm::lookAt(glm::vec3(0.0f,0.0f,10.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec3(0.0f,1.0f,0.0f));
  
      modelViewProjection = 
        glm::perspective(glm::radians(45.f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f)
        *
        modelView;
    }

    void show()
    {
    m_window = initialise(m_title);
    glfwSetKeyCallback(m_window, handle_inputs);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major > 4 || major == 4 && minor >= 3){
      m_is_opengl_4_3 = true;
    }


    compileShaders();

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
    void compileShaders()
    { 
      const char* face_vert = m_is_opengl_4_3 ? vertex_source_color : vertex_source_color_comp;
      const char* face_frag = m_is_opengl_4_3 ? fragment_source_color : fragment_source_color_comp;
      const char* pl_vert = m_is_opengl_4_3 ? vertex_source_p_l : vertex_source_p_l_comp;
      const char* pl_frag = m_is_opengl_4_3 ? fragment_source_p_l : fragment_source_p_l_comp;

      face_shader = Shader::loadShader(face_vert, face_frag, "FACE");
      pl_shader = Shader::loadShader(pl_vert, pl_frag, "PL");
      render_plane_shader = Shader::loadShader(vertex_source_clipping_plane, fragment_source_clipping_plane, "PLANE");
    }

    void loadBuffer(int i, int location, int gsEnum, int dataCount){ 
      glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

      glBufferData(GL_ARRAY_BUFFER,
                   m_scene.get_size_of_index(gsEnum),
                   m_scene.get_array_of_index(gsEnum).data(), GL_STATIC_DRAW);

      glVertexAttribPointer(location, dataCount, GL_FLOAT, GL_FALSE, dataCount * sizeof(float), nullptr);

      glEnableVertexAttribArray(location);
    }

    void initialiseBuffers()
    {
      // TODO eviter de tout recharger à chaque update()


      // Déplacer aileurs! 
      glGenBuffers(NB_GL_BUFFERS, buffers);
      glGenVertexArrays(NB_VAO_BUFFERS, m_vao); 

      unsigned int bufn = 0;

      // 1) POINT SHADER

      // 1.1) Mono points
      glBindVertexArray(m_vao[VAO_MONO_POINTS]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_POINTS, 3);

      // 1.2) Color points
      glBindVertexArray(m_vao[VAO_COLORED_POINTS]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_POINTS, 3);      
      loadBuffer(bufn++, 1, Graphics_scene::COLOR_POINTS, 3);      

      // 2) SEGMENT SHADER

      // 2.1) Mono segments
      glBindVertexArray(m_vao[VAO_MONO_SEGMENTS]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_SEGMENTS, 3);      

      // 2.2) Colored segments
      glBindVertexArray(m_vao[VAO_COLORED_SEGMENTS]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_SEGMENTS, 3);      
      loadBuffer(bufn++, 1, Graphics_scene::COLOR_SEGMENTS, 3);   

      // 3) RAYS SHADER

      // 2.1) Mono segments
      glBindVertexArray(m_vao[VAO_MONO_RAYS]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_RAYS, 3);      

      // 2.2) Colored segments
      glBindVertexArray(m_vao[VAO_COLORED_RAYS]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_RAYS, 3);      
      loadBuffer(bufn++, 1, Graphics_scene::COLOR_RAYS, 3);   
    
      // 4) LINES SHADER

      // 2.1) Mono lines
      glBindVertexArray(m_vao[VAO_MONO_LINES]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_LINES, 3);      

      // 2.2) Colored lines
      glBindVertexArray(m_vao[VAO_COLORED_LINES]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_LINES, 3);      
      loadBuffer(bufn++, 1, Graphics_scene::COLOR_LINES, 3);   

      // 5) FACE SHADER

      // 5.1) Mono faces
      glBindVertexArray(m_vao[VAO_MONO_FACES]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_FACES, 3);
      loadBuffer(bufn++, 1, Graphics_scene::FLAT_NORMAL_MONO_FACES, 3);

      // 5.2) Colored faces
      glBindVertexArray(m_vao[VAO_COLORED_FACES]); 
      loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_FACES, 3);
      loadBuffer(bufn++, 1, Graphics_scene::FLAT_NORMAL_COLORED_FACES, 3);
      loadBuffer(bufn++, 2, Graphics_scene::COLOR_FACES, 3);

      // 6) clipping plane shader
      glBindVertexArray(VAO_CLIPPING_PLANE);
      // TODO

      m_are_buffers_initialized = true;
    }

    // Uniforms mvp
    void updateUniforms(){

      // vertex uniforms
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
      float radius = 10.0f;

      float camX = static_cast<float>(sin(glfwGetTime()) * radius);
      float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

      view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);

      modelViewProjection = projection * view;
      modelView = view;

      // ================================================================

      setFaceUniforms();
      setPLUniforms();
      setClippingUniforms();
    }

    void setFaceUniforms() {
      face_shader.use();

      face_shader.setMatrix4f("mvp_matrix", glm::value_ptr(modelViewProjection));
      face_shader.setMatrix4f("mv_matrix", glm::value_ptr(modelView));
      face_shader.setFloat("point_size", m_size_points);
      
      face_shader.setVec4f("light_pos", glm::value_ptr(m_light_position));
      face_shader.setVec4f("light_diff", glm::value_ptr(m_diffuse));
      face_shader.setVec4f("light_spec", glm::value_ptr(m_specular));
      face_shader.setVec4f("light_amb", glm::value_ptr(m_ambient));
      face_shader.setFloat("spec_power", m_shininess);

      face_shader.setVec4f("clipPlane", glm::value_ptr(clip_plane));
      face_shader.setVec4f("pointPlane", glm::value_ptr(point_plane));
      face_shader.setFloat("rendering_transparency", m_rendering_transparency);
      face_shader.setFloat("rendering_mode", rendering_mode);
      
    }
    
    void setPLUniforms() {
      pl_shader.use();
      
      pl_shader.setMatrix4f("mvp_matrix", glm::value_ptr(modelViewProjection));
      pl_shader.setFloat("point_size", m_size_points);
      pl_shader.setVec4f("clipPlane", glm::value_ptr(clip_plane));
      pl_shader.setVec4f("pointPlane", glm::value_ptr(point_plane));
      pl_shader.setFloat("rendering_mode", rendering_mode);
      
    }

    void setClippingUniforms() {
      
    }
    
    void setPLUniforms() {
      glUseProgram(m_render_p_l);
      
      // vertex uniforms 
      int mvp_location = glGetUniformLocation(m_render_p_l, "mvp_matrix");      
      int psize_location = glGetUniformLocation(m_render_p_l, "point_size");  
      
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
      glUniform1f(psize_location, GLfloat(m_size_points));
      
      // frag uniforms 
      int clipPlane_location = glGetUniformLocation(m_render_p_l, "clipPlane");
      int pointPlane_location = glGetUniformLocation(m_render_p_l, "pointPlane");
      int rendering_mode_location = glGetUniformLocation(m_render_p_l, "rendering_mode");
      
      glUniform4fv(clipPlane_location, 1, glm::value_ptr(clip_plane));
      glUniform4fv(pointPlane_location, 1, glm::value_ptr(point_plane));

      glUniform1f(rendering_mode_location, rendering_mode);
    }

    void setClippingUniforms() {
      
    }

    void renderScene(float time)
    {
      if(!m_are_buffers_initialized) { initialiseBuffers(); }
      
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_PROGRAM_POINT_SIZE);
      glEnable(GL_LINE_SMOOTH);
      
      updateUniforms();

      if (m_draw_edges)     { draw_edges(); }
      if (m_draw_faces)     { draw_faces(); }
      if (m_draw_rays)      { draw_rays(); } 
      if (m_draw_vertices)  { draw_vertices(); }
      if (m_draw_lines)     { draw_lines(); }
    }

    void draw_faces(){
      face_shader.use();

      glBindVertexArray(m_vao[VAO_MONO_FACES]);
      glVertexAttrib4fv(2, glm::value_ptr(m_faces_mono_color));

      glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_FACES));
    
      glBindVertexArray(m_vao[VAO_COLORED_FACES]);
      glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_FACES));
    }

    void draw_rays() {
      pl_shader.use();
      
      glBindVertexArray(m_vao[VAO_MONO_RAYS]);
      glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));

      glLineWidth(m_size_rays);
      glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_RAYS));
    
      glBindVertexArray(m_vao[VAO_COLORED_RAYS]);
      glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_RAYS));
    }

    void draw_vertices() {
      pl_shader.use();

      glBindVertexArray(m_vao[VAO_MONO_POINTS]);
      glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));
      glDrawArrays(GL_POINTS, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_POINTS));
    
      glBindVertexArray(m_vao[VAO_COLORED_POINTS]);
      glDrawArrays(GL_POINTS, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_POINTS));

    }

    void draw_lines() {
      pl_shader.use();
      
      glBindVertexArray(m_vao[VAO_MONO_LINES]);
      glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));
      glLineWidth(m_size_lines);
      glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_LINES));
    
      glBindVertexArray(m_vao[VAO_COLORED_LINES]);
      glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_LINES));
    }

    void draw_edges() {
      pl_shader.use();
            
      glBindVertexArray(m_vao[VAO_MONO_SEGMENTS]);
      glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));
      glLineWidth(m_size_edges);
      glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_SEGMENTS));
    
      glBindVertexArray(m_vao[VAO_COLORED_SEGMENTS]);
      glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_SEGMENTS));
      
    }

    void handle_input(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS) {
          switch (key){
            case GLFW_KEY_UP:
              break;
          }
        }
    }

      
  public:


  private:
    GLFWwindow *m_window;
    const Graphics_scene &m_scene;
    const char *m_title;
    bool m_draw_vertices;
    bool m_draw_edges;
    bool m_draw_rays;
    bool m_draw_lines;
    bool m_draw_faces;
    bool m_draw_text;
    bool m_are_buffers_initialized;
    float m_size_points;
    float m_size_edges;
    float m_size_rays;
    float m_size_lines;
    glm::vec4 m_faces_mono_color;
    glm::vec4 clip_plane;
    glm::vec4 point_plane;
    glm::vec4 m_light_position;
    glm::vec4 m_ambient, m_diffuse, m_specular;
    float m_shininess;
    glm::mat4 modelView;
    glm::mat4 modelViewProjection;
    float m_rendering_transparency;
    RenderMode rendering_mode;
    bool m_is_opengl_4_3;

    Shader pl_shader, face_shader, render_plane_shader;
    
    

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