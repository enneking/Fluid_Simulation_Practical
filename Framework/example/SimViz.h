#pragma once

#include <GLviz>
#include <iostream>
#include <fstream>

struct SimViz
{
	SimViz();

	void Init();

	void draw_mesh3(GLsizei nf);

	void draw_spheres(GLsizei nv);

	void displayFunc();

	void reshapeFunc(int width, int height);

	void timerFunc(int delta_t_msec);

	void load_triangle_mesh(std::string const& filename);

	GLviz::glVertexArray  vertex_array_v, vertex_array_vf, vertex_array_vnf;
	GLviz::glArrayBuffer  vertex_array_buffer, normal_array_buffer;
	GLviz::glElementArrayBuffer  index_array_buffer;

	GLviz::UniformBufferCamera      uniform_camera;
	GLviz::UniformBufferMaterial    uniform_material;
	GLviz::UniformBufferWireframe   uniform_wireframe;
	GLviz::UniformBufferSphere      uniform_sphere;

	GLviz::ProgramMesh3   program_mesh3;
	GLviz::ProgramSphere  program_sphere;

	GLviz::Camera camera;

	float g_time= 0.0f;
	bool g_stop_simulation = true;

	bool g_enable_mesh3 = true;
	bool g_enable_wireframe = false;

	bool g_enable_points = false;
	float g_point_radius = 0.0014f;
	float g_projection_radius = 0.0f;

	float g_wireframe[4] = {
		0.0f, 0.0f, 0.0f, 1.0f
	};

	float g_mesh_material[4] = {
		0.0f, 0.25f, 1.0f, 8.0f
	};

	float g_points_material[4] = {
		1.0f, 1.0f, 1.0f, 8.0f
	};

	int g_shading_method = 0;

	std::vector<Eigen::Vector3f>               m_ref_vertices;
	std::vector<Eigen::Vector3f>               m_ref_normals;

	std::vector<Eigen::Vector3f>               m_vertices;
	std::vector<Eigen::Vector3f>               m_normals;
	std::vector<std::array<unsigned int, 3> >  m_faces;
};