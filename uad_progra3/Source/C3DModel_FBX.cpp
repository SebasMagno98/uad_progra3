#include "../stdafx.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#include "../Include/CVector3.h"
#include "../Include/C3DModel.h"
#include "../Include/C3DModel_FBX.h"


//------------------------------------------------------------------------------

void C3DModel_FBX::reset()
{
	C3DModel::reset();

	m_currentVertex = 0;
	m_currentNormal = 0;
	m_currentUV = 0;
	m_currentFace = 0;
}

//------------------------------------------------------------------------------


bool C3DModel_FBX::loadFromFile(const char * const filename)
{
	bool readFileOK = false;
	reset();
	readFileOK = readFBXFile(filename, false);

	cout << "Finished reading 3D model" << endl;
	cout << "Vertices: " << m_numVertices << endl;
	cout << "Normals: " << m_numNormals << endl;
	cout << "UVCoords: " << m_numUVCoords << endl;

	if (readFileOK)
	{
		// Check for MAX number of faces
		if (m_numVertices >= 65535 || m_numNormals >= 65535 || m_numUVCoords >= 65535)
		{
			cout << "Error: object cannot have more than 65535 vertices" << endl;
			cout << "Object attempted to load has: " << m_numVertices << " vertices" << endl;
			return false;
		}

		// If model was read without normals or UVCoords, we'll set a default value for them
		// i.e.:
		//   0,0 for UV coords
		//   face normal for normal
		if (m_numNormals == 0)
		{
			m_modelHasNormals = false;
			m_numNormals = m_numVertices;
		}
		else
		{
			m_modelHasNormals = true;
		}

		if (m_numUVCoords == 0)
		{
			m_numUVCoords = m_numVertices;
			m_modelHasUVs = false;
		}
		else
		{
			m_modelHasUVs = true;
		}

		if (readFileOK)
		{
			m_modelGeometryLoaded = true;

			if (!m_modelHasNormals)
			{
				computeFaceNormals();
			}
		}
	}
	else
	{
		cout << "Error ocurred while reading 3D model." << endl;
	}

	return readFileOK;
}

//------------------------------------------------------------------------------

C3DModel_FBX::C3DModel_FBX() :
	C3DModel(),
	m_currentVertex(0),
	m_currentNormal(0),
	m_currentUV(0),
	m_currentFace(0)
{
	cout << "Constructor: C3DModel_FBX()" << endl;
}


//------------------------------------------------------------------------------

C3DModel_FBX::~C3DModel_FBX()
{
	cout << "Destructor: C3DModel_FBX()" << endl;
	reset();
}

//------------------------------------------------------------------------------

bool C3DModel_FBX::readFBXFile(const char * const filename, bool countOnly)
{
	ifstream infile;
	string Line_Buffer;
	bool Read_File_Ok = true;
	int Line_Number = 0;

	infile.open(filename);

	if ((infile.rdstate() & std::ifstream::failbit) != 0)
	{
		cout << "Error opening file: " << filename << endl;
		return false;
	}

	while (!infile.eof())
	{
		getline(infile, Line_Buffer);
		Line_Number++;

		if (!(this->parseFBXLine(infile, Line_Buffer, countOnly, Line_Number)))
		{
			Read_File_Ok = false;
			break;
		}
	}

	infile.close();
	return Read_File_Ok;
}

//------------------------------------------------------------------------------

