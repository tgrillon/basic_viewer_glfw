#include <glad/glad.h>

namespace CGAL {
namespace OpenGL {
    class Camera {
    public:
        enum Type { PERSPECTIVE, ORTHOGRAPHIC };


    private:
        glm::mat4 m_view;
        glm::mat4 m_projection;
    }
}}