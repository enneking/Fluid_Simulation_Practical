// This file is part of GLviz.
//
// Copyright(c) 2014, 2015 Sebastian Lipponer
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <GLviz>

#include "config.hpp"

#include <Eigen/Core>

#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <exception>
#include <cmath>

using namespace Eigen;

namespace
{

GLviz::Camera camera;

float g_time(0.0f);
bool g_stop_simulation(true);

bool g_enable_mesh3(true);
bool g_enable_wireframe(false);

bool g_enable_points(false);
float g_point_radius(0.0014f);
float g_projection_radius(0.0f);

float g_wireframe[4] = {
    0.0f, 0.0f, 0.0f, 1.0f
};

float g_mesh_material[4] = {
    0.0f, 0.25f, 1.0f, 8.0f
};

float g_points_material[4] = {
    1.0f, 1.0f, 1.0f, 8.0f
};

int g_shading_method(0);

struct MyViz
{
    MyViz()
    {
        // Setup vertex array v.
        vertex_array_v.bind();

        vertex_array_buffer.bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
        vertex_array_v.unbind();

        // Setup vertex array vf.
        vertex_array_vf.bind();

        vertex_array_buffer.bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

        index_array_buffer.bind();
        vertex_array_buffer.unbind();

        vertex_array_vf.unbind();

        // Setup vertex array vnf.
        vertex_array_vnf.bind();
        
        vertex_array_buffer.bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

        normal_array_buffer.bind();
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

        index_array_buffer.bind();
        vertex_array_buffer.unbind();

        vertex_array_vnf.unbind();

        // Bind uniforms to their binding points.
        uniform_camera.bind_buffer_base(0);
        uniform_material.bind_buffer_base(1);
        uniform_wireframe.bind_buffer_base(2);
        uniform_sphere.bind_buffer_base(3);

        camera.translate(Eigen::Vector3f(0.0f, 0.0f, -2.0f));
    }

