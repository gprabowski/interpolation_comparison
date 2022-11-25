#pragma once

#include <geometry.hpp>
#include <glfw_impl/common.hpp>
#include <logger.hpp>

namespace pusn {

namespace glfw_impl {

struct frambuffer {
  uint32_t width{1};
  uint32_t height{1};

  renderable meta;
  api_agnostic_geometry geom;
  std::optional<GLuint> of_fb;
  std::optional<GLuint> color;
  std::optional<GLuint> depth;

  // frambuffer utils
  void setup() {
    if (!of_fb.has_value()) {
      GLuint tmp;
      glCreateFramebuffers(1, &tmp);
      of_fb = tmp;
    }

    width = std::max<float>(1.0f, width);
    height = std::max<float>(1.0f, height);

    if (color.has_value() && depth.has_value()) {
      GLuint textures[] = {color.value(), depth.value()};
      glNamedFramebufferTexture(of_fb.value(), GL_COLOR_ATTACHMENT0, 0, 0);
      glNamedFramebufferTexture(of_fb.value(), GL_DEPTH_STENCIL_ATTACHMENT, 0,
                                0);
      glDeleteTextures(2, textures);
    }

    GLuint tmp_c, tmp_d;
    glCreateTextures(GL_TEXTURE_2D, 1, &tmp_c);
    glCreateTextures(GL_TEXTURE_2D, 1, &tmp_d);

    color = tmp_c;
    depth = tmp_d;

    // color
    glTextureParameteri(color.value(), GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(color.value(), GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(color.value(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(color.value(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(color.value(), 1, GL_RGBA8, width, height);

    // depth
    glTextureParameteri(depth.value(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depth.value(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depth.value(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(depth.value(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(depth.value(), 1, GL_DEPTH24_STENCIL8, width, height);

    // final setup
    glNamedFramebufferTexture(of_fb.value(), GL_COLOR_ATTACHMENT0,
                              color.value(), 0);
    glNamedFramebufferTexture(of_fb.value(), GL_DEPTH_STENCIL_ATTACHMENT,
                              depth.value(), 0);
    if (glCheckNamedFramebufferStatus(of_fb.value(), GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
      LOGGER_CRITICAL("Framebuffer creation failed!");
    }
    GLenum draw_bufs[] = {GL_COLOR_ATTACHMENT0};
    glNamedFramebufferDrawBuffers(of_fb.value(), 1, draw_bufs);
  }

  void bind() { glBindFramebuffer(GL_FRAMEBUFFER, of_fb.value()); }
  void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};
} // namespace glfw_impl
} // namespace pusn
