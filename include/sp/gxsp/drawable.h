#ifndef PRIMITIVE_H_INCLUDED
#define PRIMITIVE_H_INCLUDED
#include <sp/gxsp/vertex.h>
#include <sp/gxsp/texture.h>
#include <sp/gxsp/shader.h>
#include <sp/gxsp/render_states.h>
#include <sp/math/vec.h>
#include <memory>

namespace sp
{
    class Renderer;
    class SP_API Drawable : std::enable_shared_from_this<Drawable>
    {
        public:
            typedef std::shared_ptr<Drawable>         Ptr;
            typedef std::shared_ptr<const Drawable>   ConstPtr;

            virtual ~Drawable();
            //void merge(const Drawable& other);

            Ptr get();

            virtual void add(const sp::Vertex& vertex);
            virtual void setTexture(const sp::Texture& texture);
            virtual void setShader(const sp::Shader& shader);

            virtual void setPosition(float x, float y);
                    void setPosition(const vec2f& position);
            const vec2f& getPosition() const;

            virtual void setZOrder(int order);
            int getZOrder() const;

            const Vertex* getVertices() const;
            const unsigned int* getIndices() const;
            const Texture* getTexture() const;
            const Shader* getShader() const;

            //virtual void setViewport(const rectf& rect) final;

            virtual void setVisible(bool visible);


            //a custom draw call must be set before adding it to the renderer..
            //the renderer will not draw the drawable batched, when
            //setting the drawable's draw callback to nullptr..
            //
            //it is also not valid to set a draw callback, once
            //the drawable has been added to the renderer..
            //setting enableCustomDraw(bool) wont have any affect there after..
            virtual void setDrawCallback(std::function<void()> fn, bool enable = true) final;
            virtual bool enableCustomDraw(bool enable) final;

            bool isVisible() const;

            void reset();

                    rectf getLocalBounds() const;
            virtual rectf getGlobalBounds() const;

                    long int   getUniqueID() const;
        protected:
            friend class Renderer;
            friend class Meta;
            friend class BatchDrawable;

            struct CustomDrawStates
            {
                Blending        blend_mode;
                mat             matrix;
                const Texture*  texture;
                const Shader*   shader;
                int             primitive;
            };

            static inline CustomDrawStates DEFAULT_DRAW_STATE =
            {
                BlendAlpha,
                mat{},
                nullptr,
                nullptr,
                Triangles
            };
            //specifically for transient draws to FBOs??
            static void draw(
            const Vertex* vertices,
            size_t length,
            int width,
            int height,
            const CustomDrawStates& states = DEFAULT_DRAW_STATE,
            bool use_tex_coord_array = true,
            const sp::Color& clear_color = {0, 0, 0, 0});

                                        Drawable();

            virtual void addDrawable(Renderer& renderer, bool max = false);
            virtual void removeDrawable(Renderer& renderer);
            struct DrawableStates
            {
                typedef std::shared_ptr<DrawableStates>         Ptr;
                typedef std::shared_ptr<const DrawableStates>   ConstPtr;
                States                      states;
                bool                        visible;
                vec2f                       position;
                int                         zorder;
                bool                        update;
                rectf                       bounds;

                //DISCLAIMER:
                //this* can never be dangling, because sp is owned by this*..
                Drawable*                   client;

                size_t                      vertex_entry;
                size_t                      index_entry;
                size_t                      vertex_count;
                size_t                      index_count;
                unsigned int                id;

                DrawableStates() :
                    states      {},
                    visible     {true},
                    position    {},
                    zorder      {0},
                    update      {true},
                    bounds      {},
                    client      {nullptr},
                    vertex_entry{0},
                    index_entry {0},
                    vertex_count{0},
                    index_count {0},
                    id          {0}
                {
                }
                DrawableStates(const DrawableStates& other) :
                    states      {other.states},
                    visible     {other.visible},
                    position    {other.position},
                    zorder      {other.zorder},
                    update      {other.update},
                    bounds      {other.bounds},
                    client      {other.client},
                    vertex_entry{other.vertex_entry},
                    index_entry {other.index_entry},
                    vertex_count{other.vertex_count},
                    index_count {other.index_count},
                    id          {other.id}
                {
                }

                DrawableStates& operator=(const DrawableStates& other)
                {
                    if(this != &other)
                    {
                        states       = other.states;
                        visible      = other.visible;
                        position     = other.position;
                        zorder       = other.zorder;
                        update       = other.update;
                        bounds       = other.bounds;
                        client       = other.client;
                        vertex_entry = other.vertex_entry;
                        index_entry  = other.index_entry;
                        vertex_count = other.vertex_count;
                        index_count  = other.index_count;
                        id           = other.id;
                    }
                    return *this;
                }
            };

            //shared states for updates..
            std::shared_ptr<DrawableStates> m_drawable_states;

            //non-copyable data..
            std::vector<Vertex>         m_vertices;
            std::vector<unsigned int>   m_indices;

    };
}
#endif // PRIMITIVE_H_INCLUDED
