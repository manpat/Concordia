#ifndef CAMERA
#define CAMERA

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace BlueBear {
  namespace Graphics {
    class Shader;

    class Camera {

      private:
        glm::mat4 view;
        glm::mat4 projection;

        GLfloat cameraHeight = 3.0f;
        glm::vec3 lookingAt = glm::vec3( 0.0f, 0.0f, -9.0f );
        glm::vec3 camera = glm::vec3( 0, 0, -10.0f );
        glm::vec3 originalDirection = glm::vec3( glm::normalize( lookingAt - camera ) );
        glm::vec3 up = glm::vec3( 0.0f, 0.0f, 1.0f );
        GLfloat yaw = 0.0f;
        GLfloat pitch = 0.0f;
        GLuint currentRotation = 0;

        float perspectiveAspectRatio;
        float widthHalf;
        float heightHalf;
        float zoom = 1.0f;
        const float zoomIncrement = 0.25f;
        bool dirty = true;

        GLfloat orthoRotationAngle = 45.0f;

        void doRotate();

      public:
        glm::vec3 direction = originalDirection;
        bool ortho = true;
        Camera( int screenWidth, int screenHeight );
        void move( GLfloat x, GLfloat y, GLfloat z );
        bool setOrthographic( bool flag );
        float zoomIn();
        float zoomOut();
        float setZoom( float zoomSetting );
        void position();
        void sendToShader();
        glm::mat4 getOrthoView();
        glm::mat4 getOrthoMatrix();
        void walkForward();
        void walkBackward();
        void updateFirstPersonView( int deltaX, int deltaY );
        unsigned int rotateRight();
        unsigned int rotateLeft();
        GLuint getCurrentRotation();
        void setRotationDirect( GLuint rotation );
        std::string positionToString();
        std::string directionToString();
    };
  }
}

#endif
