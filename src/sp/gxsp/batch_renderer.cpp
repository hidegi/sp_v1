#include <sp/gxsp/batch_renderer.h>
#include <sp/gxsp/transformable.h>
#include <sp/spgl.h>
#include <atomic>
#include <algorithm>
#include <queue>

namespace sp
{
    namespace
    {
        SPuint32 translateBlendFactor(Blending::SP_Factor factor)
        {
            switch(factor)
            {
                case Blending::Zero:                    return GL_ZERO;
                case Blending::One:                     return GL_ONE;
                case Blending::SrcColor:                return GL_SRC_COLOR;
                case Blending::OneMinusSrcColor:        return GL_ONE_MINUS_SRC_COLOR;
                case Blending::DstColor:                return GL_DST_COLOR;
                case Blending::OneMinusDstColor:        return GL_ONE_MINUS_DST_COLOR;
                case Blending::SrcAlpha:                return GL_SRC_ALPHA;
                case Blending::OneMinusSrcAlpha:        return GL_ONE_MINUS_SRC_ALPHA;
                case Blending::DstAlpha:                return GL_DST_ALPHA;
                case Blending::OneMinusDstAlpha:        return GL_ONE_MINUS_DST_ALPHA;
            }

            return GL_ZERO;
        }

        SPuint32 translateBlendEquation(Blending::SP_Equation eq)
        {
            switch(eq)
            {
                case Blending::Add:             return GL_FUNC_ADD_EXT;
                case Blending::Subtract:        return GL_FUNC_SUBTRACT_EXT;
                case Blending::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT_EXT;
            }

            return GL_FUNC_ADD_EXT;
        }
    }

    ///SHARED_PTR<DrawableStates>!!!!!
    struct Meta
    {
        long int                id;
        unsigned int            first_index;

        //external entries and counts..
        /*
        size_t                  client_vertex_entry;
        size_t                  client_index_entry;
        size_t                  client_vertex_count;
        size_t                  client_index_count;
        */

        //internal entries and counts..
        size_t                  vertex_entry;
        size_t                  index_entry;
        size_t                  index_count;
        size_t                  vertex_count;

        int                     zorder;
        int                     primitive;

        States                  states;

        //compare reference to update drawable states..
        std::weak_ptr<Drawable::DrawableStates>   drawable;
        bool                            toggle;
    };

    struct Batch
    {
        unsigned int    index_start;
        unsigned int    index_count;
        States          states;
    };

    Renderer::Renderer() :
        m_size          {0, 0},
        m_default_view  {},
        m_vertex_count  {0},
        m_index_count   {0},
        m_max_index     {0},
        m_max_zorder    {0},
        m_particle_count{0},
        m_index_resize  {true},
        m_index_refresh_count{1},
        m_post_process_shader{nullptr}
    {
        //createID();
    }

    Renderer::Renderer(unsigned width, unsigned height) :
        m_size  {width, height},
        m_default_view  {},
        m_vertex_count  {0},
        m_index_count   {0},
        m_max_index     {0},
        m_max_zorder    {0},
        m_particle_count{0},
        m_index_resize  {true},
        m_index_refresh_count{1},
        m_post_process_shader{nullptr}
    {
    }

    Renderer::~Renderer()
    {
        m_drawables.clear();
        m_positions.clear();
        m_colors.clear();
        m_tex_coords.clear();
        m_batches.clear();
    }

    Renderer::Renderer(const vec2u& dim) :
        Renderer(dim.x, dim.y)
    {
    }

    void Renderer::setPostProcessShader(const sp::Shader* shader)
    {
        m_post_process_shader = shader;
    }

    unsigned int Renderer::getFramebufferTexHandleGL() const
    {
        return m_primary_framebuffer.getTexHandleGL();
    }

    void Renderer::setFramebufferPosition(float x, float y)
    {
        //x 2 4 6
        //y 3 7 9

        m_frame_position.x = x;
        m_frame_position.y = y;
    }
    void Renderer::setSurfaceSize(unsigned int width, unsigned int height)
    {
        m_size.x = width;
        m_size.y = height;
        if(!m_primary_framebuffer.create(width, height, true))
            SP_PRINT_WARNING("failed to create framebuffer");
        spCheck(glScissor(0, 0, width, height))
    }

    void Renderer::setSurfaceSize(const vec2u& dim)
    {
        setSurfaceSize(dim.x, dim.y);
    }

    void Renderer::setViewport(const rectf& viewport)
    {
        m_default_view.setViewport(viewport);
        m_view_matrix       = mat(m_default_view.getMatrix());
        m_inv_view_matrix   = mat(!m_view_matrix);
        m_cache.viewport_change = true;
    }

    void Renderer::applyCurrentView()
    {
        //printf("apply current view..\n");
        m_default_view.load();
        m_cache.viewport_change = false;
    }

