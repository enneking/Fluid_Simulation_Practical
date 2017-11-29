/*
 * ShaderManager.cpp
 *
 *  Created on: Nov 23, 2015
 *      Author: alexander
 */
#include "ShaderManager.h"


/*
 * initialize the shader programs
 */
void ShaderManager::initializeShader(const std::string sfileNameFs, const std::string sfileNameVs){
	
	GLuint iVS = 0, iFS = 0;
	m_vProgContainer.emplace_back();

	//create shader and program
	iVS = glCreateShader( GL_VERTEX_SHADER );
	iFS = glCreateShader( GL_FRAGMENT_SHADER );
	m_vProgContainer.back() = glCreateProgram();


	//read content from files
	const std::string fileContent_vs = FileReader::getFileContent( sfileNameVs );
	const std::string fileContent_fs = FileReader::getFileContent( sfileNameFs );


	//compile vertex shader
	printf("Compiling vertex shader... : %s\n", sfileNameVs.c_str());
	const GLchar *vscr = fileContent_vs.c_str();
	glShaderSource(iVS, 1, &vscr, NULL);
	glCompileShader(iVS);
	FileReader::printGLSLCompileLog(iVS);


	//compile fragment shader
	printf("Compiling fragment shader... : %s\n", sfileNameFs.c_str());
	const GLchar  *fscr = fileContent_fs.c_str();
	glShaderSource(iFS, 1, &fscr, NULL);
	glCompileShader(iFS);
	FileReader::printGLSLCompileLog(iFS);

	// link program
	glBindAttribLocation(m_vProgContainer.back(), 0, "aPosition" );
	glBindAttribLocation(m_vProgContainer.back(), 1, "aNormal" );
	glBindAttribLocation(m_vProgContainer.back(), 2, "aColor" );
	fprintf(stdout, "Linking program... \n");
	glAttachShader(m_vProgContainer.back(), iVS);
	glAttachShader(m_vProgContainer.back(), iFS);
	glLinkProgram(m_vProgContainer.back());
	FileReader::printGLSLLinkLog(m_vProgContainer.back());

	fprintf(stdout, "initialization finished! \n");

}

ShaderManager::ShaderManager()
{
}



