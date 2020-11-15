#include "pch.h"
#ifndef TEDIT_WINDOW_
#define TEDIT_WINDOW_

struct GLFWwindow;

/**
 * GLFW RAII wrapper
 */
class Window final
{
public:
    enum class Dialog
    {
        OpenFile,
        SaveFile,
        SelectFolder
    }; // enum class Dialog

    Window(const std::string& title, int width, int height);
    ~Window();

    void pollInput();
    void close();
    bool shouldClose() const;
    void swapBuffers();
    void setTitle(const std::string& title);
    /**
     * @param callback void(success, selection)
     */
    void openDialog(Dialog type,
        const std::string& title,
        const std::vector<std::string>& filters,
        bool multi,
        std::function<void(bool, std::vector<std::string>&)> callback);
    bool isDialogOpen() const;

    enum Modifier : int
    {
        SHIFT = 1 << 0,
        ALT = 1 << 1,
        CONTROL = 1 << 2
    };

    bool shortcut(int modifiers, int key);

    const std::string& title() const;
    GLFWwindow* handle() const;
    int width() const;
    int height() const;

    /**
     * @param listener arguments are (xpos, ypos)
     */
    void addMouseMoveListener(std::function<void(double, double)> listener);
    /**
     * @param listener arguments are (button, action, modifiers)
     */
    void addMouseButtonListener(std::function<void(int, int, int)> listener);
    /**
     * @param listener arguments are (key, action, modifiers)
     */
    void addKeyListener(std::function<void(int, int, int)> listener);
    /**
     * @param listener arguments are (xoffset, yoffset)
     */
    void addScrollListener(std::function<void(double, double)> listener);
    /**
     * @param listener arguments are (width, height)
     */
    void addResizeListener(std::function<void(int, int)> listener);

    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int modifiers);
    void onKey(int key, int scancode, int action, int modifiers);
    void onScroll(double xoffset, double yoffset);
    void onResize(int width, int height);

private:
    GLFWwindow* handle_;
    std::string title_;
    int width_;
    int height_;

    std::atomic_bool dialogOpen_;
    std::mutex dialogMutex_;

    struct
    {
        std::vector<std::function<void(double, double)>> mouseMove;
        std::vector<std::function<void(int, int, int)>> mouseButton;
        std::vector<std::function<void(int, int, int)>> key;
        std::vector<std::function<void(double, double)>> scroll;
        std::vector<std::function<void(int, int)>> resize;
    } listeners_;
}; // class Window

#endif // TEDIT_WINDOW_
