#include "CatmullRom.h"

// MARK: - Constructor
CCatmullRom::CCatmullRom()
{
	m_vertexCount = 0;
}

CCatmullRom::~CCatmullRom()
{
    Release();
}


// MARK: - Line
void CCatmullRom::CreateLine( GLuint & vao, CVertexBufferObject & vbo, const std::vector<glm::vec3> & ctrlPoints)
{
    // Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a VBO
    vbo.Create();
    vbo.Bind();


    glm::vec2 texCoord(1.0f, 1.0f);
    glm::vec3 normal(0.0f, 1.0f, 0.0f);//define a normal for any position

    for (unsigned int i = 0; i < ctrlPoints.size(); i++) {
        float t = (float)i / 100.0f;
        glm::vec3 p0 = ctrlPoints[i];
        glm::vec3 p1 = ctrlPoints[ i + 1];
        glm::vec3 p2 = ctrlPoints[ i + 2];
        glm::vec3 p3 = ctrlPoints[ i + 3];
        glm::vec3 v = Interpolate(p0, p1, p2, p3, t);
        vbo.AddData(&v, sizeof(glm::vec3));
        vbo.AddData(&texCoord, sizeof(glm::vec2));
        vbo.AddData(&normal, sizeof(glm::vec3));
    }

    // Upload the VBO to the GPU
    vbo.UploadDataToGPU(GL_STATIC_DRAW);
    // Set the vertex attribute locations
    GLsizei stride = 2*sizeof(glm::vec3)+sizeof(glm::vec2);
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    // Normal vectors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

}

// MARK: - Create Centreline
void CCatmullRom::CreateCentreline(const float &pathWidth, const bool &doTunel)
{
    // Call Set Control Points
    SetControlPoints();

    // Call UniformlySampleControlPoints with the number of samples required
    UniformlySampleControlPoints(200);

    //CreateLine(m_vaoCentreline, m_vboCentreline, m_centrelinePoints);

    m_doTunel = doTunel;
    if (m_doTunel){
        CreateTunelOffsetCurves(pathWidth);
    }else{
        CreatePathOffsetCurves(pathWidth);
    }
}


// MARK: - Create Spline
void CCatmullRom::CreateSpline(const std::string &a_sDirectory, const std::string &a_sFilename )
{
    if (m_doTunel == true){
        CreateTunel(a_sDirectory, a_sFilename);
    }else{
        CreatePath(a_sDirectory, a_sFilename);
    }
}

// MARK: - Create Path
void CCatmullRom::CreatePath(const std::string &a_sDirectory, const std::string &a_sFilename)
{

    m_texture.Load(a_sDirectory+a_sFilename);

    m_directory = a_sDirectory;
    m_filename = a_sFilename;

    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);


    // Generate a VAO called m_vaoTrack and a VBO to get the offset curve points and indices on the graphics card
    // Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
    glGenVertexArrays(1, &m_vaoPath);
    glBindVertexArray(m_vaoPath);

    // Create a VBO
    m_vboPath.Create();
    m_vboPath.Bind();



    for (unsigned int i = 0; i < m_pathRenderingPoints.size(); i++) {
        glm::vec2 t(i / (float) m_pathRenderingPoints.size(), i / (float) m_pathRenderingPoints.size());
        glm::vec3 v = m_pathRenderingPoints[i];
        glm::vec3 n = -m_pathRenderingPointsNormal[i];
        m_vboPath.AddData(&v, sizeof(glm::vec3));
        m_vboPath.AddData(&t, sizeof(glm::vec2));
        m_vboPath.AddData(&n, sizeof(glm::vec3));
    }


    //m_vboPath.AddData(&m_pathRenderingPoints, sizeof(glm::vec3));
    // Upload the VBO to the GPU
    m_vboPath.UploadDataToGPU(GL_STATIC_DRAW);

    GLsizei stride = 2*sizeof(glm::vec3)+sizeof(glm::vec2);
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    // Normal vectors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

}


