#pragma once
#ifndef C3DMODEL_H_FBX
#define C3DMODEL_H_FBX

#include <string>
using namespace std;

#include "C3DModel.h"

class C3DModel_FBX : public C3DModel
{
private:

	int m_currentVertex, m_currentNormal, m_currentUV, m_currentFace;

	bool readFBXFile(const char * const filename,
		bool countOnly);

	bool parseFBXLine(std::ifstream &file, std::string &line, bool countOnly, int &lineNumber);

	bool readMtlib(std::string mtLibFilename);

protected:


	void reset();
	bool loadFromFile(const char * const filename);


public:
	C3DModel_FBX();
	~C3DModel_FBX();
};
#endif // !C3DMODEL_H_FBX