    void Renderer::applyBlending(const Blending& b)
    {
        if(GL_EXT_blend_func_separate_supported)
        {
            spCheck(glBlendFuncSeparateEXT(
                translateBlendFactor(b.colorSrcFactor),
                translateBlendFactor(b.colorDstFactor),
                translateBlendFactor(b.alphaSrcFactor),
                translateBlendFactor(b.alphaDstFactor)
            ))
        }
        else
        {
            spCheck(glBlendFunc(
                translateBlendFactor(b.colorSrcFactor),
                translateBlendFactor(b.colorDstFactor)
            ))
        }

        if(GL_EXT_blend_minmax_supported && GL_EXT_blend_subtract_supported)
        {
            if(GL_EXT_blend_equation_separate_supported)
            {
                spCheck(glBlendEquationSeparateEXT(
                    translateBlendEquation(b.colorEquation),
                    translateBlendEquation(b.alphaEquation)
                ))
            }
            else
            {
                spCheck(glBlendEquationEXT(translateBlendEquation(b.colorEquation)))
            }
        }
        m_cache.last_blend_mode = b;
    }

    void Renderer::initialize()
    {
        m_default_view.setViewport(
            rectf{
                0.f, 0.f,
                static_cast<float>(m_size.x),
                static_cast<float>(m_size.y),
            });
        m_cache.gl_states_set = false;
        m_cache.viewport_change = true;
    }

    /**
     *  because the fucking client loses a local variable's reference,
     *  drawables must be stored in some persistent wrapper class (the sp, shared_ptr) = ALLOCATION!!
     */

    void Renderer::addDrawable(const Drawable::Ptr primitive, bool set_max)
    {
        if(!primitive || !primitive.get())
            return;

        primitive->addDrawable(*this, set_max);
        //addDrawable(primitive->m_drawable_states, set_max);
    }
    void Renderer::addDrawable(Drawable::DrawableStates::Ptr draw_states, bool set_max)
    {
        if(!draw_states)
        {
            return;
        }
        Drawable* primitive = draw_states->client;

        size_t total_vertex_count  = primitive->m_vertices.size();
        size_t total_index_count   = primitive->m_indices.size();
        size_t client_vertex_entry = draw_states->vertex_entry;
        size_t client_index_entry  = draw_states->index_entry;
        size_t client_vertex_count = draw_states->vertex_count;
        size_t client_index_count  = draw_states->index_count;


        if((client_vertex_entry + client_vertex_count) > total_vertex_count)
        {
            SP_PRINT_WARNING("vertex entry mismatch, vertex entry: " << client_vertex_entry << ", sub-vertex count: " << client_vertex_count <<
                             ", total vertex count: " << total_vertex_count);
            return;
        }

        if((client_index_entry + client_index_count) > total_index_count)
        {
            SP_PRINT_WARNING("vertex entry mismatch, index entry: " << client_index_entry << ", sub-index count: " << client_index_count <<
                             ", total index count: " << total_index_count);
            return;
        }

        static auto sort_lmbd = SP_LAMBDA_CAPTURE_EQ_THIS(const Meta& L, const Meta& R)
        {
            return L.id < R.id;
        };

        m_max_zorder = std::max(m_max_zorder, draw_states->zorder) + 1;
        if(set_max)
            draw_states->zorder = m_max_zorder - 1;

        if(draw_states->states.custom_draw_fn)
        {
            Meta meta;
            meta.drawable                   = draw_states;
            meta.id                         = draw_states->id;
            meta.states.viewport            = draw_states->states.viewport;
            meta.states.custom_draw_fn      = draw_states->states.custom_draw_fn;
            meta.states.custom_draw_enable  = draw_states->states.custom_draw_enable;
            meta.zorder                     = draw_states->zorder;
            //
            m_drawables.push_back(meta);
            m_index_refresh_count = 1;
            std::sort(m_drawables.begin(), m_drawables.end(), sort_lmbd);
            return;
        }

        unsigned int first_index_count  = m_index_count;
        unsigned int first_vertex_count = m_vertex_count;
        //printf("vertex count (add): %lld\n", m_vertex_count);
        //DANGER!!
        m_vertex_count += draw_states->vertex_count;//primitive->m_vertices.size();
        m_index_count  += draw_states->index_count;//primitive->m_indices.size();


        Meta meta;
        meta.drawable       = draw_states;
        meta.id             = draw_states->id;
        meta.first_index    = first_vertex_count;
        meta.zorder         = draw_states->zorder;

        //DANGER!!
        meta.vertex_count   = draw_states->vertex_count;//primitive->m_vertices.size();
        meta.index_count    = draw_states->index_count;//primitive->m_indices.size();

        meta.vertex_entry   = first_vertex_count;
        meta.index_entry    = first_index_count;


        meta.toggle         = draw_states->visible;

        //render states..
        meta.states         = draw_states->states;
        meta.states.custom_draw_fn = nullptr;
        /*
        meta.texture        = primitive->m_states.texture;
        meta.shader         = primitive->m_states.shader;
        meta.primitive      = primitive->m_primitive_type;
        */
        m_drawables.push_back(meta);

        m_positions.reserve(m_vertex_count);
        m_colors.reserve(m_vertex_count);
        m_tex_coords.reserve(m_vertex_count);
        m_indices.reserve(m_index_count);

        size_t i;
        //DANGER!!
        for(i = 0; i < draw_states->vertex_count; i++)
        {

            const sp::Vertex& vertex = primitive->m_vertices[i + draw_states->vertex_entry];
            m_positions.push_back(vertex.position);
            m_colors.push_back(vertex.color);
            m_tex_coords.push_back(vertex.texCoords);
        }
        m_index_refresh_count = 1;
        ///std::sort(m_drawables.begin(), m_drawables.end(), sort_lmbd);
    }