// MARK: - Create Path Offset Curves
void CCatmullRom::CreatePathOffsetCurves(const float & pathWidth)
{
    // Compute the offset curves, one left, and one right.  Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
    m_pathWidth = pathWidth;
    glm::vec3 p, pNext, T, N, B, up;

    for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {

        p = m_centrelinePoints[i];
        pNext = m_centrelinePoints[ (i + 1) % m_centrelinePoints.size() ] ;
        up = m_centrelineUpVectors[i];
        //         cout << "p: " <<  p.x << " " << p.y << " " << p.z << endl;
        //         cout << "pNext: " <<  pNext.x << " " << pNext.y << " " << pNext.z << endl;

        T = normalize(pNext - p);
        //N = cross(T, glm::vec3(0, 1, 0));//change to interpolated up vector
        up = glm::rotate(up, 0.0f, T);
        B = normalize( up);
        N = normalize(cross(T, B));//change to interpolated up vector
        //B = cross(N, T);

        //glm::vec3 viewPoint = p + ( m_pathWidth / 2.0f ) * T;
        glm::vec3 lelfPoint = p - ( m_pathWidth / 2.0f ) * N;
        glm::vec3 rightPoint = p + ( m_pathWidth / 2.0f ) * N;

        //         cout << "lelfPoint: " <<  lelfPoint.x << " " << lelfPoint.y << " " << lelfPoint.z << endl;
        //         cout << "viewPoint: " <<  viewPoint.x << " " << viewPoint.y << " " << viewPoint.z << endl;
        //         cout << "rightPoint: " <<  rightPoint.x << " " << rightPoint.y << " " << rightPoint.z << endl << endl;
        float rotate = m_rotationsOverTime[i];
        m_leftOffsetPoints.push_back(glm::vec3(lelfPoint.x, lelfPoint.y + rotate, lelfPoint.z));
        m_rightOffsetPoints.push_back(glm::vec3(rightPoint.x, rightPoint.y - rotate, rightPoint.z));

        m_pathRenderingPoints.push_back(glm::vec3(lelfPoint.x , lelfPoint.y + rotate, lelfPoint.z ));
        m_pathRenderingPoints.push_back(glm::vec3(rightPoint.x ,rightPoint.y - rotate, rightPoint.z));

        m_pathRenderingPointsNormal.push_back(N);
        m_pathRenderingPointsNormal.push_back(N);

        m_vertexCount += 2;

    }

    m_pathRenderingPoints.push_back(m_leftOffsetPoints[0]);
    m_pathRenderingPoints.push_back(m_rightOffsetPoints[0]);

    m_pathRenderingPointsNormal.push_back(m_pathRenderingPointsNormal[0]);
    m_pathRenderingPointsNormal.push_back(m_pathRenderingPointsNormal[0]);

    m_vertexCount += 2;


    std::cout << "left size: " <<  m_leftOffsetPoints.size() << ", mid size: " << m_centrelinePoints.size() << ", right size:" << m_rightOffsetPoints.size() << std::endl;

    std::cout << "path points size: " <<  m_pathRenderingPoints.size() << ", m_vertexCount: " << m_vertexCount << std::endl;

    //CreateLine(m_vaoLeftOffsetCurve, m_vboLeftOffsetCurve, m_leftOffsetPoints);

    //CreateLine(m_vaoRightOffsetCurve, m_vboRightOffsetCurve, m_rightOffsetPoints);


}

// MARK: - Create Tunel
void CCatmullRom::CreateTunel(const std::string &a_sDirectory, const std::string &a_sFilename)
{

    m_texture.Load(a_sDirectory+a_sFilename);

    m_directory = a_sDirectory;
    m_filename = a_sFilename;

    //    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    //    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);


    // Generate a VAO called m_vaoTrack and a VBO to get the offset curve points and indices on the graphics card
    // Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
    glGenVertexArrays(1, &m_vaoPath);
    glBindVertexArray(m_vaoPath);

    // Create a VBO
    m_vboPath.Create();
    m_vboPath.Bind();
    //t
    for (unsigned int i = 0; i < m_tunelPoints.size(); i++) {
        glm::vec3 v = m_tunelPoints[i];
        glm::vec2 t = m_tunelPointsTextureCoord[i];
        glm::vec3 n = -m_tunelPointsNormal[i];

        m_vboPath.AddData(&v, sizeof(glm::vec3));
        m_vboPath.AddData(&t, sizeof(glm::vec2));
        m_vboPath.AddData(&n, sizeof(glm::vec3));
    }

    // Upload the VBO to the GPU
    m_vboPath.UploadDataToGPU(GL_STATIC_DRAW);

    GLsizei stride = 2*sizeof(glm::vec3)+sizeof(glm::vec2);
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
    // Normal vectors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

    // unbind the Vertex Array Object
    glBindVertexArray(0);

}

