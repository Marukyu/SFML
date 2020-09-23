////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2020 Laurent Gomila (laurent@sfml-dev.org)
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

// Adapted by Marukyu for World of Sand

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/SoundSource.hpp>
#include <SFML/Audio/ALCheck.hpp>
#include <SFML/Audio/SoundFilter.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <cstdlib>
#include <vector>


namespace sf
{
////////////////////////////////////////////////////////////
SoundSource::SoundSource() :
    m_source(0),
    m_filter(NULL)
{
    alCheck(alGenSources(1, &m_source));
    alCheck(alSourcei(m_source, AL_BUFFER, 0));
}


////////////////////////////////////////////////////////////
SoundSource::SoundSource(const SoundSource& copy)
{
    alCheck(alGenSources(1, &m_source));
    alCheck(alSourcei(m_source, AL_BUFFER, 0));

    setPitch(copy.getPitch());
    setVolume(copy.getVolume());
    setPosition(copy.getPosition());
    setRelativeToListener(copy.isRelativeToListener());
    setMinDistance(copy.getMinDistance());
    setAttenuation(copy.getAttenuation());
}


////////////////////////////////////////////////////////////
SoundSource::~SoundSource()
{
    alCheck(alSourcei(m_source, AL_BUFFER, 0));
    alCheck(alDeleteSources(1, &m_source));
}


////////////////////////////////////////////////////////////
void SoundSource::setPitch(float pitch)
{
    alCheck(alSourcef(m_source, AL_PITCH, pitch));
}


////////////////////////////////////////////////////////////
void SoundSource::setVolume(float volume)
{
    alCheck(alSourcef(m_source, AL_GAIN, volume * 0.01f));
}


////////////////////////////////////////////////////////////
void SoundSource::setPosition(float x, float y, float z)
{
    alCheck(alSource3f(m_source, AL_POSITION, x, y, z));
}


////////////////////////////////////////////////////////////
void SoundSource::setPosition(const Vector3f& position)
{
    setPosition(position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setRelativeToListener(bool relative)
{
    alCheck(alSourcei(m_source, AL_SOURCE_RELATIVE, relative));
}


////////////////////////////////////////////////////////////
void SoundSource::setMinDistance(float distance)
{
    alCheck(alSourcef(m_source, AL_REFERENCE_DISTANCE, distance));
}


////////////////////////////////////////////////////////////
void SoundSource::setAttenuation(float attenuation)
{
    alCheck(alSourcef(m_source, AL_ROLLOFF_FACTOR, attenuation));
}


////////////////////////////////////////////////////////////
float SoundSource::getPitch() const
{
    ALfloat pitch;
    alCheck(alGetSourcef(m_source, AL_PITCH, &pitch));

    return pitch;
}


////////////////////////////////////////////////////////////
float SoundSource::getVolume() const
{
    ALfloat gain;
    alCheck(alGetSourcef(m_source, AL_GAIN, &gain));

    return gain * 100.f;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getPosition() const
{
    Vector3f position;
    alCheck(alGetSource3f(m_source, AL_POSITION, &position.x, &position.y, &position.z));

    return position;
}


////////////////////////////////////////////////////////////
bool SoundSource::isRelativeToListener() const
{
    ALint relative;
    alCheck(alGetSourcei(m_source, AL_SOURCE_RELATIVE, &relative));

    return relative != 0;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinDistance() const
{
    ALfloat distance;
    alCheck(alGetSourcef(m_source, AL_REFERENCE_DISTANCE, &distance));

    return distance;
}


////////////////////////////////////////////////////////////
float SoundSource::getAttenuation() const
{
    ALfloat attenuation;
    alCheck(alGetSourcef(m_source, AL_ROLLOFF_FACTOR, &attenuation));

    return attenuation;
}


////////////////////////////////////////////////////////////
SoundSource& SoundSource::operator =(const SoundSource& right)
{
    // Leave m_source untouched -- it's not necessary to destroy and
    // recreate the OpenAL sound source, hence no copy-and-swap idiom

    // Assign the sound attributes
    setPitch(right.getPitch());
    setVolume(right.getVolume());
    setPosition(right.getPosition());
    setRelativeToListener(right.isRelativeToListener());
    setMinDistance(right.getMinDistance());
    setAttenuation(right.getAttenuation());

    return *this;
}


////////////////////////////////////////////////////////////
SoundSource::Status SoundSource::getStatus() const
{
    ALint status;
    alCheck(alGetSourcei(m_source, AL_SOURCE_STATE, &status));

    switch (status)
    {
        case AL_INITIAL:
        case AL_STOPPED: return Stopped;
        case AL_PAUSED:  return Paused;
        case AL_PLAYING: return Playing;
    }

    return Stopped;
}


////////////////////////////////////////////////////////////
void SoundSource::setFilter(SoundFilter* filter)
{
    // Unbind any existing filter
    if (m_filter)
    {
        m_filter->unbind(m_source);
    }

    m_filter = filter;

    // Bind the new filter
    if (m_filter)
    {
        m_filter->bind(m_source);
    }
}


////////////////////////////////////////////////////////////
SoundFilter* SoundSource::getFilter() const
{
    return m_filter;
}


////////////////////////////////////////////////////////////
void SoundSource::synchronize(Status status, SoundSource* const* sources, unsigned int sourceCount)
{
    if (status == Stopped)
    {
        // Stopping does not need any special synchronization
        for (Uint32 i = 0; i < sourceCount; ++i)
        {
            sources[i]->stop();
        }
        return;
    }

    // Check if any of the sources are currently stopped. This will require a full timestamped resynchronization
    for (Uint32 i = 0; i < sourceCount; ++i)
    {
        if (sources[i]->getStatus() == Stopped)
        {
            synchronize(status, sf::Time::Zero, sources, sourceCount);
            return;
        }
    }

    // Perform the actual synchronized playback status update
    synchronizeImpl(status, sources, sourceCount);
}


////////////////////////////////////////////////////////////
void SoundSource::synchronize(Status status, Time timeOffset, SoundSource* const* sources, unsigned int sourceCount)
{
    if (status == Stopped)
    {
        // Stopping does not need any special synchronization
        for (Uint32 i = 0; i < sourceCount; ++i)
        {
            sources[i]->stop();
        }
        return;
    }

    // Update the playback time for all sources and prepare their playback threads (if necessary)
    for (Uint32 i = 0; i < sourceCount; ++i)
    {
        sources[i]->prepareSynchronizedPlayback(timeOffset);
    }

    // Wait for the playback threads of all sources to be initialized
    bool allReady = false;
    while (!allReady)
    {
        allReady = true;

        for (Uint32 i = 0; i < sourceCount; ++i)
        {
            if (!sources[i]->isSynchronizedPlaybackReady())
            {
                allReady = false;
                sleep(milliseconds(5));
                break;
            }
        }
    }

    // Perform the actual synchronized playback status update
    synchronizeImpl(status, sources, sourceCount);
}


////////////////////////////////////////////////////////////
void SoundSource::synchronizeImpl(Status status, SoundSource* const* sources, unsigned int sourceCount)
{
    // Create a buffer containing all OpenAL source IDs
    std::vector<ALuint> sourceIDs(sourceCount);
    for (Uint32 i = 0; i < sourceCount; ++i)
    {
        sourceIDs[i] = sources[i]->m_source;
    }

    // Update the OpenAL playback status of all sources at once
    if (status == Playing)
    {
        alCheck(alSourcePlayv(sourceIDs.size(), sourceIDs.data()));
    }
    else
    {
        alCheck(alSourcePausev(sourceIDs.size(), sourceIDs.data()));
    }
}


////////////////////////////////////////////////////////////
void SoundSource::prepareSynchronizedPlayback(Time timeOffset)
{
    // Nothing special needs to be done by default.
}


////////////////////////////////////////////////////////////
bool SoundSource::isSynchronizedPlaybackReady() const
{
    // Synchronized playback is always ready by default.
    return true;
}

} // namespace sf
