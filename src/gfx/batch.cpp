#include "pch.h"
#include "batch.hpp"

namespace gfx {

Batch::Batch(size_t size)
  : capacity_(size)
  , cursor_(0)
  , data_()
  , buffer_()
{
    this->data_.resize(size, 0);
    // allocate empty buffer on GPU with specific capacity
    // we will always use glBufferSubData to upload vertices
    glGenBuffers(1, &this->buffer_);
    glBindBuffer(GL_ARRAY_BUFFER, this->buffer_);
    glBufferData(GL_ARRAY_BUFFER, this->capacity_, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);

    spdlog::info("New Batch with capacity {}", size);
}

Batch::~Batch()
{
    glDeleteBuffers(1, &this->buffer_);
}

Batch::Batch(Batch&& other)
  : capacity_(std::exchange(other.capacity_, 0))
  , cursor_(std::exchange(other.cursor_, 0))
  , data_(std::move(other.data_))
  , buffer_(std::exchange(other.buffer_, 0))
{}
Batch&
Batch::operator=(Batch&& other)
{
    if (this != &other) {
        this->capacity_ = std::exchange(other.capacity_, 0);
        this->cursor_ = std::exchange(other.cursor_, 0);
        this->data_ = std::move(other.data_);
        this->buffer_ = std::exchange(other.buffer_, 0);
    }
    return *this;
}

void
Batch::insert(const void* data, size_t length)
{
    // it is the caller's responsibility to check for remaining capacity,
    // and create another batch if necessary
    assert(this->cursor_ + length < this->data_.capacity());
    // memcpy into our buffer
    std::memcpy(this->data_.data() + this->cursor_, data, length);
    this->cursor_ += length;
}

void
Batch::flush()
{
    // copy CPU -> GPU memory
    glBindBuffer(GL_ARRAY_BUFFER, this->buffer_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->cursor_, this->data_.data());
    this->cursor_ = 0;
}

GLuint
Batch::handle() const noexcept
{
    return this->buffer_;
}

size_t
Batch::size() const noexcept
{
    return this->cursor_;
}

size_t
Batch::capacity() const noexcept
{
    return this->capacity_;
}

size_t
Batch::remaining() const noexcept
{
    return this->capacity_ - (this->cursor_ + 1);
}

} // namespace gfx