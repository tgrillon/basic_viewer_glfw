#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
    enum ViewMode {
        ORTHOGRAPHIC=0,
        PERSPECTIVE,
        NB_VIEW_MODE
    };

    enum RotationMode {
        CENTER=0, 
        WALK,
        NB_ROT_MODE
    };
public:
    Camera(glm::vec3 const& eye, glm::vec3 const& lookat, glm::vec3& up, float fov=45.0f, float near=0.1f, float far=100.0f);
    Camera(float fov=45.0f, float near=0.1f, float far=1000.0f);

    void set_cameraView(glm::vec3 const& eye, glm::vec3 const& lookat, glm::vec3 const& up);
    void update_viewMatrix();

    ViewMode switch_viewMode();
    RotationMode switch_rotationMode();

    inline glm::vec3 eye() const { return m_eye; }
    inline void set_eye(glm::vec3 const& eye) { m_eye = eye; }
    inline glm::vec3 lookat() const { return m_lookat; }
    inline void lookat(glm::vec3 const& lookat) { m_lookat = lookat; }
    inline glm::vec3 up() const { return m_up; }
    inline void set_up(glm::vec3 const& up) { m_up = up; }
    inline glm::mat4 viewMat() const { return m_viewMatrix; }

    inline ViewMode vMode() const { return m_vMode; }
    inline void set_vMode(ViewMode mode) { m_vMode = mode; }
    inline RotationMode rMode() const { return m_rMode; }
    inline void set_rMode(RotationMode mode) { m_rMode = mode; }

    inline float fov() const { return m_fov; }
    inline void set_fov(float fov) { m_fov = fov; }
    inline float near() const { return m_near; }
    inline void set_near(float near) { m_near = near; }
    inline float far() const { return m_far; }
    inline void set_far(float far) { m_far = far; }

    inline glm::vec3 forward() const { return -glm::transpose(m_viewMatrix)[2]; }
    inline glm::vec3 right() const { return -glm::transpose(m_viewMatrix)[0]; }

    inline bool is_orthographic() const { return m_vMode == ORTHOGRAPHIC; }
    inline bool is_centered() { return m_rMode == CENTER; }
private:
    glm::mat4 m_viewMatrix;
    glm::vec3 m_eye;
    glm::vec3 m_lookat;
    glm::vec3 m_up;

    ViewMode m_vMode;
    RotationMode m_rMode;

    float m_fov;
    float m_near;  
    float m_far;  
};

Camera::Camera(float fov, float near, float far) 
: m_eye({0,0,5}), m_lookat({0,0,0}), m_up(0,1,0), m_vMode(PERSPECTIVE), m_rMode(CENTER), m_fov(fov), m_near(near), m_far(far)
{ 
    update_viewMatrix();
}

Camera::Camera(glm::vec3 const& eye, glm::vec3 const& lookat, glm::vec3& up, float fov, float near, float far) 
: m_eye(eye), m_lookat(lookat), m_up(up), m_fov(fov), m_vMode(PERSPECTIVE), m_rMode(CENTER), m_near(near), m_far(far) 
{     
    update_viewMatrix(); 
}

void Camera::set_cameraView(glm::vec3 const& eye, glm::vec3 const& lookat, glm::vec3 const& up) {
    m_eye = eye;
    m_lookat = lookat;
    m_up = up;

    update_viewMatrix();
}

void Camera::update_viewMatrix() {
    m_viewMatrix = glm::lookAt(m_eye, m_lookat, m_up);
}

Camera::ViewMode Camera::switch_viewMode() {
    m_vMode = ViewMode(((int)m_vMode+1) % (int)NB_VIEW_MODE); 
    return m_vMode;
}

Camera::RotationMode Camera::switch_rotationMode() {
    m_rMode = RotationMode(((int)m_rMode+1) % (int)NB_ROT_MODE); 
    return m_rMode;
}