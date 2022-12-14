#include "Camera.h"

#include <cmath>
#include <iostream>

#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

namespace rasty {

/**
 * Camera Constructor/Destructor
 * Sets up and destroys ospray camera object
*/
Camera::Camera(int width, int height) :
    imageWidth(width), imageHeight(height), xPos(0.0), yPos(0.0), zPos(0.0),
    viewX(0.0), viewY(0.0), viewZ(0.0), orbitRadius(0.0), 
    upX(0.0), upY(1.0), upZ(0.0)
{   
    this->transform = rkcommon::math::affine3f::translate(rkcommon::math::vec3f(0.0, 0.0, 0.0));
    this->ID = createID();
    this->oCamera = ospNewCamera("perspective");
    this->updateOSPRayPosition();
    ospSetFloat(this->oCamera, "aspect", (float)this->imageWidth/imageHeight);
    ospCommit(this->oCamera);
}

Camera::~Camera()
{
    ospRelease(this->oCamera);
}


void Camera::setOrbitRadius(float radius)
{
    //for use with paths
    this->orbitRadius = radius;
    this->updateOSPRayPosition();
}

void Camera::setUpVector(float x, float y, float z)
{
    this->upX = x;
    this->upY = y;
    this->upZ = z;    
    this->updateOSPRayPosition();
}

void Camera::setPosition(float x, float y, float z)
{
    this->xPos = x;
    this->yPos = y;
    this->zPos = z;
    this->updateOSPRayPosition();
}

void Camera::setView(float x, float y, float z)
{
    this->viewX = x;
    this->viewY = y;
    this->viewZ = z;
    this->updateOSPRayPosition();
}

void Camera::setTransform(rkcommon::math::affine3f transform)
{
    this->transform = transform;
    this->updateOSPRayPosition();
}

void Camera::centerView()
{
    //center the camera's viewpoint on the center of a volume
    //std::vector<int> bounds = v->getBounds();
    this->setView(-this->xPos, -this->yPos, -this->zPos);
}

void Camera::setImageSize(int width, int height)
{
    this->imageWidth  = width;
    this->imageHeight = height;
}

int Camera::getImageWidth()
{
    return this->imageWidth;
}

int Camera::getImageHeight()
{
    return this->imageHeight;
}


/**
 * updateOSPRayPosition()
 * Updates the position of the OSPRay camera with the current values
*/
void Camera::updateOSPRayPosition()
{
    // //std::cout<< "[Camera] updateOSPRayPosition" << std::endl;
    //update OSPRay camera
    float position[] = {this->xPos, this->yPos, this->zPos};
    ospSetParam(this->oCamera, "position", OSP_VEC3F, position);

    float direction[] = {this->viewX, this->viewY, this->viewZ};
    ospSetParam(this->oCamera, "direction", OSP_VEC3F, direction);

    float up[] = {this->upX, this->upY, this->upZ};
    ospSetParam(this->oCamera, "up", OSP_VEC3F, up);

    ospSetParam(this->oCamera, "transform", OSP_AFFINE3F, this->transform);
    ospCommit(this->oCamera);
}

/**
 * setRegion()
 * Sets the region of the image to render
*/
void Camera::setRegion(float top, float right, float bottom, float left)
{
    // render only a defined region of the current view (set clip space)
    // to be used with tiles
    // bottom left of the full image is [0, 0]
    // top right of the full image is [1, 1]
    // e.g. the upper left quadrant of the image would be defined with
    // setRegion(1, 0.5, 0.5, 0)
    float upperRight[] = {right, top};
    float lowerLeft[] = {left, bottom};
    ospSetParam(this->oCamera, "imageStart", OSP_VEC2F, lowerLeft);
    ospSetParam(this->oCamera, "imageEnd", OSP_VEC2F, upperRight);
}

/**
 * asOSPRayObject()
 * Returns the OSPRay camera object
*/
OSPCamera Camera::asOSPRayObject()
{
    return this->oCamera;
}

}
