#ifndef ENTITY_H
#define ENTITY_H

#include "../common.h"

namespace gamefw {

class RenderJob;

/**
 * @brief Game object.
 */
class Entity
{
public:
    Entity();

    virtual ~Entity();
     
    /// Getters and Setters
    shared_ptr<string> getName() const;
    void setName(const char* name);
    
    shared_ptr<string> getDesc() const;
    void setDesc(const char* desc);

    shared_ptr<RenderJob> getRenderJob() const;
    void setRenderJob(shared_ptr<RenderJob> renderjob);

    /**
     * @brief World space position.
     **/
    glm::vec3 m_position;
    
    /**
     * @brief World space orientation in degrees.
     *
     * XYZ is yaw , pitch and roll respectively.
     **/
    glm::vec3 m_orientation;

    /**
     * @brief Local space velocity.
     **/
    glm::vec3 m_velocity_local;

    /**
     * @brief World space angular velocity
     */
    glm::vec3 m_angular_velocity;

private:
    shared_ptr<string> m_name;
    shared_ptr<string> m_desc;
    shared_ptr<RenderJob> m_renderjob;
};

}

#endif // ENTITY_H
