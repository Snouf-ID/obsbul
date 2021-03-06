
#include "../common.h"
#include "gamefw.h"

#include <glm/gtx/projection.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace gamefw;

const int POINTLIGHTS_IDX = 0;

Renderer::Renderer(const GLuint display_width, const GLuint display_height,
                   OpenGLVersion opengl_version)
:
m_display_width(display_width),
m_display_height(display_height),
m_camera(new Entity),
m_aspect_ratio((float) display_width / (float) display_height),
m_opengl_version(opengl_version)
{
    // Don't write to zbuffer for transparent objects.
    glAlphaFunc (GL_GREATER, 0.1) ;
    glEnable (GL_ALPHA_TEST) ;
    glClearColor(0.0,0.0,0.0,0.0);
    m_camera->setName("Camera");
    if (m_opengl_version == OGL_3_3) {
        initBuffers(display_width, display_height);
    }
}

Renderer::~Renderer()
{
    if (m_opengl_version == OGL_3_3) {
        glDeleteRenderbuffers(1, &m_depth_stencil_buffers.gbuffer);
        glDeleteRenderbuffers(1, &m_depth_stencil_buffers.pbuffer);
        glDeleteFramebuffers(1, &m_fbo.gbuffer);
        glDeleteFramebuffers(1, &m_fbo.pbuffer);
        glDeleteFramebuffers(1, &m_fbo.ppbuffer);

        // Delete manually allocated textures.
        shared_ptr<RenderJob> gbuffer_renderjob = m_gbuffer.getRenderJob();
        glDeleteTextures(gbuffer_renderjob->m_num_textures,
                        gbuffer_renderjob->m_textures);
        delete [] gbuffer_renderjob->m_textures;
        gbuffer_renderjob->m_num_textures = 0;

        shared_ptr<RenderJob> pbuffer_renderjob = m_pbuffer.getRenderJob();
        glDeleteTextures(pbuffer_renderjob->m_num_textures,
                        pbuffer_renderjob->m_textures);
        delete [] pbuffer_renderjob->m_textures;
        pbuffer_renderjob->m_num_textures = 0;

        shared_ptr<RenderJob> ppbuffer_renderjob = m_ppbuffer.getRenderJob();
        // Shared textures with gbuffer.
        ppbuffer_renderjob->m_num_textures = 0;
    }
}

void Renderer::createDepthStencilBuffer(GLuint* buffer, const GLuint width,
                                        const GLuint height)
{
    glGenRenderbuffers(1, buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_buffers.gbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_depth_stencil_buffers.gbuffer);
}

void Renderer::texParametersForRenderTargets() const
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
}
bool Renderer::checkFramebuffer() const
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    string error = "Framebuffer object error: ";

    switch (status)
    {
    case GL_FRAMEBUFFER_UNDEFINED:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_UNDEFINED";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_UNSUPPORTED";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        LOG(logERROR) << error << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;
    }

    return status == GL_FRAMEBUFFER_COMPLETE;
}

void Renderer::createTexturesForFBO(shared_ptr<RenderJob> renderjob,
                                    const GLuint num_textures,
                                    const GLenum internalformats[],
                                    const GLuint size_divisor[],
                                    const GLenum types[])
{
    renderjob->m_textures = new GLuint[num_textures];
    renderjob->m_num_textures = num_textures;
    glGenTextures(num_textures, renderjob->m_textures);

    for (uint i = 0; i < num_textures; i++) {
        glBindTexture(GL_TEXTURE_2D, renderjob->m_textures[i]);

        texParametersForRenderTargets();

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internalformats[i],
            m_display_width / size_divisor[i],
            m_display_height / size_divisor[i],
            0,
            GL_RGB,
            types[i],
            0
        );

        GLint attachment = GL_COLOR_ATTACHMENT0 + i;

        glFramebufferTexture(GL_FRAMEBUFFER, attachment,
                             renderjob->m_textures[i], 0);
    }
}

