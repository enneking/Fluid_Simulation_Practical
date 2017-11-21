/*
 * FileReader.h
 *
 *  Created on: Nov 23, 2015
 *      Author: alexander
 */

#pragma once

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

//std libs
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <string>


class FileReader{

public:
	static std::string getFileContent(const std::string fileName); ///< reads a textfile and returns it as string
	static void printGLSLCompileLog(GLuint shaderHandle); ///< checks out for shader compile errors
	static void printGLSLLinkLog(GLuint progHandle); ///< checks out for linker errors

};


