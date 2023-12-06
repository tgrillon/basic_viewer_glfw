// System headers
#include <CGAL/Graphics_scene.h>
#include "OpenGL/Basic_viewer.h"
#include <CGAL/Point_3.h>
#include <CGAL/Cartesian.h>
// Standard headers
#include <cstdlib>

// A callback which allows GLFW to report errors whenever they occur
static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
}


int main(int argc, char* argb[])
{
    CGAL::Graphics_scene scene;

    typedef CGAL::Point_3<CGAL::Cartesian<float>> Point;
    scene.face_begin();
    scene.add_point_in_face(Point(0.5f,0.5f,0));
    scene.add_point_in_face(Point(0.5,-0.5f,0));
    scene.add_point_in_face(Point(-0.5f,0.5f,0));
    scene.face_end();

    CGAL::OpenGL::Basic_Viewer test(scene, "Test opengl");

    test.show();

    return EXIT_SUCCESS;
}