// MARK: - Create Tunel Offset Curves
void CCatmullRom::CreateTunelOffsetCurves(const float & pathWidth){

    // Compute the offset curves, one left, and one right.  Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
    m_pathWidth = pathWidth;
    int numberOfSegments = 25;
    std::vector <glm::vec3> tempTunelPoints;
    std::vector <float> tempTunelPointsAngle;
    std::vector <glm::vec3> tempTunelPointsNormals;
    glm::vec3 p, pNext, up;

    for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {

        p = m_centrelinePoints[i];
        pNext = m_centrelinePoints[ (i + 1) % m_centrelinePoints.size() ] ;
        up = m_centrelineUpVectors[i];


        glm::vec3 Tangent = normalize(pNext - p);
        std::vector <glm::vec3> circlePoints;
        std::vector <float> circlePointsAngle;
        std::vector <glm::vec3> circlePointsNormals;
        float angle;

        for (unsigned int ii = 0; ii < numberOfSegments; ii++) {
            float theta = 2.0f *  glm::pi<float>() * (ii) / numberOfSegments;//get the current angle
            angle = (theta * 180 ) / glm::pi<float>();
            //std::cout << "angle: " << angle << endl;

            glm::vec3 upVector = glm::rotate(up, angle, Tangent);
            glm::vec3 BiNormal = normalize( upVector);
            glm::vec3 Normal = normalize(cross(Tangent, BiNormal));//change to interpolated up vector

            glm::vec3 lelfPoint = p - ( m_pathWidth / 2.0f ) * Normal;
            //glm::vec3 rightPoint = p + ( m_pathWidth / 2.0f ) * Normal;

            circlePoints.push_back( lelfPoint);
            //circlePoints.push_back(rightPoint);

            circlePointsAngle.push_back( angle);

            circlePointsNormals.push_back(Normal);


        }
        //circlePoints.push_back( circlePoints[0]);
        //std::cout << "size: " << circlePoints.size() << endl;
        tempTunelPoints.insert(tempTunelPoints.end(), circlePoints.begin(), circlePoints.end());
        tempTunelPointsNormals.insert(tempTunelPointsNormals.end(), circlePointsNormals.begin(), circlePointsNormals.end());
        tempTunelPointsAngle.insert(tempTunelPointsAngle.end(), circlePointsAngle.begin(), circlePointsAngle.end());
    }


    int iterator = 0;
    int numbLoops = tempTunelPoints.size() / numberOfSegments;

    while (iterator < numbLoops){

        for (unsigned int i = 0; i < numberOfSegments; ++i) {

            glm::vec2 top = ((i % 2)==0) ? glm::vec2(1.0f,0.0f) : glm::vec2(1.0f,1.0f) ;
            glm::vec2 bottom = ((i % 2)==0) ? glm::vec2(0.0f,0.0f) : glm::vec2(0.0f,1.0f) ;

            m_tunelPoints.push_back( tempTunelPoints[i + iterator * numberOfSegments] );
            m_tunelPointsNormal.push_back(tempTunelPointsNormals[i + iterator * numberOfSegments] );

            m_tunelPointsTextureCoord.push_back( top);

            if (iterator >= (numbLoops - 1)){
                m_tunelPoints.push_back( tempTunelPoints[i] );
                m_tunelPointsNormal.push_back(tempTunelPointsNormals[i] );
                m_tunelPointsTextureCoord.push_back( bottom );

            }else{
                m_tunelPoints.push_back( tempTunelPoints[(i + numberOfSegments) + iterator * numberOfSegments] );
                m_tunelPointsNormal.push_back(tempTunelPointsNormals[(i + numberOfSegments) + iterator * numberOfSegments] );
                m_tunelPointsTextureCoord.push_back( bottom );

            }
        }
        iterator += 1;
    }

    //adding two more points to close gaps
    m_tunelPoints.push_back(m_tunelPoints[0]);
    m_tunelPointsNormal.push_back(m_tunelPointsNormal[0]);
    m_tunelPointsTextureCoord.push_back(m_tunelPointsTextureCoord[0]);

    m_tunelPoints.push_back(m_tunelPoints[1]);
    m_tunelPointsNormal.push_back(m_tunelPointsNormal[1]);
    m_tunelPointsTextureCoord.push_back(m_tunelPointsTextureCoord[1]);


}