    //write to depth buffer with glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_FLOAT, buffer)??
    //stencil test by depth value (z = y position)??

    void Renderer::removeDrawable(      Drawable::DrawableStates::Ptr ptr)
    {
        if(!ptr)
            return;

        removeMetaObject(ptr->id);
    }
    void Renderer::removeDrawable(const Drawable::Ptr primitive)
    {
        if(!primitive) return;
        primitive->removeDrawable(*this);
    }

    void Renderer::removeDanglingDrawables()
    {
        static std::vector<int> to_remove;
        for(auto& meta : m_drawables)
        {
            if(!meta.drawable.lock() || meta.drawable.expired())
            {
                to_remove.push_back(meta.id);
            }
        }

        for(int i : to_remove)
        {
            removeMetaObject(i);
        }
        to_remove.clear();
    }

    void Renderer::removeMetaObject(long int id)
    {
        Meta* meta  = nullptr;
        auto it     = m_drawables.begin();
        for(; it != m_drawables.end(); ++it)
        {
            Meta& found = *it;
            if(id == found.id)
            {
                meta = &found;
                break;
            }
        }

        if(!meta)
        {
            return;
        }

        if(meta->states.custom_draw_fn)
        {
            m_drawables.erase(it);
            return;
        }
        size_t start = meta->vertex_entry;
        size_t end   = start + meta->vertex_count;

        m_positions.erase(m_positions.begin() + start, m_positions.begin() + end);
        m_colors.erase(m_colors.begin() + start, m_colors.begin() + end);
        m_tex_coords.erase(m_tex_coords.begin() + start, m_tex_coords.begin() + end);

        size_t v_entry = meta->vertex_entry;
        size_t i_entry = meta->index_entry;

        //DANGER!!
        size_t v_count = meta->vertex_count;//primitive->m_drawable_states->vertex_count;//meta->drawable.lock()->drawable->m_vertices.size();
        size_t i_count = meta->index_count;//primitive->m_drawable_states->index_count;//meta->drawable.lock()->drawable->m_indices.size();

        //not correct..
        m_drawables.erase(it);

        for(auto& m : m_drawables)
        {
            if(m.vertex_entry > v_entry)
                m.vertex_entry -= v_count;

            if(m.index_entry > i_entry)
                m.index_entry -= i_count;

            if(m.first_index > v_entry)
                m.first_index -= v_count;
        }

        m_vertex_count -= v_count;
        m_index_count  -= i_count;

        m_index_refresh_count = 1;
    }

    void swap(std::vector<int>& v, int start1, int end1, int start2, int end2)
    {
        int size2 = end2 - start2 + 1;
        int size1 = end1 - start1 + 1;
        auto begin = v.begin() + start1;
        auto end   = v.begin() + end2 + 1;

        std::rotate(begin, v.begin() + start2, end);
        std::rotate(begin + size2, begin + size2 + size1, end);
    }

