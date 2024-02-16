#include "Basic_viewer.h"
namespace CGAL {
namespace OpenGL{

  void glfwErrorCallback(int error, const char *description)
  {
    fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
  }

  GLFWwindow* initialise(const char *title)
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

  Basic_Viewer::Basic_Viewer(const Graphics_scene &graphics_scene,
                  const char *title,
                  bool draw_vertices,
                  bool draw_edges,
                  bool draw_faces,
                  bool use_mono_color,
                  bool inverse_normal,
                  bool draw_rays,
                  bool draw_text,
                  bool draw_lines) : 
    m_scene(graphics_scene),
    m_title(title),
    m_draw_vertices(draw_vertices),
    m_draw_edges(draw_edges),
    m_draw_rays(draw_rays),
    m_draw_lines(draw_lines),
    m_draw_faces(draw_faces),
    m_draw_text(draw_text),
    m_use_mono_color(use_mono_color),
    m_inverse_normal(inverse_normal),
    m_flat_shading(true),
    
    m_size_points(2.),
    m_size_edges(2.),
    m_size_rays(2.),
    m_size_lines(2.),

    m_faces_mono_color(0.24f, 0.24f, 0.78f, 1.0f),
    m_vertices_mono_color(0.78f, 0.24f, 0.24f, 1.0f),
    m_edges_mono_color(0.0f, 0.0f, 0.0f, 1.0f),
    m_rays_mono_color(0.0f, 0.0f, 0.0f, 1.0f),
    m_lines_mono_color(0.0f, 0.0f, 0.0f, 1.0f),

    m_light_position(0, 0, 0, 0),
    m_ambient(0.6f, 0.5f, 0.5f, 1.f),
    m_shininess(0.5f),
    m_diffuse(0.9f, 0.9f, 0.9f, 1.0f),
    m_specular(0.0f, 0.0f, 0.0f, 1.0f),

    point_plane(0, 0, 0, 1),
    clip_plane(0, 0, 1, 0),

    cam_speed(0.2f),
    cam_rot_speed(0.05f),

