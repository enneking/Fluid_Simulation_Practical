#include "SimViz.h"

SimViz::SimViz()
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

void SimViz::Init()
{
	GLviz::display_callback(std::bind(&SimViz::displayFunc, this));

	GLviz::reshape_callback(
		std::bind(&SimViz::reshapeFunc,this, std::placeholders::_1, std::placeholders::_2));

	GLviz::timer_callback(std::bind(&SimViz::timerFunc, this, std::placeholders::_1), 15);
}

void SimViz::draw_mesh3(GLsizei nf)
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

void SimViz::draw_spheres(GLsizei nv)
{
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

	program_sphere.use();

	vertex_array_v.bind();
	glDrawArrays(GL_POINTS, 0, nv);
	vertex_array_v.unbind();

	program_sphere.unuse();
}

void SimViz::displayFunc()
{
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vertex_array_buffer.set_buffer_data(3 * sizeof(GLfloat)
		* m_vertices.size(), m_vertices.front().data());
	normal_array_buffer.set_buffer_data(3 * sizeof(GLfloat)
		* m_normals.size(), m_normals.front().data());
	index_array_buffer.set_buffer_data(3 * sizeof(GLuint)
		* m_faces.size(), m_faces.front().data());

	uniform_camera.set_buffer_data(camera);

	if (g_enable_mesh3)
	{
		uniform_material.set_buffer_data(g_mesh_material);

		program_mesh3.set_wireframe(g_enable_wireframe);
		int screen[2] = { GLviz::screen_width(), GLviz::screen_height() };
		uniform_wireframe.set_buffer_data(g_wireframe, screen);

		program_mesh3.set_smooth(g_shading_method != 0);
		draw_mesh3(static_cast<GLsizei>(3 * m_faces.size()));
	}

	if (g_enable_points)
	{
		uniform_material.set_buffer_data(g_points_material);
		GLviz::Frustum view_frustum = camera.get_frustum();
		float g_projection_radius = view_frustum.near_() * (GLviz::screen_height()
			/ (view_frustum.top() - view_frustum.bottom()));

		uniform_sphere.set_buffer_data(g_point_radius, g_projection_radius);

		draw_spheres(static_cast<GLsizei>(m_vertices.size()));
	}
}

void SimViz::reshapeFunc(int width, int height)
{
	const float aspect = static_cast<float>(width) /
		static_cast<float>(height);

	glViewport(0, 0, width, height);
	camera.set_perspective(60.0f, aspect, 0.005f, 5.0f);
}

void SimViz::timerFunc(int delta_t_msec)
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


void SimViz::load_triangle_mesh(std::string const& filename)
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
		fqfn << R"(D:\Documents\computer science\studies\current studies\Fluid Simulation in Computer Graphics\Framework\resources\)";
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
