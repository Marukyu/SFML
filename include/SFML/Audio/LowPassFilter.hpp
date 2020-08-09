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

#ifndef SFML_LOWPASSFILTER_HPP
#define SFML_LOWPASSFILTER_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>
#include <SFML/Audio/SoundFilter.hpp>


namespace sf
{

////////////////////////////////////////////////////////////
/// \brief Audio filter that removes high frequencies
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API LowPassFilter : public SoundFilter
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    LowPassFilter();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~LowPassFilter();

    ////////////////////////////////////////////////////////////
    /// \brief Set the low pass filter's gain factor
    ///
    /// This influences the volume of the output signal.
    ///
    /// \param gain The gain factor to assign, between 0 and 1
    ///
    /// \see getGain
    ///
    ////////////////////////////////////////////////////////////
    void setGain(float gain);

    ////////////////////////////////////////////////////////////
    /// \brief Get the low pass filter's gain factor
    ///
    /// \return The current gain factor of the filter
    ///
    /// \see setGain
    ///
    ////////////////////////////////////////////////////////////
    float getGain() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the low pass filter's high frequency gain factor
    ///
    /// This influences how strongly high frequencies should be filtered out.
    ///
    /// \param gainHF The high frequency gain factor to assign, between 0 and 1
    ///
    /// \see getHighFrequencyGain
    ///
    ////////////////////////////////////////////////////////////
    void setHighFrequencyGain(float gainHF);

    ////////////////////////////////////////////////////////////
    /// \brief Get the low pass filter's high frequency gain factor
    ///
    /// \return The current high frequency gain factor of the filter
    ///
    /// \see setHighFrequencyGain
    ///
    ////////////////////////////////////////////////////////////
    float getHighFrequencyGain() const;
};

} // namespace sf


#endif // SFML_LOWPASSFILTER_HPP


////////////////////////////////////////////////////////////
/// \class sf::LowPassFilter
/// \ingroup audio
///
/// sf::LowPassFilter reduces high frequencies in an audio signal.
///
/// \see sf::SoundFilter
///
////////////////////////////////////////////////////////////
