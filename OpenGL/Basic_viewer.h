#include <CGAL/Graphics_scene.h>
#include <CGAL/Basic_shaders.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "Shader.h"
#include "Input.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace CGAL {
namespace OpenGL{
  enum RenderMode{ // rendering mode
        DRAW_ALL=-1, // draw all
        DRAW_INSIDE_ONLY, // draw only the part inside the clipping plane
        DRAW_OUTSIDE_ONLY // draw only the part outside the clipping plane
      };
  
  const int windowWidth = 1024;
  const int windowHeight = 768;
  const int windowSamples = 4;

  void glfwErrorCallback(int error, const char *description);
  GLFWwindow *initialise(const char *title);
  inline void draw_graphics_scene(const Graphics_scene &graphics_scene,
                                    const char *title = "CGAL Basic Viewer");

  class Basic_Viewer : public Input {
  public:
    Basic_Viewer(const Graphics_scene &graphics_scene,
                    const char *title,
                    bool draw_vertices = true,
                    bool draw_edges = true,
                    bool draw_faces = true,
                    bool draw_rays = true,
                    bool draw_text = true,
                    bool draw_lines = true);
    static void aggregate_inputs(GLFWwindow* window, int key, int scancode, int action, int mods);
    void show();
    
  private:
    void compileShaders();
    void loadBuffer(int i, int location, int gsEnum, int dataCount);
    void initialiseBuffers();

    void updateUniforms();

    void setFaceUniforms();
    void setPLUniforms();
    void setClippingUniforms();

    void setFaceUniforms(RenderMode rendering_mode);
    void setPLUniforms(RenderMode rendering_mode, bool set_point_size=false);

    void renderScene(float time);
    void draw_faces();
    void draw_rays();
    void draw_vertices();
    void draw_edges();
    void draw_lines();

    void generate_clipping_plane();
    void render_clipping_plane();

    void init_keys_actions();
    void handle_actions(ActionEnum action) override;
    
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
    glm::vec4 m_vertices_mono_color;
    glm::vec4 m_edges_mono_color;
    glm::vec4 m_rays_mono_color;
    glm::vec4 m_lines_mono_color;

    glm::vec4 clip_plane;
    glm::vec4 point_plane;
    glm::vec4 m_light_position;
    glm::vec4 m_ambient, m_diffuse, m_specular;
    float m_shininess;
    glm::mat4 modelView;
    glm::mat4 modelViewProjection;
    bool m_is_opengl_4_3;

    Shader pl_shader, face_shader, plane_shader;
    
    glm::mat4 cam_position, cam_rotation;


    enum { // clipping mode
        CLIPPING_PLANE_OFF=0,
        CLIPPING_PLANE_SOLID_HALF_TRANSPARENT_HALF,
        CLIPPING_PLANE_SOLID_HALF_WIRE_HALF,
        CLIPPING_PLANE_SOLID_HALF_ONLY,
        CLIPPING_PLANE_END_INDEX
      };

    int m_use_clipping_plane=CLIPPING_PLANE_OFF;
    std::vector<float> m_array_for_clipping_plane;

    // variables for clipping plane
    bool m_clipping_plane_rendering = true; // will be toggled when alt+c is pressed, which is used for indicating whether or not to render the clipping plane ;
    float m_clipping_plane_rendering_transparency = 0.5f; // to what extent the transparent part should be rendered;
    
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

    enum Actions {
      UP, LEFT, RIGHT, DOWN, FORWARD, BACKWARDS, CLIPPLANE
    };
    GLuint m_vao[NB_VAO_BUFFERS];

    static const unsigned int NB_GL_BUFFERS=(Graphics_scene::END_POS-Graphics_scene::BEGIN_POS)+
      (Graphics_scene::END_COLOR-Graphics_scene::BEGIN_COLOR)+3; // +2 for normals (mono and color), +1 for clipping plane

    GLuint buffers[NB_GL_BUFFERS]; // +1 for the vbo buffer of clipping plane
  };
}}
