/*
 * FileReader.cpp
 *
 *  Created on: Nov 23, 2015
 *      Author: alexander
 */
#include "FileReader.h"



/**
 * reads a textfile and returns the content as one string
 */
std::string FileReader::getFileContent(const std::string fileName) {
    std::string line = "";
    std::string fileContent = "";

    std::ifstream fileStream(fileName.c_str(), std::ifstream::in);

    if(fileStream.is_open()) {
        while(fileStream.good()) {
            std::getline(fileStream, line);
            fileContent += line + "\n";
        }
        fileStream.close();
    } else {
        std::cout << "Failed to open file: " << fileName << std::endl;
        return "";
    }
    return fileContent;
}

/**
 * check for shader compile errors
 */
void FileReader::printGLSLCompileLog(GLuint shaderHandle) {
    GLint shaderError;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &shaderError);
    if(shaderError != GL_TRUE) {
        std::cerr << "Shader compile error: " << std::endl;
    }

    // a log gets always printed (could be warnings)
    GLsizei length = 0;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        // a compile log can get produced even if there were no errors, but warnings!
        GLchar* pInfo = new char[length + 1];
        glGetShaderInfoLog(shaderHandle, length, &length, pInfo);
        std::cerr << "Compile log: " << std::string(pInfo) << std::endl;
        delete[] pInfo;
    }
}


void FileReader::printGLSLLinkLog(GLuint progHandle) {
    // check for program link errors:
    GLint programError;
    glGetProgramiv(progHandle, GL_LINK_STATUS, &programError);

    if(programError != GL_TRUE) {
        std::cerr << "Program could not get linked:" << std::endl;
    }

    GLsizei length = 0;
    glGetProgramiv(progHandle, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        // error log or warnings:
        GLchar* pInfo = new char[length + 1];
        glGetProgramInfoLog(progHandle, length, &length, pInfo);
        std::cout << "Linker log: " << std::string(pInfo) << std::endl;
        delete[] pInfo;
    }
}