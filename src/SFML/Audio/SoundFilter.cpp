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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/SoundFilter.hpp>
#include <SFML/Audio/ALCheck.hpp>

// Because we use OpenAL-Soft, we can assume that the EFX functions are always available.
#define AL_ALEXT_PROTOTYPES
#include <efx.h>


namespace sf
{

////////////////////////////////////////////////////////////
SoundFilter::SoundFilter() :
m_filter(0)
{
    alCheck(alGenFilters(1, &m_filter));
}


////////////////////////////////////////////////////////////
SoundFilter::~SoundFilter()
{
    alCheck(alDeleteFilters(1, &m_filter));
}


////////////////////////////////////////////////////////////
void SoundFilter::bind(unsigned int source)
{
    alCheck(alSourcei(source, AL_DIRECT_FILTER, m_filter));
}


////////////////////////////////////////////////////////////
void SoundFilter::unbind(unsigned int source)
{
    alCheck(alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL));
}


} // namespace sf