// MARK: - Interpolation
// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, float t)
{
//    float t2 = t * t;
//    float t3 = t2 * t;
//
//	glm::vec3 a = p1;
//	glm::vec3 b = 0.5f * (-p0 + p2);
//	glm::vec3 c = 0.5f * (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3);
//	glm::vec3 d = 0.5f * (-p0 + 3.0f*p1 - 3.0f*p2 + p3);
//    
//	return a + b*t + c*t2 + d*t3;

    
    glm::vec3 a = p1;
    glm::vec3 b = 0.5f * (-p0 + p2);
    glm::vec3 c = 0.5f * ( (2.0f * p0) - (5.0f * p1) + (4.0f * p2) - p3);
    glm::vec3 d = 0.5f * ( -p0 + (3.0f * p1) - (3.0f * p2) + p3);
    
    
    return ( a + (b * t) + (c * (t * t)) + (d * (t * t * t)) );
    
}

// MARK: - Control Points
std::vector<glm::vec3>  CCatmullRom::GetControlPoints() const{
    
    return m_controlPoints; 
}

void CCatmullRom::SetControlPoints()
{
    
	// Set control points (m_controlPoints) here, or load from disk
    
    //1
    m_controlPoints.push_back(glm::vec3(-850, 60, 50));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(0.0f);
    //2
    m_controlPoints.push_back(glm::vec3(-990, 140, -300));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);

    //3
    m_controlPoints.push_back(glm::vec3(-900, 250, -700));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(17.0f);
    //4
    m_controlPoints.push_back(glm::vec3(-700, 330, -1000));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(-22.0f);
    //5
    m_controlPoints.push_back(glm::vec3(-250, 490, -850));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(14.0f);
    //6
    m_controlPoints.push_back(glm::vec3(-220, 460, -600));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);
    //7
    m_controlPoints.push_back(glm::vec3(-350, 400, -350));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(26.0f);
    //8
    m_controlPoints.push_back(glm::vec3(-200, 280, -100));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(19.0f);
    //9
    m_controlPoints.push_back(glm::vec3(120, 360, -200));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(0.0f);
    //10
    m_controlPoints.push_back(glm::vec3(170, 420, -650));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);
    //11
    m_controlPoints.push_back(glm::vec3(350, 570, -850));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(17.0f);
    //12
    m_controlPoints.push_back(glm::vec3(750, 660, -800));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(-22.0f);
    //13
    m_controlPoints.push_back(glm::vec3(900, 520, -400));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(14.0f);
    //14
    m_controlPoints.push_back(glm::vec3(620, 390, -50));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);
    //15
    m_controlPoints.push_back(glm::vec3(450, 340, 200));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(26.0f);
    //16
    m_controlPoints.push_back(glm::vec3(450, 230, 400));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(19.0f);
    //17
    m_controlPoints.push_back(glm::vec3(600, 200, 600));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(0.0f);
    //18
    m_controlPoints.push_back(glm::vec3(580, 160, 800));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);
    //19
    m_controlPoints.push_back(glm::vec3(400, 270, 900));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(17.0f);
    //20
    m_controlPoints.push_back(glm::vec3(250, 320, 850));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(-22.0f);
    //21
    m_controlPoints.push_back(glm::vec3(100, 400, 600));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(14.0f);
    //22
    m_controlPoints.push_back(glm::vec3(-200, 470, 580));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);
    //23
    m_controlPoints.push_back(glm::vec3(-480, 420, 900));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(26.0f);
    //24
    m_controlPoints.push_back(glm::vec3(-700, 340, 900));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(19.0f);
    
    //25
    m_controlPoints.push_back(glm::vec3(-850, 210, 750));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(30.0f);
    //26
    m_controlPoints.push_back(glm::vec3(-900, 120, 450));
    m_controlUpVectors.push_back(glm::normalize(glm::vec3(0, 1, 0)));
    m_rotationsAtControlPoints.push_back(26.0f);

    
}


// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
	int M = (int) m_controlPoints.size();

	float fAccumulatedLength = 0.0f;
	m_distances.push_back(fAccumulatedLength);
	for (int i = 1; i < M; i++) {
		fAccumulatedLength += glm::distance(m_controlPoints[i-1], m_controlPoints[i]);
		m_distances.push_back(fAccumulatedLength);
	}

	// Get the distance from the last point to the first
	fAccumulatedLength += glm::distance(m_controlPoints[M-1], m_controlPoints[0]);
	m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3 &p, glm::vec3 &up, float &rotation)
{
  
	if (d < 0)
		return false;

	int M = (int) m_controlPoints.size();
	if (M == 0)
		return false;


	float fTotalLength = m_distances[m_distances.size() - 1];

	// The the current length along the control polygon; handle the case where we've looped around the track
	float fLength = d - (int) (d / fTotalLength) * fTotalLength;

	// Find the current segment
	int j = -1;
	for (int i = 0; i < (int)m_distances.size(); i++) {
		if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
			j = i; // found it!
			break;
		}
	}

	if (j == -1)
		return false;

	// Interpolate on current segment -- get t
	float fSegmentLength = m_distances[j + 1] - m_distances[j];
	float t = (fLength - m_distances[j]) / fSegmentLength;
	
	// Get the indices of the four points along the control polygon for the current segment
	int iPrev = ((j-1) + M) % M;
	int iCur = j;
	int iNext = (j + 1) % M;
	int iNextNext = (j + 2) % M;

    rotation = m_rotationsOverTime[j];
    
	// Interpolate to get the point (and upvector)
	p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
	if (m_controlUpVectors.size() == m_controlPoints.size())
		up = glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t));


	return true;
}


// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
    glm::vec3 p, up;
    float rot;

    // Compute the lengths of each segment along the control polygon, and the total length
    ComputeLengthsAlongControlPoints();
    float fTotalLength = m_distances[m_distances.size() - 1];

    // The spacing will be based on the control polygon
    float fSpacing = fTotalLength / numSamples;

    int count = numSamples / m_rotationsAtControlPoints.size();
    int it = 0;
    int itCount = 0;


    // Call PointAt to sample the spline, to generate the points
    for (int i = 0; i < numSamples; i++) {

        {
            float currentRotationAngle = m_rotationsAtControlPoints[itCount];
            float nextRotationAngle = (itCount >= m_rotationsAtControlPoints.size() - 1) ? m_rotationsAtControlPoints[0] : m_rotationsAtControlPoints[itCount + 1];

            if (it > count){
                //cout << "currentRotationAngle: " << currentRotationAngle << ", nextRotationAngle: " << nextRotationAngle << endl;
                itCount += 1;
                it = 0;
            }

            float percent = (float(it)/(count)) * 100;
            float rot = (itCount>0 && percent<=0) ? nextRotationAngle : ( ( ((nextRotationAngle - currentRotationAngle) * percent) / 100) + currentRotationAngle);
            //cout << "percent: " << percent << ", rotate value: " << rot << endl;
            m_rotationsOverTime.push_back(rot);
            it += 1;
        }

        Sample(i * fSpacing, p, up, rot);
        m_centrelinePoints.push_back(p);
        if (m_controlUpVectors.size() > 0)
            m_centrelineUpVectors.push_back(up);

    }


    // Repeat once more for truly equidistant points
    m_controlPoints = m_centrelinePoints;
    m_controlUpVectors = m_centrelineUpVectors;
    m_centrelinePoints.clear();
    m_centrelineUpVectors.clear();
    m_distances.clear();
    ComputeLengthsAlongControlPoints();
    fTotalLength = m_distances[m_distances.size() - 1];
    fSpacing = fTotalLength / numSamples;
    for (int i = 0; i < numSamples; i++) {
        Sample(i * fSpacing, p, up, rot);
        m_centrelinePoints.push_back(p);
        if (m_controlUpVectors.size() > 0)
            m_centrelineUpVectors.push_back(up);
    }


}