void Renderer::initBuffers(const GLuint width, const GLuint height)
{
    GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                             GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                             GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
                             GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};
    
    // GBUFFER STAGE

    {
        int num_textures = 5;
        
        m_gbuffer = *Locator::getFileService().createEntity("gbuffer");
        glGenFramebuffers(1, &m_fbo.gbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.gbuffer);

        const GLenum internalformats[] = {
            GL_RGB8,
            GL_RGB8,
            GL_RGB16F,
            GL_RGB32F,
            GL_RGB8
        };
        const GLuint size_divisor[] = {1, 1, 1, 1, 1};
        const GLenum types[] = {
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_FLOAT,
            GL_FLOAT,
            GL_UNSIGNED_BYTE
        };
        
        glActiveTexture(GL_TEXTURE0);
        shared_ptr<RenderJob> gbuffer_renderjob = m_gbuffer.getRenderJob();
        
        createTexturesForFBO(gbuffer_renderjob, num_textures, internalformats,
                             size_divisor, types);

        createDepthStencilBuffer(&m_depth_stencil_buffers.gbuffer, width, height);

        bool status = checkFramebuffer();
        assert(status);
        GLuint gbuffer_program = gbuffer_renderjob->getShaderProgramID();
        
        // Bind uniform block for pointlights.
        GLuint material_location = glGetUniformBlockIndex(gbuffer_program,
                                                          "pointlights");
        assert(material_location != GL_INVALID_INDEX);
        glGenBuffers(1, &m_uniform_blocks.pointlights);

        // Attach the UBO to pointlights index.
        glBindBufferBase(GL_UNIFORM_BUFFER, POINTLIGHTS_IDX,
                         m_uniform_blocks.pointlights);
        // Associate the block in the GLSL source to this index.
        glUniformBlockBinding(gbuffer_program, material_location, POINTLIGHTS_IDX);
        
        // Assign draw buffers.
        glDrawBuffers(num_textures, draw_buffers);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // PBUFFER STAGE.
    {
        int num_textures = 4;
        
        m_pbuffer = *Locator::getFileService().createEntity("pbuffer");

        glGenFramebuffers(1, &m_fbo.pbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.pbuffer);

        glActiveTexture(GL_TEXTURE0);
        shared_ptr<RenderJob> pbuffer_renderjob = m_pbuffer.getRenderJob();

        const GLenum internalformats[] = {
            GL_RGB8,
            GL_RGB8,
            GL_RGB8,
            GL_RGB8
        };
        const GLuint size_divisors[] = {1, 1, 1, 1};
        const GLenum types[] = {
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_BYTE
        };

        createTexturesForFBO(pbuffer_renderjob, num_textures, internalformats,
            size_divisors, types
        );
    
        createDepthStencilBuffer(&m_depth_stencil_buffers.pbuffer, width, height);

        bool status = checkFramebuffer();
        assert(status);
        
        // Assign draw buffers.
        glDrawBuffers(num_textures, draw_buffers);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    // POSTPROCESSING STAGE
    {
        int num_textures = 2;
        m_ppbuffer = *Locator::getFileService().createEntity("ppbuffer");
        shared_ptr<RenderJob> ppbuffer_renderjob = m_ppbuffer.getRenderJob();

        // Use gbuffers diffuse and specular textures.
        m_fbo.ppbuffer = m_fbo.gbuffer;
        ppbuffer_renderjob->m_textures = m_gbuffer.getRenderJob()->m_textures;
        ppbuffer_renderjob->m_num_textures = num_textures;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkOpenGLError();
}

void Renderer::render()
{
    if (m_opengl_version == OGL_3_3) {
        glEnable(GL_DEPTH_TEST);

        renderGBuffers();

        glDisable(GL_DEPTH_TEST);

        renderPBuffers();

        renderPPBuffers();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind main window's framebuffer.
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        renderEntity(m_ppbuffer);
    } else {
        renderRenderQueue();
    }
}

void Renderer::addToRenderQueue(shared_ptr<Entity> entity)
{
    m_render_queue.push(entity);
}

void Renderer::addToPointLightQueue(shared_ptr< PointLight > pointlight)
{
    m_pointlight_queue.push(pointlight);
}

void Renderer::renderEntity(const Entity& entity)
{
    shared_ptr<RenderJob> renderjob = entity.getRenderJob();
    GLuint program_id = renderjob->getShaderProgramID();
    glUseProgram(program_id);

    // Load textures.
    for (uint i = 0; i < renderjob->m_num_textures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, renderjob->m_textures[i]);
        string uniform_name("texture");
        uniform_name += (char) '0' + i;
        GLint location = glGetUniformLocation(program_id, uniform_name.c_str());
        glUniform1i(location, i);
    }


    // Calculate and bind mvp.

    // Model orientation ...
   glm::mat4 model(glm::yawPitchRoll(entity.m_orientation.x,
                                     entity.m_orientation.y,
                                     entity.m_orientation.z));
    // ... + translation
    model[3] = glm::vec4(entity.m_position, 1.0);

    // Normal transform.
    glm::mat4 normalmatrix = glm::transpose(glm::inverse(model));

    // View transform.
    glm::mat4 view_orientation_x(glm::rotate(glm::mat4(1.0f),
                                             m_camera->m_orientation.y,
                                             glm::vec3(-1.0f, 0.0f, 0.0f)));
    glm::mat4 view_orientation(glm::rotate(view_orientation_x,
                               m_camera->m_orientation.x,
                               glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 view(glm::translate(view_orientation, -m_camera->m_position));

    // Projection transform
    const GLfloat FOV = 60.0f;
    const GLfloat near_z = 1.0f;
    const GLfloat far_z = 1000.f;
    glm::mat4 projection = glm::perspective(FOV, m_aspect_ratio, near_z, far_z);

    glm::mat4 mvp = projection * view * model;

    // Bind the matrices to uniforms.
    GLint location_mvp = glGetUniformLocation(program_id, "mvp");
    glUniformMatrix4fv(location_mvp, 1, GL_FALSE, &mvp[0][0]);
    GLint location_model = glGetUniformLocation(program_id, "model");
    glUniformMatrix4fv(location_model, 1, GL_FALSE, &model[0][0]);
    GLint location_normalmatrix = glGetUniformLocation(program_id, "normalmatrix");
    glUniformMatrix4fv(location_normalmatrix, 1, GL_FALSE, &normalmatrix[0][0]);
    glUniform3fv(glGetUniformLocation(program_id, "viewer_position"),
                 1, &m_camera->m_position[0]);

    // Bind display height and width uniforms.
    GLint location_width = glGetUniformLocation(program_id, "display_width");
    glUniform1f(location_width, (GLfloat) m_display_width);
    GLint location_height = glGetUniformLocation(program_id, "display_height");
    glUniform1f(location_height, (GLfloat) m_display_height);
    GLint location_near_z = glGetUniformLocation(program_id, "near_z");
    glUniform1f(location_near_z, (GLfloat) near_z);
    GLint location_far_z = glGetUniformLocation(program_id, "far_z");
    glUniform1f(location_far_z, (GLfloat) far_z);

    glBindVertexArray(renderjob->m_buffer_objects.vao);
    
    // Bind material uniform block.
    if (m_opengl_version == OGL_3_3 && renderjob->m_uniforms.materials != 0) {
        glBindBufferBase(GL_UNIFORM_BUFFER, renderjob_enums::MATERIAL,
                         renderjob->m_uniforms.materials);
        glEnableVertexAttribArray(renderjob_enums::MATERIAL_IDX);
    }

    glEnableVertexAttribArray(renderjob_enums::POSITION);
    glEnableVertexAttribArray(renderjob_enums::NORMAL);
    glEnableVertexAttribArray(renderjob_enums::TEXCOORD);
    glDrawElements(GL_TRIANGLES, renderjob->m_vertex_count, GL_UNSIGNED_SHORT, 0);
    // Cleanup.
    glDisableVertexAttribArray(renderjob_enums::POSITION);
    glDisableVertexAttribArray(renderjob_enums::NORMAL);
    glDisableVertexAttribArray(renderjob_enums::TEXCOORD);

    if (m_opengl_version == OGL_3_3) {
        glBindBufferBase(GL_UNIFORM_BUFFER, renderjob_enums::MATERIAL, 0);
        glDisableVertexAttribArray(renderjob_enums::MATERIAL_IDX);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::renderGBuffers()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo.gbuffer);
    renderRenderQueue();
}

void Renderer::renderRenderQueue()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    while (!m_render_queue.empty()) {
        shared_ptr<Entity> current_entity = m_render_queue.front();
        m_render_queue.pop();
        renderEntity(*current_entity);
    } 
}

uint Renderer::loadLightsIntoUniformBlocks()
{
    uint num_pointlights = 0;
    vector<GLfloat> pointlight_buffer;
    while (!m_pointlight_queue.empty()) {
        shared_ptr<PointLight> pointlight = m_pointlight_queue.front();
        m_pointlight_queue.pop();
        pointlight_buffer.push_back(pointlight->m_position.x);
        pointlight_buffer.push_back(pointlight->m_position.y);
        pointlight_buffer.push_back(pointlight->m_position.z);
        pointlight_buffer.push_back(0.0f); // padding.
        pointlight_buffer.push_back(pointlight->m_color.r);
        pointlight_buffer.push_back(pointlight->m_color.g);
        pointlight_buffer.push_back(pointlight->m_color.b);
        pointlight_buffer.push_back(pointlight->m_intensity);
        num_pointlights++;
    }
    const int POINTLIGHTS_SIZE = sizeof(GLfloat) * 8;
    
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_blocks.pointlights);
    glBufferData(GL_UNIFORM_BUFFER, POINTLIGHTS_SIZE * pointlight_buffer.size(),
                 &pointlight_buffer[0], GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, POINTLIGHTS_IDX,
                     m_uniform_blocks.pointlights);
    return num_pointlights;
}


void Renderer::renderPBuffers()
{
    uint num_pointlights = loadLightsIntoUniformBlocks();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo.pbuffer);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    GLuint program_id = m_gbuffer.getRenderJob()->getShaderProgramID();
    glUseProgram(program_id);
    glUniform1i(glGetUniformLocation(program_id, "num_pointlights"),
                num_pointlights);
    renderEntity(m_gbuffer);

}


void gamefw::Renderer::renderPPBuffers()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo.ppbuffer);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    renderEntity(m_pbuffer);
}

void gamefw::Renderer::changeCamera(shared_ptr< Entity > camera)
{
    m_camera = camera;
}
