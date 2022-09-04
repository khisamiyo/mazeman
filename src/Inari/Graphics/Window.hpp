#pragma once

#include <glm/ext/vector_int2.hpp>
#include <glm/vec4.hpp>

#include <memory>
#include <string_view>

namespace inari
{
    class Window
    {
        friend class IGame;
    public:
        explicit Window(const std::string_view& title, int width, int height);
        ~Window();

        void clear(const glm::vec4& color);
        void display();

        void setWindowSize(const glm::ivec2& size);
        glm::ivec2 getWindowSize() const;

        void setTitle(const std::string_view& title);

        void setFrameLimit(int screenFps);

    protected:
        void begin();
        void end();

    private:
        std::unique_ptr<struct WindowData> m_data;

        std::unique_ptr<int> m_frameLimit;
    };
}