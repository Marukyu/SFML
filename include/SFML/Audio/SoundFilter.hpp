////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2019 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

// Additional file by Marukyu for World of Sand

#ifndef SFML_SOUNDFILTER_HPP
#define SFML_SOUNDFILTER_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>


namespace sf
{

////////////////////////////////////////////////////////////
/// \brief Base class for sound filters (effects)
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundFilter
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~SoundFilter();

protected:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundFilter();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int m_filter; //!< OpenAL handle for the filter

private:

    ////////////////////////////////////////////////////////////
    /// \brief Activates the filter on the specified OpenAL source
    ///
    /// \param source The OpenAL identifier of the source to bind the filter to
    ///
    ////////////////////////////////////////////////////////////
    void bind(unsigned int source);

    ////////////////////////////////////////////////////////////
    /// \brief Deactivates the filter on the specified OpenAL source
    ///
    /// \param source The OpenAL identifier of the source to unbind the filter from
    ///
    ////////////////////////////////////////////////////////////
    void unbind(unsigned int source);

    friend class SoundSource;
};

} // namespace sf


#endif // SFML_SOUNDFILTER_HPP


////////////////////////////////////////////////////////////
/// \class sf::SoundFilter
/// \ingroup audio
///
/// sf::SoundFilter is the abstract base class for real-time audio filters.
///
/// \see sf::SoundSource
///
////////////////////////////////////////////////////////////