    m_are_buffers_initialized(false),
    m_is_opengl_4_3(false)
    {
      init_keys_actions();
      cam_position = glm::vec3(0,0,-5);
      cam_forward = glm::vec3(0,0,1);

      modelView = glm::lookAt(glm::vec3(0.0f,0.0f,10.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec3(0.0f,1.0f,0.0f));
      cam_perspective = glm::perspective(glm::radians(45.f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);

      modelViewProjection = cam_perspective * modelView;
    }

    void Basic_Viewer::show()
    {
      m_window = initialise(m_title);

      glfwSetWindowUserPointer(m_window, this);
      glfwSetKeyCallback(m_window, key_callback);
      glfwSetCursorPosCallback(m_window, cursor_callback);
      glfwSetMouseButtonCallback(m_window, mouse_btn_callback);

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
        glClearColor(1.0f,1.0f,1.0f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderScene(test);

        glfwSwapBuffers(m_window);
        handle_events();
      }

      glfwTerminate();
    }

  void Basic_Viewer::compileShaders() { 
    const char* face_vert = m_is_opengl_4_3 ? vertex_source_color : vertex_source_color_comp;
    const char* face_frag = m_is_opengl_4_3 ? fragment_source_color : fragment_source_color_comp;
    const char* pl_vert = m_is_opengl_4_3 ? vertex_source_p_l : vertex_source_p_l_comp;
    const char* pl_frag = m_is_opengl_4_3 ? fragment_source_p_l : fragment_source_p_l_comp;
    const char* plane_vert = vertex_source_clipping_plane;
    const char* plane_frag = fragment_source_clipping_plane;


    face_shader = Shader::loadShader(face_vert, face_frag, "FACE");
    pl_shader = Shader::loadShader(pl_vert, pl_frag, "PL");
    plane_shader = Shader::loadShader(plane_vert, plane_frag, "PLANE");
  }

  void Basic_Viewer::loadBuffer(int i, int location, int gsEnum, int dataCount){ 
    glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

    glBufferData(GL_ARRAY_BUFFER,
                  m_scene.get_size_of_index(gsEnum),
                  m_scene.get_array_of_index(gsEnum).data(), GL_STATIC_DRAW);

    glVertexAttribPointer(location, dataCount, GL_FLOAT, GL_FALSE, dataCount * sizeof(float), nullptr);

    glEnableVertexAttribArray(location);
  }

  void Basic_Viewer::initialiseBuffers()
  {
    // TODO eviter de tout recharger à chaque update()


    // Déplacer aileurs! 
    glGenBuffers(NB_GL_BUFFERS, buffers);
    glGenVertexArrays(NB_VAO_BUFFERS, m_vao); 

    unsigned int bufn = 0;

    // 1) POINT SHADER

    // 1.1) Mono points
    pl_shader.use();
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
    face_shader.use();
    glBindVertexArray(m_vao[VAO_MONO_FACES]);
    loadBuffer(bufn++, 0, Graphics_scene::POS_MONO_FACES, 3);
    if (m_flat_shading) {
      loadBuffer(bufn++, 1, Graphics_scene::FLAT_NORMAL_MONO_FACES, 3);
    } else {
      loadBuffer(bufn++, 1, Graphics_scene::SMOOTH_NORMAL_MONO_FACES, 3);
    }

    // 5.2) Colored faces
    glBindVertexArray(m_vao[VAO_COLORED_FACES]); 
    loadBuffer(bufn++, 0, Graphics_scene::POS_COLORED_FACES, 3);
    if (m_flat_shading) {
      loadBuffer(bufn++, 1, Graphics_scene::FLAT_NORMAL_COLORED_FACES, 3);
    } else {
      loadBuffer(bufn++, 1, Graphics_scene::SMOOTH_NORMAL_COLORED_FACES, 3);
    }
    loadBuffer(bufn++, 2, Graphics_scene::COLOR_FACES, 3);

    // 6) clipping plane shader
    if (m_is_opengl_4_3) {
      generate_clipping_plane();
      plane_shader.use();

      glBindVertexArray(m_vao[VAO_CLIPPING_PLANE]);
      glBindBuffer(GL_ARRAY_BUFFER, buffers[bufn++]);
      glBufferData(GL_ARRAY_BUFFER,
                    m_array_for_clipping_plane.size()*sizeof(float),
                    m_array_for_clipping_plane.data(), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
      glEnableVertexAttribArray(0);
    }

    m_are_buffers_initialized = true;
  }

  void Basic_Viewer::updateUniforms(){
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);

    modelView = cam_rotation_mode == WALK ?
      glm::lookAt(cam_position, cam_position + cam_forward, glm::vec3(0,1,0))
      :
      glm::lookAt(cam_position, cam_look_center, glm::vec3(0,1,0));

    modelViewProjection = projection * modelView;

    // ================================================================

    setFaceUniforms();
    setPLUniforms();
    setClippingUniforms();
  }

  void Basic_Viewer::setFaceUniforms() {
    face_shader.use();

    face_shader.setMatrix4f("mvp_matrix", glm::value_ptr(modelViewProjection));
    face_shader.setMatrix4f("mv_matrix", glm::value_ptr(modelView));
    // face_shader.setFloat("point_size", m_size_points);
    
    face_shader.setVec4f("light_pos", glm::value_ptr(m_light_position));
    face_shader.setVec4f("light_diff", glm::value_ptr(m_diffuse));
    face_shader.setVec4f("light_spec", glm::value_ptr(m_specular));
    face_shader.setVec4f("light_amb", glm::value_ptr(m_ambient));
    face_shader.setFloat("spec_power", m_shininess);    
  }

  void Basic_Viewer::setFaceUniforms(RenderMode rendering_mode) {
    face_shader.use();

    face_shader.setVec4f("clipPlane", glm::value_ptr(clip_plane));
    face_shader.setVec4f("pointPlane", glm::value_ptr(point_plane));
    face_shader.setFloat("rendering_transparency", m_clipping_plane_rendering_transparency);
    face_shader.setFloat("rendering_mode", rendering_mode);
  }

  void Basic_Viewer::setPLUniforms() {
    pl_shader.use();
    
    pl_shader.setMatrix4f("mvp_matrix", glm::value_ptr(modelViewProjection));
  }

  void Basic_Viewer::setPLUniforms(RenderMode rendering_mode, bool set_point_size) {
    pl_shader.use();
    
    pl_shader.setFloat("point_size", m_size_points);
    pl_shader.setVec4f("clipPlane", glm::value_ptr(clip_plane));
    pl_shader.setVec4f("pointPlane", glm::value_ptr(point_plane));
    pl_shader.setFloat("rendering_mode", rendering_mode);
  }

  void Basic_Viewer::setClippingUniforms() {
    glm::vec3 rot_axis = glm::vec3(1, 0, 0);
    if (m_clipping_plane_rotation_axis == Y_CP_AXIS) rot_axis = glm::vec3(0, 1, 0);
    if (m_clipping_plane_rotation_axis == XY_CP_AXIS) rot_axis = glm::vec3(1, 1, 0);
    
    glm::mat4 clipping_mMatrix = glm::mat4(1.0f);
    clipping_mMatrix = glm::rotate(clipping_mMatrix, glm::radians(m_clipping_plane_angle_rot), rot_axis);
    point_plane = clipping_mMatrix * glm::vec4(0, 0, 0, 1);
    clip_plane = clipping_mMatrix * glm::vec4(0, 0, 1, 0);
    plane_shader.use();

    plane_shader.setMatrix4f("vp_matrix", glm::value_ptr(modelViewProjection));
    plane_shader.setMatrix4f("m_matrix", glm::value_ptr(clipping_mMatrix));
  }
  
  void Basic_Viewer::renderScene(float time)
  {
    if(!m_are_buffers_initialized) { initialiseBuffers(); }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_LINE_SMOOTH);
    
    updateUniforms();

    if (m_draw_vertices)  { 
      if (m_use_clipping_plane == CLIPPING_PLANE_SOLID_HALF_ONLY) { 
        setPLUniforms(DRAW_INSIDE_ONLY, true); 
      } else { 
        setPLUniforms(DRAW_ALL, true); 
      }
      draw_vertices(); 
    }
    if (m_draw_edges) {
       if (m_use_clipping_plane == CLIPPING_PLANE_SOLID_HALF_ONLY) { 
        setPLUniforms(DRAW_INSIDE_ONLY); 
      } else { 
        setPLUniforms(DRAW_ALL); 
      }
      draw_edges(); 
    }
    if (m_draw_faces) { 
      if (m_use_clipping_plane == CLIPPING_PLANE_SOLID_HALF_TRANSPARENT_HALF) {
        // The z-buffer will prevent transparent objects from being displayed behind other transparent objects.
        // Before rendering all transparent objects, disable z-testing first.

        // 1. draw solid first
        setFaceUniforms(DRAW_INSIDE_ONLY);
        draw_faces();

        // 2. draw transparent layer second with back face culling to avoid messy triangles
        glDepthMask(false); //disable z-testing
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        setFaceUniforms(DRAW_OUTSIDE_ONLY);
        draw_faces();

        // 3. draw solid again without culling and blend to make sure the solid mesh is visible
        glDepthMask(true); //enable z-testing
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        setFaceUniforms(DRAW_INSIDE_ONLY);
        draw_faces();

        // 4. render clipping plane here
        render_clipping_plane();
      } else if (m_use_clipping_plane == CLIPPING_PLANE_SOLID_HALF_WIRE_HALF ||
               m_use_clipping_plane == CLIPPING_PLANE_SOLID_HALF_ONLY) 
      {
        // 1. draw solid HALF
        setFaceUniforms(DRAW_INSIDE_ONLY);
        draw_faces();

        // 2. render clipping plane here
        render_clipping_plane();
      } else {
        // 1. draw solid FOR ALL
        setFaceUniforms(DRAW_ALL);
        draw_faces(); 
      }
    }
    if (m_draw_rays)      { draw_rays(); } 
    if (m_draw_lines)     { draw_lines(); }
  }

  void Basic_Viewer::draw_faces(){
    face_shader.use();

    glBindVertexArray(m_vao[VAO_MONO_FACES]);
    glVertexAttrib4fv(2, glm::value_ptr(m_faces_mono_color));
    glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_FACES));
  
