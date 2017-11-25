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

#include <vector>

#include "FileReader.h"

class ShaderManager{
public:

	ShaderManager( ); 
	void initializeShader(const std::string sfileNameFs, const std::string sfileNameVs); 


	//get prog
	GLuint getProg(int Index){ return m_vProgContainer[Index]; }

private:
	std::vector<GLuint> m_vProgContainer; 

    
};
