// System headers
#include <CGAL/Graphics_scene.h>
#include "OpenGL/Basic_viewer.h"
#include <CGAL/Point_3.h>
#include <CGAL/Vector_3.h>
#include <CGAL/Cartesian.h>
#include <CGAL/IO/Color.h>
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
    typedef CGAL::Vector_3<CGAL::Cartesian<float>> Vector;
    
    Vector normal = Vector(0.0f,0.0f,-1.0f);

    scene.face_begin(CGAL::Color(255, 255, 0));
    scene.add_point_in_face(Point(0.5f,0.5f,0), normal);
    scene.add_point_in_face(Point(0.5,-0.5f,0), normal);
    scene.add_point_in_face(Point(-0.5f,0.5f,0), normal);
    scene.face_end();

    //CGAL::Color(255, 255, 0)

    scene.face_begin(CGAL::Color(255, 255, 0));
    scene.add_point_in_face(Point(-0.5f,0.5f,0), normal);
    scene.add_point_in_face(Point(0.5f,-0.5f,0), normal);
    scene.add_point_in_face(Point(-0.5,-0.5f,0), normal);
    scene.face_end();


    scene.face_begin();
    scene.add_point_in_face(Point(0.5f,0.5f,1), normal);
    scene.add_point_in_face(Point(0.5,-0.5f,1), normal);
    scene.add_point_in_face(Point(-0.5f,0.5f,1), normal);
    scene.face_end();

    //CGAL::Color(255, 255, 0)

    scene.face_begin();
    scene.add_point_in_face(Point(-0.5f,0.5f,1), normal);
    scene.add_point_in_face(Point(0.5f,-0.5f,1), normal);
    scene.add_point_in_face(Point(-0.5,-0.5f,1), normal);
    scene.face_end();
    

    auto array = scene.get_array_of_index(CGAL::Graphics_scene::POS_COLORED_FACES).data();
    auto index = scene.get_size_of_index(CGAL::Graphics_scene::POS_COLORED_FACES);



    CGAL::OpenGL::draw_graphics_scene(scene, "Test opengl");

    return EXIT_SUCCESS;
}
