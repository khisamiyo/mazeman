#include "SpriteBatch.hpp"

#include <glad/glad.h>

#include <glm/ext/matrix_transform.hpp>

#include "Resources/Shader.hpp"
#include "Resources/Texture2D.hpp"

/* TODO
 * Enum SpriteSortMode
 * {
 *      Deferred,   // All sprites are drawing when End() invokes,
 *                  // in order of draw call sequence.
 *      Immediate,  // Each sprite is drawing at individual draw call,
 *                  // instead of End().
 *      Texture,    // Same as Deferred, except sprites are sorted by
 *                  // texture prior to drawing.
 * }
 */

struct SpriteData
{
    const std::shared_ptr<Texture2D>& texture;

    struct VertexData
    {
        glm::vec2 position;
        glm::vec2 uv;

        VertexData(const glm::vec2& position, const glm::vec2& uv)
            : position(position)
            , uv(uv)
        {
        }
    };
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;

    float radian = 0.0f;
    glm::vec2 origin = glm::vec2(0.0f);
    glm::vec4 color = glm::vec4(1.0f);

    SpriteData()
        : texture(nullptr)
    {
    }

    SpriteData(const std::shared_ptr<Texture2D>& texture)
        : texture(texture)
    {
    }
};

constexpr std::size_t sizeOfVertexData = sizeof(SpriteData::VertexData);

SpriteBatch::SpriteBatch(const std::shared_ptr<Shader>& spriteShader)
    : m_isBegan(false)
    , m_shader(spriteShader)
    , m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_transformMatrix(1.0f)
{

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)sizeof(glm::vec2));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SpriteBatch::~SpriteBatch()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void SpriteBatch::begin(const glm::mat4& transformMatrix)
{
    m_transformMatrix = transformMatrix;
    m_isBegan = true;
}

void SpriteBatch::draw(const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, const glm::vec4& destRect)
{
    if (texture != nullptr)
    {
        const glm::vec4 sourceRect(0.0f, 0.0f, texture->getSize());
        draw(texture, color, destRect, sourceRect, 0.0f, glm::vec2(0.0f));
    }
}

void SpriteBatch::draw(const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, const glm::vec4& destRect,
                       const glm::vec4& sourceRect)
{
    draw(texture, color, destRect, sourceRect, 0.0f, glm::vec2(0.0f));
}

void SpriteBatch::draw(const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, const glm::vec4& destRect,
                       const glm::vec4& sourceRect, float rotationInRadian, const glm::vec2& origin)
{
    if (!m_isBegan || m_shader == nullptr || m_vbo == 0 || m_vao == 0 || texture == nullptr)
    {
        return;
    }

    SpriteData data(texture);
    data.vertices.emplace_back(glm::vec2(destRect.x, destRect.y), glm::vec2(sourceRect.x, sourceRect.y)); // 0
    data.vertices.emplace_back(glm::vec2(destRect.x + destRect.z, destRect.y),
                               glm::vec2(sourceRect.z, sourceRect.y)); // 1
    data.vertices.emplace_back(glm::vec2(destRect.x, destRect.y + destRect.w),
                               glm::vec2(sourceRect.x, sourceRect.w)); // 2
    data.vertices.emplace_back(glm::vec2(destRect.x + destRect.z, destRect.y + destRect.w),
                               glm::vec2(sourceRect.z, sourceRect.w)); // 3
    data.indices = { 0, 1, 2, 1, 2, 3 };

    data.radian = rotationInRadian;
    data.origin = origin;
    data.color = color;

    m_spriteBuffer.push_back(data);
}

void SpriteBatch::draw(const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, const glm::vec2& position)
{
    draw(texture, color, position, glm::vec4(0.0f), 0.0f, glm::vec2(0.0f));
}

void SpriteBatch::draw(const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, const glm::vec2& position,
                       const glm::vec4& sourceRect)
{
    draw(texture, color, position, sourceRect, 0.0f, glm::vec2(0.0f));
}

void SpriteBatch::draw(const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, const glm::vec2& position,
                       const glm::vec4& sourceRect, float rotationInRadian, const glm::vec2& origin)
{
    if (texture != nullptr)
    {
        const glm::vec4 sRect = sourceRect != glm::vec4(0.0f) ? sourceRect : glm::vec4(0.0f, 0.0f, texture->getSize());
        const glm::vec4 destRect(position, texture->getSize());
        draw(texture, color, destRect, sRect, rotationInRadian, origin);
    }
}

void SpriteBatch::end()
{
    m_isBegan = false;

    flush();
}

void SpriteBatch::flush()
{
    if (m_shader == nullptr || m_spriteBuffer.empty() || m_vbo == 0 || m_vao == 0)
    {
        return;
    }

    // AlphaBlend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader->use();
    m_shader->set("transform", m_transformMatrix);

    glBindVertexArray(m_vao);
    for (const auto& it : m_spriteBuffer)
    {
        if (it.texture != nullptr)
        {
            glActiveTexture(GL_TEXTURE0);
            it.texture->bind();
            m_shader->set("image", 0);
        }

        m_shader->set("radian", it.radian);
        m_shader->set("origin", it.origin);
        m_shader->set("color", it.color);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeOfVertexData * it.vertices.size(), it.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(it.indices) * it.indices.size(), it.indices.data(),
                     GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    glBindVertexArray(0);

    m_spriteBuffer.clear();
}