    void draw_mesh3(GLsizei nf)
    {
        program_mesh3.use();

        if (g_shading_method == 0)
        {
            // Flat.
            vertex_array_vf.bind();
            glDrawElements(GL_TRIANGLES, nf,
                GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
            vertex_array_vf.unbind();
        }
        else
        {
            // Smooth.
            vertex_array_vnf.bind();
            glDrawElements(GL_TRIANGLES, nf,
                GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
            vertex_array_vnf.unbind();
        }

        program_mesh3.unuse();
    }

    void draw_spheres(GLsizei nv)
    {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

        program_sphere.use();

        vertex_array_v.bind();
        glDrawArrays(GL_POINTS, 0, nv);
        vertex_array_v.unbind();

        program_sphere.unuse();
    }

    GLviz::glVertexArray  vertex_array_v, vertex_array_vf, vertex_array_vnf;
    GLviz::glArrayBuffer  vertex_array_buffer, normal_array_buffer;
    GLviz::glElementArrayBuffer  index_array_buffer;

    GLviz::UniformBufferCamera      uniform_camera;
    GLviz::UniformBufferMaterial    uniform_material;
    GLviz::UniformBufferWireframe   uniform_wireframe;
    GLviz::UniformBufferSphere      uniform_sphere;

    GLviz::ProgramMesh3   program_mesh3;
    GLviz::ProgramSphere  program_sphere;
};

std::unique_ptr<MyViz> viz;

std::vector<Eigen::Vector3f>               m_ref_vertices;
std::vector<Eigen::Vector3f>               m_ref_normals;

std::vector<Eigen::Vector3f>               m_vertices;
std::vector<Eigen::Vector3f>               m_normals;
std::vector<std::array<unsigned int, 3> >  m_faces;

void load_triangle_mesh(std::string const& filename);

void
displayFunc()
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    viz->vertex_array_buffer.set_buffer_data(3 * sizeof(GLfloat)
        * m_vertices.size(), m_vertices.front().data());
    viz->normal_array_buffer.set_buffer_data(3 * sizeof(GLfloat)
        * m_normals.size(), m_normals.front().data());
    viz->index_array_buffer.set_buffer_data(3 * sizeof(GLuint)
        * m_faces.size(), m_faces.front().data());

    viz->uniform_camera.set_buffer_data(camera);

    if (g_enable_mesh3)
    {
        viz->uniform_material.set_buffer_data(g_mesh_material);
        
        viz->program_mesh3.set_wireframe(g_enable_wireframe);
        int screen[2] = { GLviz::screen_width(), GLviz::screen_height() };
        viz->uniform_wireframe.set_buffer_data(g_wireframe, screen);        

        viz->program_mesh3.set_smooth(g_shading_method != 0);
        viz->draw_mesh3(static_cast<GLsizei>(3 * m_faces.size()));
    }

    if (g_enable_points)
    {
        viz->uniform_material.set_buffer_data(g_points_material);
        GLviz::Frustum view_frustum = camera.get_frustum();
        float g_projection_radius = view_frustum.near_() * (GLviz::screen_height()
            / (view_frustum.top() - view_frustum.bottom()));

        viz->uniform_sphere.set_buffer_data(g_point_radius, g_projection_radius);

        viz->draw_spheres(static_cast<GLsizei>(m_vertices.size()));
    }
}

void
reshapeFunc(int width, int height)
{
    const float aspect = static_cast<float>(width) /
        static_cast<float>(height);

    glViewport(0, 0, width, height);
    camera.set_perspective(60.0f, aspect, 0.005f, 5.0f);
}

void
closeFunc()
{
    viz = nullptr;
}

void
timerFunc(int delta_t_msec)
{
    float delta_t_sec = static_cast<float>(delta_t_msec) / 1000.0f;

    if (!g_stop_simulation)
    {
        g_time += delta_t_sec;

        const float k = 50.0f;
        const float a = 0.03f;
        const float v = 10.0f;

        for (unsigned int i(0); i < m_vertices.size(); ++i)
        {
            const float x = m_ref_vertices[i].x() + m_ref_vertices[i].y()
                + m_ref_vertices[i].z();

            const float u = 5.0f * (x - 0.75f * std::sin(2.5f * g_time));
            const float w = (a / 2.0f) * (1.0f
                + std::sin(k * x + v * g_time));

            m_vertices[i] = m_ref_vertices[i] + (std::exp(-u * u) * w)
                * m_ref_normals[i];
        }

        GLviz::set_vertex_normals_from_triangle_mesh(
            m_vertices, m_faces, m_normals);
    }
}

void TW_CALL reset_simulation(void*)
{
    g_time = 0.0f;
}

void TW_CALL
export_frame(void*)
{ 
  GLviz::export_frame("export");
}

void
load_triangle_mesh(std::string const& filename)
{
    std::cout << "\nRead " << filename << "." << std::endl;
    std::ifstream input(filename);

    if (input.good())
    {
        input.close();
        GLviz::load_raw(filename, m_vertices, m_faces);
    }
    else
    {
        input.close();

        std::ostringstream fqfn;
        fqfn << path_resources;
        fqfn << filename;
        GLviz::load_raw(fqfn.str(), m_vertices, m_faces);
    }

    std::cout << "  #vertices " << m_vertices.size() << std::endl;
    std::cout << "  #faces    " << m_faces.size() << std::endl;

    GLviz::set_vertex_normals_from_triangle_mesh(
        m_vertices, m_faces, m_normals);

    m_ref_vertices = m_vertices;
    m_ref_normals = m_normals;
}

}

int
main(int argc, char* argv[])
{
    GLviz::init(argc, argv);
  
    viz = std::unique_ptr<MyViz>(new MyViz());
    
    try
    {
        load_triangle_mesh("stanford_dragon_v40k_f80k.raw");
    }
    catch(std::runtime_error const& e)
    {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Setup AntTweakBar.
    {
        TwBar* bar = GLviz::twbar();

        TwAddVarRO(bar, "time", TW_TYPE_FLOAT,
            &g_time, " precision=3 label='t in sec' group='Simulation' ");

        TwAddButton(bar, "Reset",
            reset_simulation, NULL,
            " key=r help='Reset simulation' group='Simulation' ");

        TwAddVarRW(bar, "Stop", TW_TYPE_BOOLCPP,
            &g_stop_simulation,
            " key=SPACE help='Stop simulation' group='Simulation' ");

        TwAddVarRW(bar, "Draw Triangle Mesh", TW_TYPE_BOOLCPP,
            &g_enable_mesh3,
            " key=1 help='Draw Triangle Mesh' group='Triangle Mesh' ");

        TwType shading_type = TwDefineEnumFromString("shading_type", "Flat,Phong");
        TwAddVarRW(bar, "Shading", shading_type, &g_shading_method, " key=5 group='Triangle Mesh' ");

        TwAddSeparator(bar, NULL, " group='Triangle Mesh' ");

        TwAddVarRW(bar, "Wireframe", TW_TYPE_BOOLCPP,
            &g_enable_wireframe,
            " key=w help='Draw Wireframe' group='Triangle Mesh' ");

        TwAddVarRW(bar, "Wireframe Color", TW_TYPE_COLOR3F,
            g_wireframe,
            " help='Wireframe Color' group='Triangle Mesh' ");

        TwAddSeparator(bar, NULL, " group='Triangle Mesh' ");

        TwAddVarRW(bar, "Mesh Material", TW_TYPE_COLOR3F,
            g_mesh_material,
            " help='Mesh Ambient' group='Triangle Mesh' ");

        TwAddVarRW(bar, "Mesh Shininess", TW_TYPE_FLOAT,
            &g_mesh_material[3],
            " min=1e-12 max=1000 help='Mesh Shininess' group='Triangle Mesh' ");

        TwAddVarRW(bar, "Draw Points", TW_TYPE_BOOLCPP,
            &g_enable_points,
            " key=2 help='Draw Points' group='Points' ");

        TwAddVarRW(bar, "Radius", TW_TYPE_FLOAT,
            &g_point_radius,
            " min=0 max=0.1 step=0.0001 key=2 help='Radius' group='Points' ");

        TwAddVarRW(bar, "Points Material", TW_TYPE_COLOR3F,
            g_points_material,
            " help='Points Ambient' group='Points' ");

        TwAddVarRW(bar, "Points Shininess", TW_TYPE_FLOAT,
            &g_points_material[3],
            " min=1e-12 max=1000 help='Points Shininess' group='Points' ");

        TwAddButton(bar, "Export Frame", export_frame, nullptr, nullptr);
    }

    GLviz::display_callback(displayFunc);
    GLviz::reshape_callback(reshapeFunc);
    GLviz::close_callback(closeFunc);
    GLviz::timer_callback(timerFunc, 15);

    return GLviz::exec(camera);
}
