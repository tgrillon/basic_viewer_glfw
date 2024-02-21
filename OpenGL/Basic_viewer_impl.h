#include "Basic_viewer.h"
namespace CGAL {
namespace OpenGL{

  void Basic_Viewer::error_callback(int error, const char *description)
  {
    //fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
  }

  GLFWwindow* Basic_Viewer::create_window(int width, int height, const char *title)
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
    glfwSetErrorCallback(error_callback);

    // Set additional window options
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, windowSamples); // MSAA

      // Create window using GLFW
    GLFWwindow *window = glfwCreateWindow(width,
                                          height,
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
    
    m_size_points(20.),
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

    cam_speed(0.2f),
    cam_rot_speed(0.05f),

    m_are_buffers_initialized(false),
    m_is_opengl_4_3(false),
    rendering_mode(DRAW_ALL)
    {
      init_keys_actions();
      cam_position = glm::vec3(0,0,-5);
      cam_forward = glm::vec3(0,0,1);
    }

    void Basic_Viewer::show()
    {
      m_window = create_window(window_size.x, window_size.y, m_title);
      
      glfwSetWindowUserPointer(m_window, this);
      glfwSetKeyCallback(m_window, key_callback);
      glfwSetCursorPosCallback(m_window, cursor_callback);
      glfwSetMouseButtonCallback(m_window, mouse_btn_callback);
      glfwSetFramebufferSizeCallback(m_window, window_size_callback);

      print_help();
      set_cam_mode(cam_mode);

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
        handle_events();
      }

      glfwTerminate();
    }

