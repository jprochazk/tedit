
#include "pch.h"
#include "camera.hpp"
#include "window.hpp"

namespace gfx {

Camera::Camera(Window* window)
  : window_(window)
  , proj_()
  , view_()
  , pos_(0, 0)
  , viewport_(0, 0, window->width(), window->height())
  , zoom_(1)
{
    float hw = ((float)this->window_->width() / 2.f) / this->zoom_;
    float hh = ((float)this->window_->height() / 2.f) / this->zoom_;
    this->proj_ = glm::ortho(hw, -hw, -hh, hh, -1.f, 1.f);
    this->view_ = glm::lookAt(glm::vec3(this->pos_.x, this->pos_.y, 0.f),
        glm::vec3(this->pos_.x, this->pos_.y, 1.f),
        glm::vec3(0.f, 1.f, 0.f));
}

void
Camera::resize(int width, int height)
{
    float hw = (float)this->window_->width() / 2.f;
    float hh = (float)this->window_->height() / 2.f;
    this->proj_ = glm::ortho(hw, -hw, -hh, hh, -1.f, 1.f);
    this->viewport_.z = this->window_->width();
    this->viewport_.w = this->window_->height();
}

void
Camera::move(glm::vec2 to)
{
    this->pos_ = to;
    this->view_ = glm::lookAt(glm::vec3(this->pos_.x, this->pos_.y, 0.f),
        glm::vec3(this->pos_.x, this->pos_.y, 1.f),
        glm::vec3(0.f, 1.f, 0.f));
}

void
Camera::zoom(float wheel)
{
    this->zoom_ = std::clamp(this->zoom_ + wheel / 10, 0.1f, 20.f);
    float hw = ((float)this->window_->width() / 2.f) / this->zoom_;
    float hh = ((float)this->window_->height() / 2.f) / this->zoom_;
    this->proj_ = glm::ortho(hw, -hw, -hh, hh, -1.f, 1.f);
}

void
Camera::zoom(float wheel, glm::vec2 mouse)
{
    auto prev_mw = this->world(mouse);
    this->zoom(wheel);
    auto curr_mw = this->world(mouse);
    this->move(this->pos_ + (prev_mw - curr_mw));
}

glm::vec2
Camera::world(glm::vec2 mouse) const
{
    return glm::unProject(
        glm::vec3(mouse.x, this->viewport_.w - mouse.y, 1), this->view_, this->proj_, this->viewport_);
}

const glm::mat4&
Camera::projection() const
{
    return this->proj_;
}

const glm::mat4&
Camera::view() const
{
    return this->view_;
}

const glm::vec2&
Camera::pos() const
{
    return this->pos_;
}

float
Camera::zoom() const
{
    return this->zoom_;
}

} // namespace gfx