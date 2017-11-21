/*
 * ShaderManager.h
 *
 *  Created on: Nov 23, 2015
 *      Author: alexander
 */

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//std libs
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#include "FileReader.h"

class ShaderManager{
public:

	ShaderManager( const std::string fileNameFragmentShader,
	const std::string fileNameVertexShader ); ///< constructs a ShaderManager object from the filenames of the fragment and vertex shader
	void initializeShader(); ///< compiles and links the shader into the program


	//get prog
	GLuint getProg(){ return m_prog; } ///< get the program

private:
	GLuint m_vs=0, m_fs=0, m_prog=0; ///< the vertex and fragment shader and the program
	const std::string m_fileNameFs, m_fileNameVs; ///< strings that contain the name of the vertex and fragment shader files


};