    void Renderer::refresh()
    {
        //removeDanglingDrawables();

        for(auto& meta : m_drawables)
        {
            if(!meta.drawable.lock() || meta.drawable.expired())
                continue;

            if(meta.zorder != meta.drawable.lock()->zorder)
            {
                meta.zorder = meta.drawable.lock()->zorder;
                m_max_zorder = std::max(m_max_zorder, meta.zorder);
                m_index_refresh_count = 1;
            }

            if(meta.states.custom_draw_fn)
            {
                meta.states.custom_draw_enable = meta.drawable.lock()->states.custom_draw_enable;
                continue;
            }

            auto ptr = meta.drawable.lock();
            if(meta.toggle != ptr->visible)
            {
                m_index_refresh_count = 1;
                meta.toggle = ptr->visible;
            }

            if(meta.vertex_count != ptr->vertex_count)
            {
                m_index_refresh_count = 1;
            }
        }

        static std::vector<Meta> sorter;
        static size_t index_count = 0;
        if(m_index_refresh_count > 0 )
        {
            sorter.clear();
            m_indices.clear();
            m_batches.clear();
            index_count = 0;
        }

        Meta* previous = nullptr;

        bool resize = false;
        for(auto& meta : m_drawables)
        {
            if(!meta.drawable.lock() || meta.drawable.expired())
                continue;

            if(meta.states.custom_draw_fn)
            {
                if(m_index_refresh_count > 0)
                {
                    sorter.push_back(meta);
                }
                continue;
            }


            auto ptr = meta.drawable.lock();
            meta.states.texture = meta.drawable.lock()->states.texture;
            meta.vertex_entry = (previous) ? (previous->vertex_entry + previous->vertex_count) : 0;
            //meta.index_entry  = (previous) ? (previous->index_entry  + previous->index_count) : 0;
            meta.first_index  = (previous) ? (previous->first_index  + previous->vertex_count) : 0;


            std::vector<Vertex>& vertices = ptr->client->m_vertices;
            if(meta.vertex_count != ptr->vertex_count /*ptr->drawable->m_vertices.size()*/)
            {
                //for varying vertex counts (not tested yet)..
                long int new_count      = static_cast<long int>(ptr->vertex_count/*ptr->drawable->m_vertices.size()*/);
                long int old_count      = static_cast<long int>(meta.vertex_count);
                long int difference     = (new_count - old_count);
                m_vertex_count         += (difference);

                meta.vertex_count       = static_cast<size_t>(new_count);
                meta.index_count        = ptr->index_count;//ptr->drawable->m_indices.size();
                //meta.first_index        = m_vertex_count;

                ///DANGER??
                if(new_count > old_count)
                {
                    m_positions.resize(m_vertex_count);
                    m_colors.resize(m_vertex_count);
                    m_tex_coords.resize(m_vertex_count);


                    if(m_vertex_count > m_positions.size())
                    {
                        //SP_PRINT_WARNING("WARNING, resize inaccurate");
                        for(size_t i = 0; i < meta.vertex_count; i++)
                        {
                            m_positions.push_back(vertices[i + ptr->vertex_entry].position + ptr->position);
                            m_colors.push_back(vertices[i + ptr->vertex_entry].color);
                            m_tex_coords.push_back(vertices[i + ptr->vertex_entry].texCoords);
                        }
                    }
                    else
                    {
                        m_positions.erase(m_positions.begin() + meta.vertex_entry, m_positions.begin() + meta.vertex_entry + new_count);
                        m_colors.erase(m_colors.begin() + meta.vertex_entry, m_colors.begin() + meta.vertex_entry + new_count);
                        m_tex_coords.erase(m_tex_coords.begin() + meta.vertex_entry, m_tex_coords.begin() + meta.vertex_entry + new_count);
                        for(size_t i = 0; i < meta.vertex_count; i++)
                        {
                            m_positions.insert(m_positions.begin() + meta.vertex_entry + i, vertices[i + ptr->vertex_entry].position + ptr->position);
                            m_colors.insert(m_colors.begin() + meta.vertex_entry + i, vertices[i + ptr->vertex_entry].color);
                            m_tex_coords.insert(m_tex_coords.begin() + meta.vertex_entry + i, vertices[i + ptr->vertex_entry].texCoords);
                        }
                    }
                }
                else
                {
                    m_positions.erase(m_positions.begin() + meta.vertex_entry + new_count, m_positions.begin() + meta.vertex_entry + old_count);
                    m_colors.erase(m_colors.begin() + meta.vertex_entry + new_count, m_colors.begin() + meta.vertex_entry + old_count);
                    m_tex_coords.erase(m_tex_coords.begin() + meta.vertex_entry + new_count, m_tex_coords.begin() + meta.vertex_entry + old_count);
                }

                resize = ptr->update = true;
            }


            if(ptr->update || resize)
            {
                //ok..
                std::vector<Vertex>& vertices = ptr->client->m_vertices;
                size_t vertex_entry = meta.vertex_entry;

                //DANGER!!
                size_t length =  ptr->vertex_count;//vertices.size();

                if(meta.vertex_count != length)
                {
                    SP_PRINT_WARNING("miscalculation");
                    continue;
                }
                size_t entry = ptr->vertex_entry;
                for(size_t i = 0; i < length; i++)
                {
                    Vertex& vertex = vertices[i + entry];
                    m_positions  [i + vertex_entry] = vertex.position + ptr->position;
                    m_colors     [i + vertex_entry] = vertex.color;
                    m_tex_coords [i + vertex_entry] = vertex.texCoords;
                }
                ptr->update = false;
            }

            if(m_index_refresh_count > 0)
            {
                sorter.push_back(meta);
                //DANGER!!
                index_count +=  ptr->index_count;//ptr->drawable->m_indices.size();
            }

            previous = &meta;
        }


        if(m_index_refresh_count > 0)
        {
          //  printf("ok 3..\n");
            if(sorter.empty())
            {
                m_index_refresh_count = 0;
                return;
            }

            m_index_count = index_count;
            m_indices.reserve(m_index_count);
            --m_index_refresh_count;
            static auto lmbd =
            SP_LAMBDA_CAPTURE_EQ_THIS(const Meta& L, const Meta& R)->bool
            {
                int left_z = L.zorder;
                int right_z = R.zorder;

                m_max_zorder = std::max(left_z, (int)m_max_zorder);
                m_max_zorder = std::max(right_z, (int)m_max_zorder);
                const sp::Texture* lt = L.states.texture;
                const sp::Texture* rt = R.states.texture;
                unsigned int lh = lt ? lt->getHandleGL() : 0;
                unsigned int rh = rt ? rt->getHandleGL() : 0;

                if(left_z == right_z)
                    return (lh < rh);

                return (left_z < right_z);
            };
            std::sort(sorter.begin(), sorter.end(), lmbd);

            Batch batch;
            batch.states.texture       = nullptr;
            batch.states.shader        = nullptr;
            batch.index_start   = 0;
            batch.index_count   = 0;

            Meta* first_batch_draw     = nullptr;

            for(auto& m : sorter)
            {
                if(!m.states.custom_draw_fn)
                {
                    first_batch_draw = &m;
                    break;
                }
            }

            if(!first_batch_draw)
            {
                //only custom drawables in memory..
                for(auto& m : sorter)
                {
                    Batch batch;
                    batch.states.custom_draw_fn         = m.states.custom_draw_fn;
                    batch.states.viewport               = m.states.viewport;
                    batch.states.custom_draw_enable     = m.states.custom_draw_enable;
                    m_batches.push_back(batch);
                }
                printf("%lld draw calls..\n", m_batches.size());
                return;
            }

            const sp::Texture* texture  = first_batch_draw->states.texture           ;
            const sp::Shader* shader    = first_batch_draw->states.shader            ;
            int primitive_type          = first_batch_draw->states.primitive_type    ;
            bool lighting               = first_batch_draw->states.lighting          ;
            float point_size            = first_batch_draw->states.point_size        ;
            Blending blending           = first_batch_draw->states.blend_mode        ;
            const Viewport* viewport    = first_batch_draw->states.viewport          ;
            //std::function<void()>  cb   = (*sorter.begin()).states.custom_draw_fn;

            unsigned int tex_obj        = texture ? texture->getHandleGL() : 0;
            unsigned int shader_obj     = shader ? shader->getHandleGL() : 0;


            unsigned int index_count    = 0;
            unsigned int offset         = 0;
            bool last_draw              = true;

            //printf("sorter size: %lld\n", sorter.size());
            for(auto it = sorter.begin(); it != sorter.end(); ++it)
            {
                Meta& meta = *it;
                States& states = meta.states;


                if(states.custom_draw_fn)
                {
                    if(meta.drawable.lock()->states.custom_draw_fn && states.custom_draw_enable)
                    {
                        Batch tmp;
                        tmp.states.viewport             = states.viewport;
                        tmp.states.custom_draw_fn       = states.custom_draw_fn;
                        tmp.states.custom_draw_enable   = states.custom_draw_enable;
                        last_draw = true;
                        m_batches.push_back(tmp);
                    }
                    continue;
                }


                if(!meta.toggle)
                {
                    continue;
                }
                unsigned meta_tex = meta.states.texture ? meta.states.texture->getHandleGL() : 0;
                unsigned meta_shader = meta.states.shader ? meta.states.shader->getHandleGL() : 0;
                if(     tex_obj                      != meta_tex
                   ||   shader_obj                   != meta_shader
                   ||   states.primitive_type   != primitive_type
                   ||   states.lighting         != lighting
                   //||   states.viewport         != viewport
                   ||   states.blend_mode       != blending
                   )
                {

                    //printf("batch draw..\n");
                    batch.states.texture        = texture;
                    batch.states.shader         = shader;
                    batch.states.primitive_type = primitive_type;
                    batch.states.point_size     = point_size;
                    batch.states.blend_mode     = blending;
                    batch.states.lighting       = lighting;
                    batch.states.viewport       = viewport;

                    batch.index_start           = offset;
                    batch.index_count           = index_count;

                    texture                     = meta.states.texture;
                    shader                      = meta.states.shader;
                    primitive_type              = meta.states.primitive_type;
                    lighting                    = meta.states.lighting;
                    point_size                  = meta.states.point_size;
                    blending                    = meta.states.blend_mode;
                    viewport                    = meta.states.viewport;

                    tex_obj                     = meta_tex;
                    shader_obj                  = meta_shader;

                    offset                     += index_count;
                    index_count                 = 0;

                    m_batches.push_back(batch);
                }

                meta.index_entry = m_indices.size();
                size_t entry = meta.drawable.lock()->index_entry;
                for(size_t i = entry; i < (entry + meta.index_count); i++)
                {
                    unsigned index = 0;
                    if(meta.drawable.lock())
                    {
                        //DANGER!!
                        index = meta.drawable.lock()->client->m_indices[i] + meta.first_index;
                    }
                    else
                    {
                        index = i + meta.first_index;
                    }
                    m_indices.push_back(index);
                    ++index_count;
                }

                last_draw = false;
            }

            //printf("batch draw..\n");
            batch.index_start           = offset;
            batch.index_count           = index_count;
            batch.states.texture        = texture;
            batch.states.shader         = shader;
            batch.states.primitive_type = primitive_type;
            batch.states.point_size     = point_size;
            batch.states.blend_mode     = blending;
            batch.states.lighting       = lighting;
            batch.states.viewport       = viewport;
            m_batches.push_back(batch);
        }
    }

