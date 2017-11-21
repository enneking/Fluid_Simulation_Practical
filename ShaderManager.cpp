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
void ShaderManager::initializeShader( ){

	//create shader and program
	m_vs = glCreateShader( GL_VERTEX_SHADER );
	m_fs = glCreateShader( GL_FRAGMENT_SHADER );
	m_prog = glCreateProgram();


	//read content from files
	const std::string fileContent_vs = FileReader::getFileContent( m_fileNameVs );
	const std::string fileContent_fs = FileReader::getFileContent( m_fileNameFs );


	//compile vertex shader
	printf("Compiling vertex shader... : %s\n", m_fileNameVs.c_str());
	const GLchar *vscr = fileContent_vs.c_str();
	glShaderSource(m_vs, 1, &vscr, NULL);
	glCompileShader( m_vs );
	FileReader::printGLSLCompileLog( m_vs );


	//compile fragment shader
	printf("Compiling fragment shader... : %s\n", m_fileNameFs.c_str());
	const GLchar  *fscr = fileContent_fs.c_str();
	glShaderSource(m_fs, 1, &fscr, NULL);
	glCompileShader( m_fs );
	FileReader::printGLSLCompileLog( m_fs );

	// link program
	glBindAttribLocation( m_prog, 0, "aPosition" );
	glBindAttribLocation( m_prog, 1, "aNormal" );
	glBindAttribLocation( m_prog, 2, "aColor" );
	fprintf(stdout, "Linking program... \n");
	glAttachShader( m_prog, m_vs );
	glAttachShader( m_prog, m_fs );
	glLinkProgram( m_prog );
	FileReader::printGLSLLinkLog( m_prog );

	fprintf(stdout, "initialization finished! \n");

}

ShaderManager::ShaderManager( const std::string fileNameFragmentShader, const std::string fileNameVertexShader)
: m_fileNameFs( fileNameFragmentShader), m_fileNameVs( fileNameVertexShader){
}



