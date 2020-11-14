#include "pch.h"

#ifndef TEDIT_BATCH_
#define TEDIT_BATCH_

#include "gfx/gl.h"

namespace gfx {

/**
 * Batch of vertices.
 * This is a wrapper over an OpenGL buffer object. It manages it's GPU memory.
 * Grab the handle with Batch::handle() and bind it to a VAO, or draw from it directly.
 */
class Batch
{
public:
    /**
     * @param size maximum batch size
     */
    Batch(size_t size);
    ~Batch();
    Batch(const Batch& other) = delete;
    Batch& operator=(const Batch& other) = delete;
    Batch(Batch&& other);
    Batch& operator=(Batch&& other);

    /**
     * Insert data into the batch. Never allocates. Checking for overflow is user's responsibility.
     */
    void insert(const void* data, size_t length);
    void flush();
    GLuint handle() const noexcept;
    size_t size() const noexcept;
    size_t capacity() const noexcept;
    size_t remaining() const noexcept;

private:
    size_t capacity_;
    size_t cursor_;
    std::vector<uint8_t> data_;
    GLuint buffer_;

}; // class Batch

} // namespace gfx

#endif // TEDIT_BATCH_
