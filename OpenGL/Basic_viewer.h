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
        DRAW_ALL = -1, // draw all
        DRAW_INSIDE_ONLY, // draw only the part inside the clipping plane
        DRAW_OUTSIDE_ONLY // draw only the part outside the clipping plane
      };
  
  enum CAM_MODE { PERSPECTIVE, ORTHOGRAPHIC };
  enum CAM_ROTATION_MODE { CENTER, WALK };

  const int windowSamples = 4;

  void glfwErrorCallback(int error, const char *description);
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
    void show();
    
  private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_callback(GLFWwindow* window, double xpos, double ypo);
    static void mouse_btn_callback(GLFWwindow* window, int button, int action, int mods);
    static void window_size_callback(GLFWwindow* window, int width, int height);
  
    static GLFWwindow* create_window (int width, int height, const char *title);
    static void error_callback (int error, const char *description);

    void compileShaders();
    void loadBuffer(int i, int location, int gsEnum, int dataCount);
    void initialiseBuffers();

    void updateUniforms();
    void setFaceUniforms();
    void setPLUniforms();
    void setClippingUniforms();

    void renderScene(float time);
    void draw_faces();
    void draw_rays();
    void draw_vertices();
    void draw_edges();
    void draw_lines();

    void init_keys_actions();

    void start_action(ActionEnum action) override;
    void action(ActionEnum action) override;
    void end_action(ActionEnum action) override;
    
    void translate(const glm::vec3 dir);
    void mouse_rotate();
    void set_cam_mode(CAM_MODE mode);
    void switch_rotation_mode();
    void zoom(float z);

    void fullscreen();
    
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
    
    Cursor mouse_old;

    float cam_speed, cam_rot_speed;

    glm::mat4 cam_projection;
    glm::vec3 cam_position;
    glm::vec2 cam_view = {0, 0};
    glm::vec3 cam_forward = {}, cam_look_center = {};
    float cam_orth_zoom = 1.0f;

    glm::ivec2 window_size {1024, 768};
    glm::ivec2 old_window_size;
    glm::ivec2 old_window_pos;

    bool is_fullscreen = false;
    CAM_MODE cam_mode = PERSPECTIVE;
    CAM_ROTATION_MODE cam_rotation_mode = CENTER;
    
    enum VAO_TYPES
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
      UP, LEFT, RIGHT, DOWN, FORWARD, BACKWARDS, MOUSE_ROTATE,
      SWITCH_CAM_MODE, SWITCH_CAM_ROTATION,
      FULLSCREEN,
      INC_ZOOM, DEC_ZOOM,
      INC_MOVE_SPEED_D1, INC_MOVE_SPEED_1,
      DEC_MOVE_SPEED_D1, DEC_MOVE_SPEED_1,
      INC_ROT_SPEED_D1, INC_ROT_SPEED_1,
      DEC_ROT_SPEED_D1, DEC_ROT_SPEED_1,
    };


    GLuint m_vao[NB_VAO_BUFFERS];

    static const unsigned int NB_GL_BUFFERS=(Graphics_scene::END_POS-Graphics_scene::BEGIN_POS)+
      (Graphics_scene::END_COLOR-Graphics_scene::BEGIN_COLOR)+3; // +2 for normals (mono and color), +1 for clipping plane

    GLuint buffers[NB_GL_BUFFERS]; // +1 for the vbo buffer of clipping plane
  };
}}
