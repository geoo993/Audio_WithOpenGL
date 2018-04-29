#pragma once
#include "Common.h"
#include "vertexBufferObject.h"
#include "vertexBufferObjectIndexed.h"
#include "Texture.h"
#include "Transform.h"


class CCatmullRom
{
public:
	CCatmullRom();
	~CCatmullRom();

	void CreateCentreline(const float &pathWidth, const bool &doTunel);
	void RenderCentreline();
	void RenderOffsetCurves();

    void CreateSpline(const std::string &a_sDirectory, const std::string &a_sFilename );
    
	void RenderPath();
    
    int GetIndexAtCentralLinePosition(const glm::vec3 &p) const { 
        int index = -1;
        for (int i = 0; i < m_centrelinePoints.size(); ++i){
            if (m_centrelinePoints[i].x == p.x && m_centrelinePoints[i].y == p.y && m_centrelinePoints[i].z == p.z){
                index = i;
                break;
            }
        }
        return index; 
    }
    glm::vec3 GetCentralLineStartingPosition() const { return m_centrelinePoints[0]; }
    glm::vec3 GetCentralLinePositionAtIndex(const unsigned int &idx) const { return m_centrelinePoints[idx % m_centrelinePoints.size()]; }
    glm::vec3 GetCentralLineUpVectorAtIndex(const unsigned int &idx) const { return m_centrelineUpVectors[idx % m_centrelineUpVectors.size()]; }
    float GetCentralLineRotationAtIndex(const unsigned int &idx) const { return m_rotationsOverTime[idx % m_rotationsOverTime.size()]; }
    
    unsigned int GetCentralLineSize() const { return m_centrelinePoints.size(); }
    float GetPathWidth(){ return m_pathWidth; }
    
    std::vector<glm::vec3> GetControlPoints() const;
    std::vector<glm::vec3> GetPathCentralPoints() const { return m_centrelinePoints; }
    std::vector<glm::vec3> GetPathLeftPoints() const { return m_leftOffsetPoints; }
    std::vector<glm::vec3> GetPathRightPoints() const { return m_rightOffsetPoints; };
    
	int CurrentLap(float d); // Return the currvent lap (starting from 0) based on distance along the control curve.

	bool Sample(float d, glm::vec3 &p, glm::vec3 &up, float &rotation); // Return a point on the centreline based on a certain distance along the control curve.

    glm::vec3 Interpolate(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, float t);

private:

	void SetControlPoints();
	void ComputeLengthsAlongControlPoints();
	void UniformlySampleControlPoints(int numSamples);
    void CreateLine( GLuint & vao, CVertexBufferObject & vbo, const std::vector<glm::vec3> &ctrlPoints);
    void RenderLine(const GLuint & vao, const std::vector<glm::vec3> & ctrlPoints);
    void Release();
    
    void CreatePathOffsetCurves(const float & pathWidth);
    void CreateTunelOffsetCurves(const float & pathWidth);
    void CreatePath(const std::string &a_sDirectory, const std::string &a_sFilename);
    void CreateTunel(const std::string &a_sDirectory, const std::string &a_sFilename);
    
    std::vector <glm::vec3> DrawCircle( const glm::vec3 & first, const glm::vec3 & second, const int & segmentsCount);
    void RenderTunel(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const int & segments);
    
	std::vector<float> m_distances;
	CTexture m_texture;
    std::string m_directory;
    std::string m_filename;

    CVertexBufferObject m_vboCentreline;
    CVertexBufferObject m_vboLeftOffsetCurve;
    CVertexBufferObject m_vboRightOffsetCurve;
    CVertexBufferObject m_vboPath;
    
	GLuint m_vaoCentreline;
	GLuint m_vaoLeftOffsetCurve;
	GLuint m_vaoRightOffsetCurve;
	GLuint m_vaoPath;

    
    std::vector<glm::vec3> m_controlPoints;		// Control points, which are interpolated to produce the centreline points
	std::vector<glm::vec3> m_controlUpVectors;	// Control upvectors, which are interpolated to produce the centreline upvectors
	std::vector<glm::vec3> m_centrelinePoints;	// Centreline points
	std::vector<glm::vec3> m_centrelineUpVectors;// Centreline upvectors 
    
    std::vector<glm::vec3> m_tunelPoints;//render tunel
    std::vector<glm::vec2> m_tunelPointsTextureCoord;//render tunel texture coord
    std::vector<glm::vec3> m_tunelPointsNormal;//render tunel normal
    
    std::vector<glm::vec3> m_pathRenderingPoints;
    std::vector<glm::vec3> m_pathRenderingPointsNormal;
    
    std::vector<float> m_rotationsOverTime;
    std::vector<float> m_rotationsAtControlPoints;// rotation at control points upvectors

	std::vector<glm::vec3> m_leftOffsetPoints;	// Left offset curve points
	std::vector<glm::vec3> m_rightOffsetPoints;	// Right offset curve points


	unsigned int m_vertexCount;				// Number of vertices in the track VBO
    
    float m_pathWidth;
    bool m_doTunel;

public:
    CTransform transform;
    
};
