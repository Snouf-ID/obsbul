
#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include "../common.h"
#include "../util/objfile.h"

#include "entity.h"
#include "openglversion.h"

typedef struct _vertex t_vertex;

namespace gamefw {

/**
 * @brief Thrown when EntityFactory fails in the creation of an Entity.
 */
class EntityCreationError: public exception
{
    /**
     * @return Description of exception.
     */
    virtual const char* what() const throw();
};

/**
 * @brief Takes care of Entity creation.
 *
 * Is used inside FileService to serve entities to the GameWorld. Parses a xml-
 * file and initializes an Entity according to it.
 */
class EntityFactory
{
public:
    /**
     * @brief Creates EntityFactory.
     *
     * @param opengl_version OpenGL version to be used for rendering.
     **/
    EntityFactory(const OpenGLVersion opengl_version = OGL_3_3);
    
    
    /**
     * Constructs an Entity from it's file. The format is simple XML.
     *
     * @throw EntityCreationError When the creation fails.
     *
     * @param path The absolute path to the Entity's file.
     * @return Reference to the constructed Entity.
     */
    shared_ptr<Entity> createEntity(const std::string& path);

private:
    void loadModel(const ObjFile& model, shared_ptr<RenderJob> renderjob);
    
    void genVertexBuffers(shared_ptr<RenderJob> renderjob,
            const t_vertex* vertex_buffer, size_t vertex_buffer_length,
            const GLushort* element_buffer, size_t element_buffer_length) const;

    void createMaterials(shared_ptr<RenderJob> renderjob, const ObjFile& model) const;

    const std::string makeDefineFromEnum(const char* enum_name, int index) const;
    
    const OpenGLVersion m_opengl_version;
};

}

#endif // ENTITYFACTORY_H
