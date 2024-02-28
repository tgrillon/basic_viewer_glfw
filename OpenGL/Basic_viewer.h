#pragma once

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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace CGAL::OpenGL {
  enum RenderMode{ // rendering mode
        DRAW_ALL=-1, // draw all
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
                    bool use_mono_color=false,
                    bool inverse_normal=false,
                    bool draw_rays = true,
                    bool draw_text = true,
                    bool draw_lines = true);    
    
    void show();
    void make_screenshot(const std::string& pngpath);
    
  private:

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_callback(GLFWwindow* window, double xpos, double ypo);
    static void mouse_btn_callback(GLFWwindow* window, int button, int action, int mods);
    static void window_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  
    static GLFWwindow* create_window (int width, int height, const char *title, bool hidden = false);
    static void error_callback (int error, const char *description);

    void compileShaders();
    void loadBuffer(int i, int location, int gsEnum, int dataCount);
    void loadBuffer(int i, int location, const std::vector<float>& vector, int dataCount);
    void initialiseBuffers();

    void updateUniforms();

    void setFaceUniforms();
    void setPLUniforms();
    void setClippingUniforms();

    void renderScene();
    void draw_faces();
    void draw_rays();
    void draw_lines();
    
    void draw_faces_(RenderMode mode);
    void draw_vertices(RenderMode mode);
    void draw_edges(RenderMode mode);

    void generate_clipping_plane();
    void render_clipping_plane();

    void init_keys_actions();

    void start_action(ActionEnum action) override;
    void action_event(ActionEnum action) override;
    void end_action(ActionEnum action) override;
    
    void translate(const glm::vec3 dir);
    void mouse_rotate();
    void mouse_translate();
    void set_cam_mode(CAM_MODE mode);
    void switch_rotation_mode();

    glm::vec2 to_ndc(double, double);
    glm::vec3 mapping_cursor_toHemisphere(double x, double y);
    glm::mat4 get_rotation(glm::vec3 const& start, glm::vec3 const& end);
    void rotate_clipping_plane();

    void translate_clipping_plane();
    void translate_clipping_plane_cam_dir();
    // void translate_clipping_plane_n_dir();

    void switch_axis(int axis);

    void zoom(float z);
    void fullscreen();
    void screenshot(const std::string& pngpath);

    void print_help();
    
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
    bool m_flat_shading;
    bool m_use_mono_color;
    bool m_inverse_normal;

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
    
    // CLIPPING PLANE 
    enum { // clipping mode
        CLIPPING_PLANE_OFF=0,
        CLIPPING_PLANE_SOLID_HALF_TRANSPARENT_HALF,
        CLIPPING_PLANE_SOLID_HALF_WIRE_HALF,
        CLIPPING_PLANE_SOLID_HALF_ONLY,
        CLIPPING_PLANE_END_INDEX
      };

    int m_use_clipping_plane=CLIPPING_PLANE_OFF;
    std::vector<float> m_array_for_clipping_plane;

    bool m_clipping_plane_rendering = true; // will be toggled when alt+c is pressed, which is used for indicating whether or not to render the clipping plane ;
    float m_clipping_plane_rendering_transparency = 0.5f; // to what extent the transparent part should be rendered;
    
    /******* CAMERA ******/  
    Cursor mouse_old;
    
    float cam_speed, cam_rot_speed;

    glm::mat4 cam_projection;
    glm::vec3 cam_position;
    glm::vec2 cam_view = {0, 0};
    glm::vec3 cam_forward = {}, cam_look_center = {};
    float cam_orth_zoom = 1.0f;

    glm::ivec2 window_size {500, 450};
    glm::ivec2 old_window_size;
    glm::ivec2 old_window_pos;

    bool is_fullscreen = false;
    CAM_MODE cam_mode = PERSPECTIVE;
    CAM_ROTATION_MODE cam_rotation_mode = CENTER;

    /***************CLIPPING PLANE****************/

    int m_cstr_axis_enum = NO_AXIS;
    glm::vec3 m_cstr_axis = {1., 0., 0.};

    glm::mat4 clipping_mMatrix = glm::mat4(1.0);

    enum {
      NO_AXIS=0,
      X_AXIS, 
      Y_AXIS, 
      Z_AXIS,
      NB_AXIS_ENUM
    };

    enum Actions {
      MOUSE_ROTATE, MOUSE_TRANSLATE,
      UP, LEFT, RIGHT, DOWN, FORWARD, BACKWARDS, 
      SWITCH_CAM_MODE, SWITCH_CAM_ROTATION,
      FULLSCREEN, SCREENSHOT,
      INC_ZOOM, DEC_ZOOM,
      INC_MOVE_SPEED_D1, INC_MOVE_SPEED_1,
      DEC_MOVE_SPEED_D1, DEC_MOVE_SPEED_1,
      INC_ROT_SPEED_D1, INC_ROT_SPEED_1,
      DEC_ROT_SPEED_D1, DEC_ROT_SPEED_1,
      
      CLIPPING_PLANE_MODE, CLIPPING_PLANE_DISPLAY,
      VERTICES_DISPLAY, FACES_DISPLAY, EDGES_DISPLAY, TEXT_DISPLAY,
      INVERSE_NORMAL, SHADING_MODE, MONO_COLOR,
      INC_LIGHT_ALL, INC_LIGHT_R, INC_LIGHT_G, INC_LIGHT_B,
      DEC_LIGHT_ALL, DEC_LIGHT_R, DEC_LIGHT_G, DEC_LIGHT_B,
      INC_POINTS_SIZE, DEC_POINTS_SIZE,
      INC_EDGES_SIZE, DEC_EDGES_SIZE,

      CP_ROTATION, CP_TRANSLATION, 
      CP_TRANS_CAM_DIR, CP_TRANS_N_DIR, 
      CONSTRAINT_AXIS, 

      EXIT
    };
    
    /*********************/

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




    GLuint m_vao[NB_VAO_BUFFERS];

    static const unsigned int NB_GL_BUFFERS=(Graphics_scene::END_POS-Graphics_scene::BEGIN_POS)+
      (Graphics_scene::END_COLOR-Graphics_scene::BEGIN_COLOR)+3; // +2 for normals (mono and color), +1 for clipping plane

    GLuint buffers[NB_GL_BUFFERS]; // +1 for the vbo buffer of clipping plane
  };
}
