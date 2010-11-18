
#include "entity.h"

using namespace gamefw;

Entity::Entity()
:
m_name(new string("")), m_desc(new string("")),
m_position(glm::vec3(0.0f, 0.0f, 0.0f)),
m_velocity_local(glm::vec3(0.0f, 0.0f, 0.0f))
{
	m_orientation.yaw = 0.0f;
	m_orientation.pitch = 0.0f;
	m_orientation.roll = 0.0f;
}

Entity::Entity(const gamefw::Entity& entity)
:
m_name(entity.getName()),
m_desc(entity.getDesc()),
m_position(entity.m_position),
m_orientation(entity.m_orientation),
m_velocity_local(entity.m_velocity_local),
m_renderjob(entity.m_renderjob)
{
}


Entity::~Entity()
{
}

// Entity::Entity(const gamefw::Entity& entity)
// {
// }

shared_ptr<string> Entity::getDesc() const
{
    return m_desc;
}

shared_ptr<string> Entity::getName() const
{
    return m_name;
}

void Entity::setRenderJob(shared_ptr< RenderJob > renderjob)
{
    m_renderjob = renderjob;
}

shared_ptr< RenderJob > Entity::getRenderJob() const
{
    return m_renderjob;
}

void Entity::setDesc(const char* desc)
{
    shared_ptr<string> temp(new string(desc));
    m_desc.swap(temp);
}

void Entity::setName(const char* name)
{
    shared_ptr<string> temp(new string(name));
    m_name.swap(temp);
}