    glBindVertexArray(m_vao[VAO_COLORED_FACES]);
    if (m_use_mono_color) {
      glDisableVertexAttribArray(2);
      glVertexAttrib4fv(2, glm::value_ptr(m_faces_mono_color));
    } else {
      glEnableVertexAttribArray(2);
    }
    glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_FACES));
  }

  void Basic_Viewer::draw_rays() {
    pl_shader.use();
    
    glBindVertexArray(m_vao[VAO_MONO_RAYS]);
    glVertexAttrib4fv(1, glm::value_ptr(m_rays_mono_color));

    glLineWidth(m_size_rays);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_RAYS));
  
    glBindVertexArray(m_vao[VAO_COLORED_RAYS]);
    if (m_use_mono_color) {
      glDisableVertexAttribArray(1);
      glVertexAttrib4fv(1, glm::value_ptr(m_rays_mono_color));
    } else {
      glEnableVertexAttribArray(1);
    }
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_RAYS));
  }

  void Basic_Viewer::draw_vertices() {
    pl_shader.use();

    glBindVertexArray(m_vao[VAO_MONO_POINTS]);
    glVertexAttrib4fv(1, glm::value_ptr(m_vertices_mono_color));
    glDrawArrays(GL_POINTS, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_POINTS));
  
    glBindVertexArray(m_vao[VAO_COLORED_POINTS]);
    if (m_use_mono_color) {
      glDisableVertexAttribArray(1);
      glVertexAttrib4fv(1, glm::value_ptr(m_vertices_mono_color));
    } else {
      glEnableVertexAttribArray(1);
    }
    glDrawArrays(GL_POINTS, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_POINTS));

  }

  void Basic_Viewer::draw_lines() {
    pl_shader.use();
    
    glBindVertexArray(m_vao[VAO_MONO_LINES]);
    glVertexAttrib4fv(1, glm::value_ptr(m_lines_mono_color));
    glLineWidth(m_size_lines);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_LINES));
  
    glBindVertexArray(m_vao[VAO_COLORED_LINES]);
    if (m_use_mono_color) {
      glDisableVertexAttribArray(1);
      glVertexAttrib4fv(1, glm::value_ptr(m_lines_mono_color));
    } else {
      glEnableVertexAttribArray(1);
    }
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_LINES));
  }

  void Basic_Viewer::draw_edges() {
    pl_shader.use();
          
    glBindVertexArray(m_vao[VAO_MONO_SEGMENTS]);
    glVertexAttrib4fv(1, glm::value_ptr(m_edges_mono_color));
    glLineWidth(m_size_edges);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_SEGMENTS));
  
    glBindVertexArray(m_vao[VAO_COLORED_SEGMENTS]);
    if (m_use_mono_color) {
      glDisableVertexAttribArray(1);
      glVertexAttrib4fv(1, glm::value_ptr(m_edges_mono_color));
    } else {
      glEnableVertexAttribArray(1);
    }
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_SEGMENTS));
    
  }

  void Basic_Viewer::generate_clipping_plane() {
      size_t size=((m_scene.bounding_box().xmax()-m_scene.bounding_box().xmin()) +
                (m_scene.bounding_box().ymax()-m_scene.bounding_box().ymin()) +
                (m_scene.bounding_box().zmax()-m_scene.bounding_box().zmin()));
      
      const unsigned int nbSubdivisions=30;

      auto& array = m_array_for_clipping_plane;
      array.clear();
      for (unsigned int i=0; i<=nbSubdivisions; ++i)
      {
        const float pos = float(size*(2.0*i/nbSubdivisions-1.0));
        array.push_back(pos);
        array.push_back(-float(size));
        array.push_back(0.f);

        array.push_back(pos);
        array.push_back(float(size));
        array.push_back(0.f);

        array.push_back(-float(size));
        array.push_back(pos);
        array.push_back(0.f);

        array.push_back(float(size));
        array.push_back(pos);
        array.push_back(0.f);
      }
    }

  void Basic_Viewer::render_clipping_plane() {
    if (!m_is_opengl_4_3) return;
      if (!m_clipping_plane_rendering) return;
      plane_shader.use();
      glBindVertexArray(m_vao[VAO_CLIPPING_PLANE]);
      glLineWidth(0.1f);
      glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_array_for_clipping_plane.size()/3));
      glLineWidth(1.f);
  }

  void Basic_Viewer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    Basic_Viewer* viewer = static_cast<Basic_Viewer*>(glfwGetWindowUserPointer(window)); 
    viewer->on_key_event(key, scancode, action, mods);
  }

  
  void Basic_Viewer::cursor_callback(GLFWwindow* window, double xpos, double ypo)
  {
    Basic_Viewer* viewer = static_cast<Basic_Viewer*>(glfwGetWindowUserPointer(window)); 
    viewer->on_cursor_event(xpos, ypo);
  }

  
  void Basic_Viewer::mouse_btn_callback(GLFWwindow* window, int button, int action, int mods)
  {
    Basic_Viewer* viewer = static_cast<Basic_Viewer*>(glfwGetWindowUserPointer(window)); 
    viewer->on_mouse_btn_event(button, action, mods);
  }

  void Basic_Viewer::start_action(ActionEnum action){
    switch (action) {
      case MOUSE_ROTATE:
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouse_old = get_cursor();
        break;
    }
  }

  void Basic_Viewer::action(ActionEnum action){
    switch (action){
      case UP:
        translate(glm::vec3(0, cam_speed, 0));
        break;
      case DOWN:
        translate(glm::vec3(0, -cam_speed, 0));
        break;
      case LEFT:
        translate(glm::vec3(cam_speed, 0, 0));
        break;
      case RIGHT:
        translate(glm::vec3(-cam_speed, 0, 0));
        break;
      case FORWARD:
        translate(glm::vec3(0, 0, cam_speed));
        break;
      case BACKWARDS:
        translate(glm::vec3(0, 0, -cam_speed));
        break;
      case MOUSE_ROTATE: 
        mouse_rotate();
        break;
      case SWITCH_CAM_MODE:
        switch_cam_mode();
        break;
      case SWITCH_CAM_ROTATION:
        switch_rotation_mode();
        break;
      case INC_MOVE_SPEED_D1:
        cam_speed += 0.1f;
        break;
      case DEC_MOVE_SPEED_D1:
        cam_speed -= 0.1f;
        break;
      case INC_MOVE_SPEED_1:
        cam_speed++;
        break;
      case DEC_MOVE_SPEED_1:
        cam_speed--;
        break;
      case INC_ROT_SPEED_D1:
        cam_rot_speed += 0.1f;
        break;
      case DEC_ROT_SPEED_D1:
        cam_rot_speed -= 0.1f;
        break;
      case INC_ROT_SPEED_1:
        cam_rot_speed++;
        break;
      case DEC_ROT_SPEED_1:
        cam_rot_speed--;
        break;
      case CLIPPING_PLANE_DISPLAY:
        m_clipping_plane_rendering = !m_clipping_plane_rendering;
        break;
      case CLIPPING_PLANE_MODE:
        m_use_clipping_plane = (m_use_clipping_plane + 1) % CLIPPING_PLANE_END_INDEX;
        break;
      case VERTICES_DISPLAY:
        m_draw_vertices = !m_draw_vertices;
        break;
      case FACES_DISPLAY:
        m_draw_faces = !m_draw_faces;
        break;
      case EDGES_DISPLAY:
        m_draw_edges = !m_draw_edges;
        break;
      case TEXT_DISPLAY:
        m_draw_text = !m_draw_text;
        break;
      case SHADING_MODE:
        m_flat_shading = !m_flat_shading;
        m_are_buffers_initialized = false;
        break;
      case INVERSE_NORMAL:
        m_inverse_normal = !m_inverse_normal;
        m_scene.reverse_all_normals();
        m_are_buffers_initialized = false;
        break;
      case MONO_COLOR:
        m_use_mono_color = !m_use_mono_color;
        break;
      case INC_EDGES_SIZE: 
        if (m_size_edges < 100)
          m_size_edges++;
        break;
      case DEC_EDGES_SIZE:
        if (m_size_edges>0)
          m_size_edges--; 
        break;
      case INC_POINTS_SIZE: 
        if (m_size_points < 100)
          m_size_points++;
        break;
      case DEC_POINTS_SIZE:
        if (m_size_points>5)
          m_size_points--; 
        break;
      case INC_LIGHT_ALL:
        m_ambient.r += 0.01;
        if (m_ambient.r > 1) m_ambient.r=1; 
        m_ambient.g += 0.01;
        if (m_ambient.g > 1) m_ambient.g=1; 
        m_ambient.b += 0.01;
        if (m_ambient.b > 1) m_ambient.b=1; 
        break;
      case DEC_LIGHT_ALL:
        m_ambient.r -= 0.01;
        if (m_ambient.r < 0) m_ambient.r=0; 
        m_ambient.g-= 0.01;
        if (m_ambient.g < 0) m_ambient.g=0; 
        m_ambient.b-= 0.01;
        if (m_ambient.b < 0) m_ambient.b=0; 
        break;
      case INC_LIGHT_R:
        m_ambient.r+= 0.01;
        if (m_ambient.r > 1) m_ambient.r=1; 
        break;
      case INC_LIGHT_G:
        m_ambient.g+= 0.01;
        if (m_ambient.g > 1) m_ambient.g=1; 
        break;
      case INC_LIGHT_B:
        m_ambient.b+= 0.01;
        if (m_ambient.b > 1) m_ambient.b=1; 
        break;
      case DEC_LIGHT_R:
        m_ambient.r-= 0.01;
        if (m_ambient.r < 0) m_ambient.r=0; 
        break;
      case DEC_LIGHT_G:  
        m_ambient.g-= 0.01;
        if (m_ambient.g < 0) m_ambient.g=0; 
        break;
      case DEC_LIGHT_B:
        m_ambient.b-= 0.01;
        if (m_ambient.b < 0) m_ambient.b=0;
        break;
      case INC_CP_ROT_ANGLE_STEP:
        if (m_clipping_plane_angle_step < 10)
          m_clipping_plane_angle_step += 0.1; 
        break;
      case DEC_CP_ROT_ANGLE_STEP:
        if (m_clipping_plane_angle_step > 0.2)
        m_clipping_plane_angle_step -= 0.1; 
        break;
      case CP_ROTATION_AXIS:
        m_clipping_plane_rotation_axis = (m_clipping_plane_rotation_axis + 1) % 3;
        break;
      case CP_NEGATIVE_ROTATION:
        m_clipping_plane_angle_rot -= m_clipping_plane_angle_step;
        break;
      case CP_POSITIVE_ROTATION:
        m_clipping_plane_angle_rot += m_clipping_plane_angle_step;
        break;
    }
  }

  void Basic_Viewer::end_action(ActionEnum action){
    switch (action) {
      case MOUSE_ROTATE:
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    }
  }

  void Basic_Viewer::init_keys_actions() {
    add_action(GLFW_KEY_UP, GLFW_KEY_LEFT_SHIFT, true, FORWARD);
    add_action(GLFW_KEY_DOWN, GLFW_KEY_LEFT_SHIFT, true, BACKWARDS);

    add_action(GLFW_KEY_UP, true, UP);
    add_action(GLFW_KEY_DOWN, true, DOWN);
    add_action(GLFW_KEY_LEFT, true, LEFT);
    add_action(GLFW_KEY_RIGHT, true, RIGHT);

    add_action(GLFW_KEY_C, GLFW_KEY_LEFT_CONTROL, false, SWITCH_CAM_MODE);
    add_action(GLFW_KEY_V, GLFW_KEY_LEFT_CONTROL, false, SWITCH_CAM_ROTATION);

    add_action(GLFW_KEY_X, false, INC_MOVE_SPEED_1);
    add_action(GLFW_KEY_X, GLFW_KEY_LEFT_CONTROL, false, INC_MOVE_SPEED_D1);
    add_action(GLFW_KEY_X, GLFW_KEY_LEFT_SHIFT, false, DEC_MOVE_SPEED_1);
    add_action(GLFW_KEY_X, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, false, DEC_MOVE_SPEED_D1);

    add_action(GLFW_KEY_R, false, INC_ROT_SPEED_1);
    add_action(GLFW_KEY_R, GLFW_KEY_LEFT_CONTROL, false, INC_ROT_SPEED_D1);
    add_action(GLFW_KEY_R, GLFW_KEY_LEFT_SHIFT, false, DEC_ROT_SPEED_1);
    add_action(GLFW_KEY_R, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, false, DEC_ROT_SPEED_D1);

    add_mouse_action(GLFW_MOUSE_BUTTON_1, true, MOUSE_ROTATE);
  
    add_action(GLFW_KEY_C, false, CLIPPING_PLANE_MODE);
    add_action(GLFW_KEY_C, GLFW_KEY_LEFT_ALT, false, CLIPPING_PLANE_DISPLAY); 

    add_action(GLFW_KEY_F, false, FACES_DISPLAY);
    add_action(GLFW_KEY_V, false, VERTICES_DISPLAY);
    add_action(GLFW_KEY_E, false, EDGES_DISPLAY);
    add_action(GLFW_KEY_T, false, TEXT_DISPLAY);
    
    add_action(GLFW_KEY_S, false, SHADING_MODE);
    add_action(GLFW_KEY_N, false, INVERSE_NORMAL);
    add_action(GLFW_KEY_M, false, MONO_COLOR);

    add_action(GLFW_KEY_H, true, DEC_POINTS_SIZE);
    add_action(GLFW_KEY_J, true, INC_POINTS_SIZE);
    add_action(GLFW_KEY_H, GLFW_KEY_LEFT_CONTROL, true, DEC_EDGES_SIZE);
    add_action(GLFW_KEY_J, GLFW_KEY_LEFT_CONTROL, true, INC_EDGES_SIZE);
  
    add_action(GLFW_KEY_PAGE_UP, true, INC_LIGHT_ALL);
    add_action(GLFW_KEY_PAGE_DOWN, true, DEC_LIGHT_ALL);

    add_action(GLFW_KEY_PAGE_UP, GLFW_KEY_LEFT_SHIFT, true, INC_LIGHT_R);
    add_action(GLFW_KEY_PAGE_DOWN, GLFW_KEY_LEFT_SHIFT, true, DEC_LIGHT_R);
    add_action(GLFW_KEY_PAGE_UP, GLFW_KEY_LEFT_ALT, true, INC_LIGHT_G);
    add_action(GLFW_KEY_PAGE_DOWN, GLFW_KEY_LEFT_ALT, true, DEC_LIGHT_G);
    add_action(GLFW_KEY_PAGE_UP, GLFW_KEY_LEFT_CONTROL, true, INC_LIGHT_B);
    add_action(GLFW_KEY_PAGE_DOWN, GLFW_KEY_LEFT_CONTROL, true, DEC_LIGHT_B);

    add_action(GLFW_KEY_Y, true, CP_NEGATIVE_ROTATION);
    add_action(GLFW_KEY_U, true, CP_POSITIVE_ROTATION);

    add_action(GLFW_KEY_Y, GLFW_KEY_LEFT_CONTROL, true, DEC_CP_ROT_ANGLE_STEP);
    add_action(GLFW_KEY_U, GLFW_KEY_LEFT_CONTROL, true, INC_CP_ROT_ANGLE_STEP);

    add_action(GLFW_KEY_Q, GLFW_KEY_LEFT_CONTROL, false, CP_ROTATION_AXIS);
  }

  void Basic_Viewer::translate(glm::vec3 dir){
    const float delta = 1.0f/60;
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3{0, 1, 0}, cam_forward)); 
    glm::vec3 up = glm::cross(cam_forward, right);

    glm::vec3 result = 
      dir.x * right * delta +
      dir.y * up * delta +
      dir.z * cam_forward * delta;

    cam_position += result;
    cam_look_center += result;
  }

  void Basic_Viewer::mouse_rotate(){
    glm::vec2 cursor_delta {
      get_cursor().x - mouse_old.x, 
      get_cursor().y - mouse_old.y
    };

    mouse_old = get_cursor();
    
    cam_view += cursor_delta * cam_rot_speed;

    std::cout << cam_view.x << " " << cam_view.y << std::endl;

    glm::vec3 dir =  {
      cos(glm::radians(cam_view.x)) * cos(glm::radians(cam_view.y)),
      sin(glm::radians(cam_view.y)),
      sin(glm::radians(cam_view.x)) * cos(glm::radians(cam_view.y))
    };


    if (cam_rotation_mode == WALK){
      dir.y = -dir.y;
      cam_forward = dir;
      return;
    }

    // cam_rotation_mode == CENTER
    glm::vec3 camToCenter = cam_look_center - cam_position;
    cam_forward = normalize(camToCenter);
    cam_position = cam_look_center + dir * length(camToCenter);
  }

  void Basic_Viewer::switch_cam_mode() {
    if (cam_mode == PERSPECTIVE){
      //cam_perspective = glm::ortho();
      return;
    }

    cam_perspective = glm::perspective(glm::radians(45.f),
                                       (float)windowWidth/(float)windowHeight, 0.1f, 100.0f);
  }

  void Basic_Viewer::switch_rotation_mode() {
    if (cam_rotation_mode == CENTER){
      cam_rotation_mode = WALK;
      cam_view.x += 180;

      return;
    }

    cam_view.x -= 180;
    
    glm::vec3 camToCenter = glm::normalize(cam_look_center - cam_position);
    cam_forward = camToCenter;
    cam_rotation_mode = cam_rotation_mode == WALK ? CENTER : WALK;
  }

  // Blocking call
  inline void draw_graphics_scene(const Graphics_scene &graphics_scene, const char *title)
  {
    Basic_Viewer(graphics_scene, title).show();
  }
} // End OpenGL Namespace
} // End CGAL Namespace