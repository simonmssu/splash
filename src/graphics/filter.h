/*
 * Copyright (C) 2015 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @filter.h
 * The Filter class
 */

#ifndef SPLASH_FILTER_H
#define SPLASH_FILTER_H

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "./core/constants.h"

#include "./core/attribute.h"
#include "./graphics/framebuffer.h"
#include "./graphics/object.h"
#include "./graphics/texture.h"
#include "./graphics/texture_image.h"
#include "./image/image.h"

namespace Splash
{

/*************/
class Filter : public Texture
{
  public:
    /**
     *  Constructor
     * \param root Root object
     */
    Filter(RootObject* root);

    /**
     * No copy constructor, but a move one
     */
    Filter(const Filter&) = delete;
    Filter(Filter&&) = default;
    Filter& operator=(const Filter&) = delete;

    /**
     *  Bind the filter
     */
    void bind() override;

    /**
     *  Unbind the filter
     */
    void unbind() override;

    /**
     * Get the shader parameters related to this texture
     * Texture should be locked first
     */
    std::unordered_map<std::string, Values> getShaderUniforms() const override;

    /**
     *  Get specs of the texture
     * \return Return the texture specs
     */
    ImageBufferSpec getSpec() const override { return _fbo->getColorTexture()->getSpec(); }

    /**
     *  Get the id of the gl texture
     * \return Return the texture id
     */
    GLuint getTexId() const override { return _fbo->getColorTexture()->getTexId(); }

    /**
     * Set whether to keep the input image ratio
     * \param keepRatio Keep ratio if true
     */
    void setKeepRatio(bool keepRatio);

    /**
     * Set the bit depth to 16 bpc
     * \param active If true, set to 16bpc, otherwise 8bpc
     */
    void setSixteenBpc(bool active);

    /**
     *  Render the filter
     */
    void render() override;

  protected:
    std::vector<std::weak_ptr<Texture>> _inTextures;
    std::shared_ptr<Object> _screen;
    std::unique_ptr<Framebuffer> _fbo{nullptr};

    bool _shaderAttributesRegistered{false};
    std::unordered_map<std::string, Values> _filterUniforms; //!< Contains all filter uniforms

    /**
     *  Try to link the given GraphObject to this object
     * \param obj Shared pointer to the (wannabe) child object
     */
    bool linkIt(const std::shared_ptr<GraphObject>& obj) final;

    /**
     *  Try to unlink the given GraphObject from this object
     * \param obj Shared pointer to the (supposed) child object
     */
    void unlinkIt(const std::shared_ptr<GraphObject>& obj) final;

    /**
     *  Updates the shader uniforms according to the textures and images the filter is connected to.
     */
    virtual void updateUniforms();

    /**
     *  Register new functors to modify attributes
     */
    virtual void registerAttributes();

  private:

    // Filter parameters
    static constexpr int _defaultSize[2]{512, 512};
    int _sizeOverride[2]{-1, -1}; //!< If set to positive values, overrides the size given by input textures
    bool _keepRatio{false};
    bool _sixteenBpc{true};

    // Mipmap capture
    int _grabMipmapLevel{-1};
    Value _mipmapBuffer{};
    Values _mipmapBufferSpec{};

    /**
     * Update the size override to take (or not) ratio into account
     */
    void updateSizeWrtRatio();

    /**
     * Register attributes related to the default shader
     */
    virtual void registerDefaultShaderAttributes();
};

} // namespace Splash

#endif // SPLASH_FILTER_H