  void Basic_Viewer::compileShaders() { 
    const char* face_vert = m_is_opengl_4_3 ? vertex_source_color : vertex_source_color_comp;
    const char* face_frag = m_is_opengl_4_3 ? fragment_source_color : fragment_source_color_comp;
    const char* pl_vert = m_is_opengl_4_3 ? vertex_source_p_l : vertex_source_p_l_comp;
    const char* pl_frag = m_is_opengl_4_3 ? fragment_source_p_l : fragment_source_p_l_comp;

    face_shader = Shader::loadShader(face_vert, face_frag, "FACE");
    pl_shader = Shader::loadShader(pl_vert, pl_frag, "PL");
    render_plane_shader = Shader::loadShader(vertex_source_clipping_plane, fragment_source_clipping_plane, "PLANE");
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

  void Basic_Viewer::updateUniforms(){
    modelView = cam_rotation_mode == WALK ?
      glm::lookAt(cam_position, cam_position + cam_forward, glm::vec3(0,1,0))
      :
      glm::lookAt(cam_position, cam_look_center, glm::vec3(0,1,0));

    modelViewProjection = cam_projection * modelView;

    // ================================================================

    setFaceUniforms();
    setPLUniforms();
    setClippingUniforms();
  }

  void Basic_Viewer::setFaceUniforms() {
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

  void Basic_Viewer::setPLUniforms() {
    pl_shader.use();
    
    pl_shader.setMatrix4f("mvp_matrix", glm::value_ptr(modelViewProjection));
    pl_shader.setFloat("point_size", m_size_points);
    pl_shader.setVec4f("clipPlane", glm::value_ptr(clip_plane));
    pl_shader.setVec4f("pointPlane", glm::value_ptr(point_plane));
    pl_shader.setFloat("rendering_mode", rendering_mode);
    
  }

  void Basic_Viewer::setClippingUniforms() {
    
  }
  
  void Basic_Viewer::renderScene(float time)
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

  void Basic_Viewer::draw_faces(){
    face_shader.use();

    glBindVertexArray(m_vao[VAO_MONO_FACES]);
    glVertexAttrib4fv(2, glm::value_ptr(m_faces_mono_color));

    glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_FACES));
  
    glBindVertexArray(m_vao[VAO_COLORED_FACES]);
    glDrawArrays(GL_TRIANGLES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_FACES));
  }

  void Basic_Viewer::draw_rays() {
    pl_shader.use();
    
    glBindVertexArray(m_vao[VAO_MONO_RAYS]);
    glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));

    glLineWidth(m_size_rays);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_RAYS));
  
    glBindVertexArray(m_vao[VAO_COLORED_RAYS]);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_RAYS));
  }

  void Basic_Viewer::draw_vertices() {
    pl_shader.use();

    glBindVertexArray(m_vao[VAO_MONO_POINTS]);
    glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));
    glDrawArrays(GL_POINTS, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_POINTS));
  
    glBindVertexArray(m_vao[VAO_COLORED_POINTS]);
    glDrawArrays(GL_POINTS, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_POINTS));

  }

  void Basic_Viewer::draw_lines() {
    pl_shader.use();
    
    glBindVertexArray(m_vao[VAO_MONO_LINES]);
    glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));
    glLineWidth(m_size_lines);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_LINES));
  
    glBindVertexArray(m_vao[VAO_COLORED_LINES]);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_LINES));
  }

  void Basic_Viewer::draw_edges() {
    pl_shader.use();
          
    glBindVertexArray(m_vao[VAO_MONO_SEGMENTS]);
    glVertexAttrib4fv(1, glm::value_ptr(m_faces_mono_color));
    glLineWidth(m_size_edges);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_MONO_SEGMENTS));
  
    glBindVertexArray(m_vao[VAO_COLORED_SEGMENTS]);
    glDrawArrays(GL_LINES, 0, m_scene.number_of_elements(Graphics_scene::POS_COLORED_SEGMENTS));
    
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

  void Basic_Viewer::window_size_callback(GLFWwindow* window, int width, int height) {
    Basic_Viewer* viewer = static_cast<Basic_Viewer*>(glfwGetWindowUserPointer(window)); 

    viewer->window_size = {width, height};
    viewer->set_cam_mode(viewer->cam_mode);

    glViewport(0, 0, width, height);
  }

  void Basic_Viewer::start_action(ActionEnum action){
    switch (action) {
      case MOUSE_TRANSLATE:
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
      case MOUSE_TRANSLATE:
        mouse_translate();
        break;
      case SWITCH_CAM_MODE:
        set_cam_mode(cam_mode == PERSPECTIVE ? ORTHOGRAPHIC : PERSPECTIVE);
        break;
      case SWITCH_CAM_ROTATION:
        switch_rotation_mode();
        break;
      case FULLSCREEN:
        fullscreen();
        break;
      case INC_ZOOM:
        zoom(1.0f);
        break;
      case DEC_ZOOM:
        zoom(-1.0f);
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
    }
  }

  void Basic_Viewer::end_action(ActionEnum action){
    switch (action) {
      case MOUSE_TRANSLATE:
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

    add_action(GLFW_KEY_C, false, SWITCH_CAM_MODE);
    add_action(GLFW_KEY_V, false, SWITCH_CAM_ROTATION);

    add_action(GLFW_KEY_F, false, FULLSCREEN);

    add_action(GLFW_KEY_Z, false, INC_ZOOM);
    add_action(GLFW_KEY_Z, GLFW_KEY_LEFT_SHIFT, false, DEC_ZOOM);

    add_action(GLFW_KEY_S, false, INC_MOVE_SPEED_1);
    add_action(GLFW_KEY_S, GLFW_KEY_LEFT_CONTROL, false, INC_MOVE_SPEED_D1);
    add_action(GLFW_KEY_S, GLFW_KEY_LEFT_SHIFT, false, DEC_MOVE_SPEED_1);
    add_action(GLFW_KEY_S, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, false, DEC_MOVE_SPEED_D1);

    add_action(GLFW_KEY_R, false, INC_ROT_SPEED_1);
    add_action(GLFW_KEY_R, GLFW_KEY_LEFT_CONTROL, false, INC_ROT_SPEED_D1);
    add_action(GLFW_KEY_R, GLFW_KEY_LEFT_SHIFT, false, DEC_ROT_SPEED_1);
    add_action(GLFW_KEY_R, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, false, DEC_ROT_SPEED_D1);

    add_mouse_action(GLFW_MOUSE_BUTTON_1, true, MOUSE_ROTATE);
    add_mouse_action(GLFW_MOUSE_BUTTON_2, true, MOUSE_TRANSLATE);

    set_action_description(FORWARD, "Move forward");
    set_action_description(BACKWARDS, "Move backwards");
    
    set_action_description(UP, "Move right");
    set_action_description(RIGHT, "Move right");
    set_action_description(LEFT, "Move left");
    set_action_description(DOWN, "Move down");


    set_action_description(SWITCH_CAM_MODE, "Switch to Perspective/Orthographic view");
    set_action_description(SWITCH_CAM_ROTATION, "Switch to default/first person mode");

    set_action_description(FULLSCREEN, "Switch to windowed/fullscreen mode");
    
    set_action_description(MOUSE_ROTATE, "Rotate the view");
    set_action_description(MOUSE_TRANSLATE, "Move the view");

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

  void Basic_Viewer::set_cam_mode(CAM_MODE mode) {
    cam_mode = mode;

    float ratio = (float)window_size.x/(float)window_size.y;

    if (cam_mode == PERSPECTIVE){
      cam_projection = glm::perspective(glm::radians(45.f), (float)window_size.x/(float)window_size.y, 0.1f, 1000.0f);
      return;
    }
    cam_projection = glm::ortho(0.0f, cam_orth_zoom * ratio, 0.0f, cam_orth_zoom, 0.1f, 100.0f);
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

  void Basic_Viewer::fullscreen(){
    is_fullscreen = !is_fullscreen;

    if (is_fullscreen) {
      int count;
      old_window_size = window_size;

      GLFWmonitor* monitor = glfwGetMonitors(&count)[0];
      const GLFWvidmode* mode = glfwGetVideoMode(monitor);

      glfwGetWindowPos(m_window, &old_window_pos.x, &old_window_pos.y); 
      glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
      glViewport(0, 0, mode->width, mode->height);

      std::cout << window_size.x << " " << window_size.y;
      return;
    }

    window_size = old_window_size;
    glfwSetWindowMonitor(m_window, nullptr, old_window_pos.x, old_window_pos.y, window_size.x, window_size.y, 60);
    glViewport(0, 0, window_size.x, window_size.y);

  }

  void Basic_Viewer::mouse_translate(){
    glm::vec3 cursor_delta {
      get_cursor().x - mouse_old.x, 
      get_cursor().y - mouse_old.y,
      0
    };

    mouse_old = get_cursor();

    if (cursor_delta.x == 0 && cursor_delta.y == 0)
      return;
    
    translate(glm::normalize(cursor_delta) * cam_speed);
  }

  void Basic_Viewer::print_help(){
    std::unordered_map<Input::ActionEnum, std::vector<KeyData>> action_keys = get_action_keys();

    ActionEnum sorted_camera_actions[] = {
      MOUSE_ROTATE, MOUSE_TRANSLATE, UP, DOWN, LEFT, RIGHT, FORWARD, BACKWARDS, FULLSCREEN,
      SWITCH_CAM_MODE, SWITCH_CAM_ROTATION
    };

    std::cout << std::endl << "Help for Basic Viewer OpenGl :" << std::endl;

    auto print_action = [this](std::vector<KeyData> keys, ActionEnum action){
      std::string line;

      std::string action_str = get_action_description(action);

      if (action_str.empty()) action_str = "No description found";
        
      line += "   " + action_str;

      if (keys.size() > 1) {
        line += " (Alternatives : " + get_key_string(keys[1]);

        if (keys.size() > 2)
          line += ", " + get_key_string(keys[2]);

        line += ").";
      }

      std::cout
        << std::setw(12)
        << (keys.size() > 0 ? get_key_string(keys[0]) : "(unbound)")
        << line
        << std::setw(0) 
        << std::endl;
    };

    for (ActionEnum action : sorted_camera_actions){
      std::vector<KeyData> keys = action_keys[action];

      print_action(keys, action);
      
      action_keys.erase(action);
    }

    for (auto pair : action_keys){
      print_action(pair.second, pair.first);
    }
  }

  void Basic_Viewer::zoom(float z){
    if (cam_mode == ORTHOGRAPHIC){
      cam_orth_zoom += z;
      set_cam_mode(ORTHOGRAPHIC);
    }
  }

  // Blocking call
  inline void draw_graphics_scene(const Graphics_scene &graphics_scene, const char *title)
  {
    Basic_Viewer(graphics_scene, title).show();
  }
} // End OpenGL Namespace
} // End CGAL Namespace