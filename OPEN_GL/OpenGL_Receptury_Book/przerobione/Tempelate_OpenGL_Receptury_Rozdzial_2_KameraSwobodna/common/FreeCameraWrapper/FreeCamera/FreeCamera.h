#pragma once
#include "AbstractCamera.h"

class CFreeCamera :
    public CAbstractCamera
{
public:
    CFreeCamera(void);
    ~CFreeCamera(void);

    void Update();

    void Walk(const float dt);
    void Strafe(const float dt);
    void Lift(const float dt);

    void SetTranslation(const glm::vec3& t);
    glm::vec3 GetTranslation() const;

    void SetSpeed(const float speed);
    const float GetSpeed() const;

protected:

    float speed;			//szybkoć ruchu kamery w m/s
    glm::vec3 translation;
};
