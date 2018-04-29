#pragma once

#include "Common.h"

class CCamera {
public:
	//CCamera();										// Constructor - sets default values for camera position, viewvector, upvector, and speed
	~CCamera();										// Destructor

    CCamera(const glm::vec3 &position,
            const glm::vec3 &worldUp,
            const GLfloat &pitch,
            const GLfloat &yaw,
            const GLfloat &fieldOfView,
            const GLfloat &width,
            const GLfloat &height,
            const GLfloat &zNear,
            const GLfloat &zFar,
            const GLfloat &speed,
            const GLfloat &speedRatio,
            const GLfloat &sensitivity);

	glm::vec3 GetPosition() const;					// Gets the position of the camera centre of projection
	glm::vec3 GetView() const;						// Gets the position of the camera view point
	glm::vec3 GetStrafeVector() const;				// Gets the camera strafe vector
    glm::mat4 GetViewProjection() const;
	glm::mat4* GetPerspectiveProjectionMatrix();	// Gets the camera perspective projection matrix
	glm::mat4* GetOrthographicProjectionMatrix();	// Gets the camera orthographic projection matrix
	glm::mat4 GetViewMatrix();						// Gets the camera view matrix - note this is not stored in the class but returned using glm::lookAt() in GetViewMatrix()

    glm::vec3 GetLeft();                               // Gets the camera left vector
    glm::vec3 GetRight();                              // Gets the camera right vector
    glm::vec3 GetUp();                                 // Gets the camera up vector
    glm::vec3 GetDown();                               // Gets the camera down vector
    glm::vec3 GetForward();                            // Gets the camera forward vector
    glm::vec3 GetBackward();                           // Gets the camera backward vector
    glm::vec3 GetVelocity();                           // Gets the camera velocity vector

    // re-computer all the vector
    void UpdateCameraVectors( );

	// Set the camera position, viewpoint, and up vector
	void Set(glm::vec3 &position, glm::vec3 &viewpoint, glm::vec3 &upVector);
	
	// Rotate the camera viewpoint -- this effectively rotates the camera
	void RotateViewPoint(float angle, glm::vec3 &viewPoint);

    // Rotate the camera around a viewpoint
    void RotateAroundPoint(const float &distance, const glm::vec3 &viewpoint, const float &angle, const float &y);

    // get a position in front of camera using a ofset distance
    glm::vec3 PositionInFrontOfCamera( const GLfloat &distance);

	// Respond to mouse movement to rotate the camera
    void SetViewByMouse(GLFWwindow *window, const bool &enableMouse = true);
    void SetViewByMouse(const GLfloat &mouseX, const GLfloat &mouseY, const GLboolean &constrainPitch = true, const bool &enableMouse = true);

	// Respond to keyboard presses on arrow keys to translate the camera
    void TranslateByKeyboard(const double &dt, const int &keyPressed);

	// Strafe the camera (move it side to side)
	void Strafe(double direction);

	// Advance the camera (move it forward or backward)
	void Advance(double direction);

	// Update the camera
	void Update(GLFWwindow *window, const double &dt, const int &key, const bool &moveCamera, const bool &enableMouse);

	// Set the projection matrices
    void SetPerspectiveProjectionMatrix(const GLfloat &fieldOfView, const GLfloat &aspectRatio, const GLfloat &nearClippingPlane, const GLfloat &farClippingPlane);
    void SetOrthographicProjectionMatrix(const GLfloat &width, const GLfloat height, const GLfloat &zNear, const GLfloat &zFar);
    void SetOrthographicProjectionMatrix(float value , float zNear, float zFar);
    void SetOrthographicProjectionMatrix(int width, int height);

	glm::mat3 ComputeNormalMatrix(const glm::mat4 &modelViewMatrix);

    glm::vec3 GetPathViewPoint(const glm::vec3 &p, const glm::vec3 &pNext, const float &offSet);
    glm::vec3 GetPathLeftPoints(const glm::vec3 &p, const float &pathWidth, const float &rotation);
    glm::vec3 GetPathRightPoints(const glm::vec3 &p, const float &pathWidth, const float &rotation);

    glm::vec3 GetPathT() const { return m_T; };
    glm::vec3 GetPathN() const { return m_N; };
    glm::vec3 GetPathB() const { return m_B; };

private:
    //view and projection matrix
    glm::mat4 m_perspectiveProjectionMatrix;        // Perspective projection matrix
    glm::mat4 m_orthographicProjectionMatrix;        // Orthographic projection matrix
    glm::mat4 m_viewMatrix;

    // Camera Attributes
    glm::vec3 m_position;            // The position of the camera's centre of projection
    glm::vec3 m_previousPosition;    // The previous position of the camera's centre of projection
    glm::vec3 m_view;                // The camera's viewpoint (point where the camera is looking)

    glm::vec3 m_strafeVector;        // The camera's strafe vector

    glm::vec3 m_front;               // The camera's forward vector
    glm::vec3 m_back;                // The camera's backward vector
    glm::vec3 m_left;                // The camera's left vector
    glm::vec3 m_right;               // The camera's right vector
    glm::vec3 m_up;                  // The camera's up vector
    glm::vec3 m_down;                // The camera's down vector
    glm::vec3 m_velocity;            // The camera's velocity vector

    glm::vec3 m_worldUp;             // The worlds up vector, the original position of the world

    // Eular Angles
    GLfloat m_fieldOfView;           // The view from the camera
    GLfloat m_yaw;
    GLfloat m_pitch;
    GLfloat m_horizontalAngle;       // horizontal angle : toward -Z
    GLfloat m_verticalAngle;         // vertical angle : 0, toward -Y

    // Camera options
    GLfloat m_movementSpeed;         // How fast the camera moves
    GLfloat m_speedRatio;            // How much the speed increases
    GLfloat m_mouseSensitivity;      // How sensitive mouse is

    // Screen
    GLfloat m_screenWidth;           // the width size of the screen window
    GLfloat m_screenHeight;          // the height size of the screen window
    GLfloat m_screenRatio;           // the width/height of the screen window

    // Mouse
    GLboolean m_firstMouse = true;

    // Spline
    glm::vec3 m_T;                   // tangent on the spline
    glm::vec3 m_N;                   // normal on the spline
    glm::vec3 m_B;                   // bitangent on the spline

};