// MARK: - DrawCircle
std::vector <glm::vec3> CCatmullRom::DrawCircle( const glm::vec3 & first, const glm::vec3 & second, const int & segmentsCount){
    
    //        Using vectors, generally if t is the parameter then and point P on the circle is given by; 
    //        P = R * cos ( t ) * u + R * sin ( t ) * nu + c 
    //        Where u is a unit vector from the centre of the circle to any point on the circumference; R is the radius; n is a unit vector perpendicular to the plane and c is the centre of the circle.
            
    
    glm::vec3 t = second - first;
    float tLength = glm::length(t);
    glm::vec3 u = glm::normalize(t); // tangent unit  vector
    float R = 200.0f;
    glm::vec3 n = t/tLength; //unit normal vector perpendicular
    glm::vec3 c = glm::vec3( -first.x, first.y, -first.z);
    std::vector <glm::vec3> p;
    
    for (int step = 0; step < segmentsCount; ++step)
    {
        float angle = (2.0f * glm::pi<float>() * float(step)) / float(segmentsCount); 
        glm::vec3 P = R * glm::cos(angle) * u + R * glm::sin(angle) * (n * u) + c;
        
        p.push_back(P);
    }
    
    return p;
    
}

// MARK: - Draw Tunel
void CCatmullRom::RenderTunel(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const int & segments){

    std::vector <glm::vec3> ab = DrawCircle(a, b, segments);
    std::vector <glm::vec3> bc = DrawCircle(b, c, segments);
    for (int i = 0; i < segments; ++i)
    {
        m_tunelPoints.push_back( ab[i]);
        m_tunelPoints.push_back( bc[i]);
    }

    m_tunelPoints.push_back(m_tunelPoints[0]);
    m_tunelPoints.push_back(m_tunelPoints[1]);
   
}

// MARK: - Render Line
void CCatmullRom::RenderLine(const GLuint & vao, const std::vector<glm::vec3> & ctrlPoints) {

    // Bind the VAO and render it
    glBindVertexArray(vao);
    m_texture.BindTexture2D();
    glDrawArrays(GL_LINE_STRIP, 0, ctrlPoints.size());
    glDrawArrays(GL_POINTS, 0, ctrlPoints.size());

}

// MARK: - Render Center Line
void CCatmullRom::RenderCentreline()
{
	// Bind the VAO m_vaoCentreline and render it
    RenderLine(m_vaoCentreline, m_centrelinePoints);
    
}

// MARK: - Render Offset Curves
void CCatmullRom::RenderOffsetCurves()
{
    
	// Bind the VAO m_vaoLeftOffsetCurve and render it
    RenderLine(m_vaoLeftOffsetCurve, m_leftOffsetPoints);
    
	// Bind the VAO m_vaoRightOffsetCurve and render it
    RenderLine(m_vaoRightOffsetCurve, m_rightOffsetPoints);
}

// MARK: - Draw Path
void CCatmullRom::RenderPath()
{
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    
	// Bind the VAO m_vaoTrack and render it
    glBindVertexArray(m_vaoPath);
    m_texture.BindTexture2D();
    
    if (m_doTunel){
        glDrawArrays( GL_TRIANGLE_STRIP, 0, m_tunelPoints.size() );
    }else{
        glDrawArrays( GL_TRIANGLE_STRIP, 0, m_vertexCount );
    }
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
}

// MARK: - Get Lap
int CCatmullRom::CurrentLap(float d)
{
	return (int)(d / m_distances.back());
}

// MARK: - Release
// Release memory on the GPU 
void CCatmullRom::Release()
{
    m_texture.Release();
    glDeleteVertexArrays(1, &m_vaoPath);
    m_vboPath.Release();
}