bool C3DModel_FBX::parseFBXLine(std::ifstream &file,std::string &line, bool countOnly, int &Line_Number)
{
	bool parsed = false;
	bool unrecognizedLine = false;

	bool readingVertex = false;
	bool readingNormal = false;
	bool readingUV = false;
	bool readingTexture = false;

	char *nextToken = nullptr;
	char *token = nullptr;
	char *token2 = nullptr;
	char *nextToken2 = nullptr;

	const char *delimiterToken = " ,:\t";
	const char *delimiterData = "/";

	int currentToken = 0;
	int numTokens = 0;
	int numExpectedTokens = 4;

	vector<string> tokens;

	string materialFilename;
	string originalLine = line;

	token = strtok_s((char *)line.c_str(), delimiterToken, &nextToken);

	if (token == NULL)
	{
		parsed = true;
	}

	while (token != NULL)   //Se entra en el ciclo.
	{

		if (currentToken == 0)
		{
			
			if (strcmp(token, "Vertices") == 0)  //Vertices
			{
				token = strtok_s(nextToken, " *", &nextToken2);

				m_numVertices = stoi(token) / 3;   //Triangulos

				m_verticesRaw = new float[m_numVertices * 3];  //Se dan las coordenadas

				getline(file, line); // Se obtiene una linea.

				Line_Number++;// Se pasa a la siguiente

				token = strtok_s((char *)line.c_str(), "a: ,", &nextToken);  //Datos de los vertices

				for (int i = 0; i < m_numVertices * 3; i++) // Ciclo que se realizará por cada linea del archivo, saldra hasta encontrar una llave.
				{
					if (token == NULL || *token == '\n' || *token == '\0' || *token == ' ')
					{
						getline(file, line);
						Line_Number++;
						if (line == "} ")
						{
							break;
						}
						else
						{
							token = strtok_s((char*)line.c_str(), ",", &nextToken); 
							
						}
					}
					m_verticesRaw[i] = stof(token);
					token = strtok_s(NULL, ",", &nextToken);
				}
				parsed = true;
			}

			else if (strcmp(token, "PolygonVertexIndex") == 0)  //Indices de los vertices
			{
				token = strtok_s(nextToken, " *", &nextToken2); // Se crea  un token que almacena parte del texto del archivo.

				m_numFaces = stoi(token) / 3; // Triangulos 

				m_vertexIndices = new unsigned short[m_numFaces * 3]; // Coordenadas

				getline(file, line);

				Line_Number++;

				token = strtok_s((char *)line.c_str(), "a: ,", &nextToken);

				for (int i = 0; i < m_numFaces * 3; i++)
				{
					if (token == NULL || *token == '\n' || *token == '\0' || *token == ' ')
					{
						getline(file, line);

						Line_Number++;

						if (line == "} ")
						{
							break;
						}
						else
						{
							token = strtok_s((char*)line.c_str(), ",", &nextToken);
						}
					}
					short Temp = stoi(token);
					if (Temp < 0)
					{
						Temp *= -1;
						Temp -= 1;
					}
					m_vertexIndices[i] = Temp;
					token = strtok_s(NULL, ",", &nextToken);
				}
				parsed = true;
			}

			
			else if (strcmp(token, "UV") == 0)
			{
				token = strtok_s(nextToken, " *", &nextToken2);

				m_numUVCoords = stof(token) / 2;

				m_uvCoordsRaw = new float[m_numUVCoords * 2];

				getline(file, line);

				Line_Number++;

				token = strtok_s((char*)line.c_str(), "a: ,", &nextToken);

				for (int i = 0; i < m_numUVCoords * 2; i++)
				{
					if (token == NULL || *token == '\n' || *token == '\0' || *token == ' ')
					{
						getline(file, line);
						Line_Number++;
						if (line == "} ")
						{
							break;
						}
						else
						{
							token = strtok_s((char*)line.c_str(), ",", &nextToken);
						}
					}
					m_uvCoordsRaw[i] = stof(token);

					token = strtok_s(NULL, ",", &nextToken);
				}
				parsed = true;
			}

			else if (strcmp(token, "UVIndex") == 0)
			{
				int UVIndex;

				token = strtok_s(nextToken, " *", &nextToken2);

				UVIndex = stoi(token) / 3;

				m_UVindices = new unsigned short[UVIndex * 3];

				m_normalIndices = new unsigned short[UVIndex * 3];

				getline(file, line);

				Line_Number++;

				token = strtok_s((char *)line.c_str(), "a: ,", &nextToken);

				for (int i = 0; i < UVIndex * 3; i++)
				{
					if (token == NULL || *token == '\n' || *token == '\0' || *token == ' ')
					{
						getline(file, line);
						Line_Number++;
						if (line == "} ")
						{
							break;
						}
						else
						{
							token = strtok_s((char*)line.c_str(), ",", &nextToken);
						}
					}
					m_UVindices[i] = (unsigned short)stoi(token);

					m_normalIndices[i] = (unsigned short)stoi(token);

					token = strtok_s(NULL, ",", &nextToken);

				}
				parsed = true;
			}

			else if (strcmp(token, "Normals") == 0)
			{
				token = strtok_s(nextToken, " *", &nextToken2);

				m_numNormals = (stoi(token) / 3) / 3;

				m_normalsRaw = new float[(m_numNormals * 3) * 3];

				getline(file, line);
				Line_Number++;


				token = strtok_s((char *)line.c_str(), "a: ,", &nextToken);

				for (int i = 0; i < (m_numNormals * 3) * 3; i++)
				{
					if (token == NULL || *token == '\n' || *token == '\0' || *token == ' ')
					{
						getline(file, line);
						Line_Number++;
						if (line == "} ")
						{
							break;
						}
						else
						{
							token = strtok_s((char*)line.c_str(), ",", &nextToken);
						}
					}
					m_normalsRaw[i] = stof(token);

					token = strtok_s(NULL, ",", &nextToken);
				}
				parsed = true;
			}

			else
			{
				// Unrecognized line
				if (countOnly)
				{
					cout << "Ignoring line #" << Line_Number << ": " << originalLine << endl;
				}				
				unrecognizedLine = true;
			}

			if (countOnly || unrecognizedLine)
			{
				return true;
			}
		}

		else
		{
			tokens.push_back(std::string(token));
		}

	}
	return parsed;
}

//------------------------------------------------------------------------------

bool C3DModel_FBX::readMtlib(std::string mtLibFilename)
{
	return false;
}

//------------------------------------------------------------------------------