    void Renderer::clear(const Color& color)
    {
        m_primary_framebuffer.bind();
        spCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f))
        spCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    }

    void Renderer::applyTexture(const Texture* texture)
    {
        sp::Texture::bind(texture, Texture::SP_Mapping::Normalized);
        m_cache.last_texture = texture;
    }

    void Renderer::applyShader(const Shader* shader)
    {
        if(m_cache.last_shader == shader)
            return;

        sp::Shader::bind(shader);
        m_cache.last_shader = shader;
    }

    void Renderer::setAlphaThreshold(float threshold)
    {
        if(threshold > 0.f)
            m_cache.alpha_threshold = threshold;
    }

    void Renderer::resetStatesGL()
    {
        static bool shader_available = Shader::shader_objects_supported();
        /*
        if(Controller::active())
        {
        */

        if(GL_ARB_multitexture_supported)
        {
            spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
        }

        spCheck(glPointSize(1.f));
        spCheck(glDisable(GL_CULL_FACE))
        spCheck(glDisable(GL_LIGHTING))
        spCheck(glDisable(GL_DEPTH_TEST))
        spCheck(glDisable(GL_ALPHA_TEST))
        spCheck(glEnable(GL_TEXTURE_2D))
        spCheck(glEnable(GL_BLEND))
        spCheck(glEnable(GL_SCISSOR_TEST))
        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadIdentity())
        spCheck(glEnableClientState(GL_VERTEX_ARRAY))
        spCheck(glEnableClientState(GL_COLOR_ARRAY))
        spCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY))

        m_cache.gl_states_set = true;

        applyBlending(BlendAlpha);
        applyTexture(NULL);
        if(shader_available)
            applyShader(NULL);

        setViewport(rectf{m_default_view.getOrigin(), m_default_view.getSize()});

        //}

    }

    void Renderer::setupDraw()
    {
        if(!m_cache.gl_states_set)
            resetStatesGL();

        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadIdentity())

        if(m_cache.viewport_change)
            applyCurrentView();
        if(m_blend_mode != m_cache.last_blend_mode)
            applyBlending(m_blend_mode);
    }

    void Renderer::cleanupDraw()
    {
        if(m_cache.last_shader)
            applyShader(NULL);

        if(m_cache.last_texture)
        {
            if(GL_ARB_multitexture_supported)
            {
                spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
                spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
            }
            applyTexture(NULL);
        }
    }

    vec2f Renderer::mapPixelsToCoords(int x, int y)
    {
        sp::vec2f normalized;
        normalized.x = -1.f + 2.f * (x - m_default_view.getOrigin().x) / m_default_view.getSize().x;
        normalized.y =  1.f - 2.f * (y - m_default_view.getOrigin().y) / m_default_view.getSize().y;
        return m_inv_view_matrix.transformVertex(normalized);
    }

    vec2i Renderer::mapCoordsToPixels(float x, float y)
    {
        sp::vec2f normalized = m_view_matrix.transformVertex(vec2f{x, y});

        sp::vec2i pixel;
        pixel.x = static_cast<int>(std::floor(( normalized.x + 1.f) / 2.f * m_default_view.getSize().x + m_default_view.getOrigin().x + .5f));
        pixel.y = static_cast<int>(std::floor((-normalized.y + 1.f) / 2.f * m_default_view.getSize().y + m_default_view.getOrigin().y + .5f));
        return pixel;
    }


    void Renderer::draw(Drawable::Ptr drawable)
    {
        if(!drawable)
            return;

        if(drawable->m_vertices.empty() || drawable->m_indices.empty())
            return;

		setupDraw();
		spCheck(glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT | GL_CLIENT_VERTEX_ARRAY_BIT))
        spCheck(glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT))

        const States& states = drawable->m_drawable_states->states;

		if(states.primitive_type == GL_POINTS)
		{
			spCheck(glEnable(GL_LINE_SMOOTH));
			spCheck(glEnable(GL_POINT_SMOOTH));
			spCheck(glEnable(GL_POLYGON_SMOOTH));
			spCheck(glPointSize(states.point_size))

			if(GL_ARB_point_sprite_supported)
			{
				spCheck(glEnable(GL_POINT_SPRITE_ARB))
				spCheck(glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE))
			}
		}

		applyBlending(states.blend_mode);
        applyTexture(states.texture);
        applyShader(states.shader);

        static Transformable trm;

        trm.setPosition(drawable->m_drawable_states->position);
        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadMatrixf(trm.getMatrix()()))

        const char* data = reinterpret_cast<const char*>(drawable->getVertices());



        spCheck(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), data));
        spCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + 8))
        spCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 12))

        //DANGER!!
        spCheck(glDrawElements(states.primitive_type, (size_t)drawable->m_indices.size(), GL_UNSIGNED_INT, (void*)(&drawable->m_indices[0]))) //OFFSET!!
		if(states.primitive_type == GL_POINTS)
		{
			spCheck(glDisable(GL_POINT_SPRITE_ARB))
			spCheck(glPointSize(1.f))
			spCheck(glDisable(GL_LINE_SMOOTH));
			spCheck(glDisable(GL_POINT_SMOOTH));
			spCheck(glDisable(GL_POLYGON_SMOOTH));
			if(GL_ARB_point_sprite_supported)
			{
				spCheck(glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_FALSE));
			}
		}
		cleanupDraw();

		spCheck(glPopClientAttrib())
		spCheck(glPopAttrib())
    }

    void Renderer::drawFrame(float x, float y, float width, float height, const sp::Texture* texture, const sp::Shader* shader, Viewport* viewport)
    {
        static float frame_position[2 * 6] =
        {
            1.f, 1.f,
            0.f, 0.f,
            0.f, 1.f,
            0.f, 0.f,
            1.f, 0.f,
            1.f, 1.f
        };

        frame_position[2] = frame_position[4] = frame_position[6] = x;
        frame_position[3] = frame_position[7] = frame_position[9] = y;
        frame_position[0] = frame_position[8] = frame_position[10] = width;
        frame_position[1] = frame_position[5] = frame_position[11] = height;

        static SPuint8 frame_color[4 * 6] =
        {
            255, 255, 255, 255,
            255, 255, 255, 255,
            255, 255, 255, 255,
            255, 255, 255, 255,
            255, 255, 255, 255,
            255, 255, 255, 255
        };

        static float frame_tex_coords[2 * 6] =
        {
            1.f, 0.f,
            0.f, 1.f,
            0.f, 0.f,
            0.f, 1.f,
            1.f, 1.f,
            1.f, 0.f
        };

        if(viewport)
        {
            viewport->load();
            m_cache.viewport_change = true;
        }
        else
        {
            if(m_cache.viewport_change)
                applyCurrentView();
        }

        if(GL_ARB_multitexture_supported)
        {
            spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
        }
        applyTexture(texture);
        applyShader(shader);

        spCheck(glVertexPointer(2, GL_FLOAT, 0, &frame_position[0]));
        spCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &frame_color[0]));
        spCheck(glTexCoordPointer(2, GL_FLOAT, 0, &frame_tex_coords[0]));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    }


    /*
     *  1. draw scene to fbo..
     *  2. draw color attachment to screen..
     *  3. draw radial gradient to fbo..
     *  4. draw color attachment to screen..
     */
    void Renderer::draw()
    {
        //printf("ok 1..\n");
        refresh();

        if((!m_index_count || m_indices.empty()) && m_batches.empty())
        {
            return;
        }
        //printf("pos: %lld, colors: %lld, tcs: %lld\n", m_positions.size(), m_colors.size(), m_tex_coords.size());
        setupDraw();
        spCheck(glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT | GL_CLIENT_VERTEX_ARRAY_BIT))
        spCheck(glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT))

        if(m_cache.alpha_threshold > 0.f)
        {
            spCheck(glEnable(GL_ALPHA_TEST))
            spCheck(glAlphaFunc(GL_GREATER, m_cache.alpha_threshold))
        }
        spCheck(glVertexPointer(2, GL_FLOAT, 0, &m_positions[0]));
        spCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &m_colors[0]));
        spCheck(glTexCoordPointer(2, GL_FLOAT, 0, &m_tex_coords[0]));

        /*
        static const sp::Texture*   texture    = nullptr;
        static const sp::Shader*    shader     = nullptr;
        */
        static unsigned int tex_obj        = 0;
        static unsigned int shader_obj     = 0;

               const Viewport*      viewport   = &m_default_view;
        static bool                 lighting   = false;
        static Blending             blending   = m_cache.last_blend_mode;

        /**
         *  needed:
         *  - particle batch..
         *  - flag for point sprites..
         */

        //this draws the plain scene without any post-effects..
        for(auto& batch : m_batches)
        {
            if(batch.states.custom_draw_enable)
            {
                //tested..
                spCheck(;)
                spCheck(glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS))
                spCheck(glPushAttrib(GL_ALL_ATTRIB_BITS))
                spCheck(glMatrixMode(GL_MODELVIEW))
                spCheck(glPushMatrix())
                spCheck(glMatrixMode(GL_PROJECTION))
                spCheck(glPushMatrix())
                spCheck(glMatrixMode(GL_TEXTURE))
                spCheck(glPushMatrix())
                spCheck(glMatrixMode(GL_MODELVIEW))
                resetStatesGL();

                if(batch.states.viewport && !batch.states.viewport->defaulted())
                {
                    batch.states.viewport->load();
                    m_cache.viewport_change = true;
                }

                if(batch.states.custom_draw_fn)
                    batch.states.custom_draw_fn();

                if(m_cache.viewport_change)
                    applyCurrentView();

                spCheck(glMatrixMode(GL_PROJECTION))
                spCheck(glPopMatrix())
                spCheck(glMatrixMode(GL_MODELVIEW))
                spCheck(glPopMatrix())
                spCheck(glMatrixMode(GL_TEXTURE))
                spCheck(glPopMatrix())
                spCheck(glMatrixMode(GL_MODELVIEW))
                spCheck(glPopClientAttrib())
                spCheck(glPopAttrib())
            }
            else
            {
                unsigned int batch_tex = batch.states.texture ? batch.states.texture->getHandleGL() : 0;
                if(batch_tex != tex_obj)
                {
                    if(GL_ARB_multitexture_supported)
                    {
                        spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
                        spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
                    }
                    applyTexture(batch.states.texture);
                }

                unsigned int batch_shader = batch.states.shader ? batch.states.shader->getHandleGL() : 0;
                if(batch_shader != shader_obj)
                {
                    shader_obj = batch_shader;
                    applyShader(batch.states.shader);
                }

                viewport = batch.states.viewport;
                if(viewport && !viewport->defaulted() && *viewport != m_default_view)
                {
                    viewport->load();
                    m_cache.viewport_change = true;
                }
                else
                {
                    if(m_cache.viewport_change)
                        applyCurrentView();
                }

                if(batch.states.lighting != lighting)
                {
                    lighting = batch.states.lighting;
                    if(lighting)
                    {
                        spCheck(glEnable(GL_LIGHTING))
                    }
                    else
                    {
                        spCheck(glDisable(GL_LIGHTING))
                    }
                }

                if(batch.states.blend_mode != blending)
                {
                    blending = batch.states.blend_mode;
                    applyBlending(blending);
                }

                if(batch.states.primitive_type == GL_POINTS)
                {
                    spCheck(glEnable(GL_LINE_SMOOTH));
                    spCheck(glEnable(GL_POINT_SMOOTH));
                    spCheck(glEnable(GL_POLYGON_SMOOTH));
                    spCheck(glPointSize(batch.states.point_size))
                    //static float max;
                    //glGetFloatv(GL_MAX_POINT_SIZE, &max);
                    //glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
                    if(GL_ARB_point_sprite_supported)
                    {
                        spCheck(glEnable(GL_POINT_SPRITE_ARB))
                        spCheck(glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE))
                    }
                }
                //printf("index start: %lld index count: %lld, index cache: %lld\n", batch.index_start, batch.index_count, m_indices.size());
                spCheck(glDrawElements(batch.states.primitive_type, batch.index_count, GL_UNSIGNED_INT, (void*)(&m_indices[0] + batch.index_start)))

                if(batch.states.primitive_type == GL_POINTS)
                {
                    spCheck(glDisable(GL_POINT_SPRITE_ARB))
                    spCheck(glPointSize(1.f))
                    spCheck(glDisable(GL_LINE_SMOOTH));
                    spCheck(glDisable(GL_POINT_SMOOTH));
                    spCheck(glDisable(GL_POLYGON_SMOOTH));
                    if(GL_ARB_point_sprite_supported)
                    {
                        spCheck(glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_FALSE));
                    }
                }
            }
        }

        if(m_cache.alpha_threshold > 0.f)
        {
            spCheck(glDisable(GL_ALPHA_TEST))
            spCheck(glAlphaFunc(GL_GREATER, 0.f))
        }
        spCheck(glPopAttrib())
        spCheck(glPopClientAttrib())

        m_primary_framebuffer.display();

        spCheck(glClearColor(0.f, 0.f, 0.f, 0.f))
        spCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))

        drawFrame(m_frame_position.x, m_frame_position.y,
                  (float)m_size.x, (float)m_size.y,
                  &m_primary_framebuffer.getColorTexture(), m_post_process_shader);

        cleanupDraw();
    }
}
