////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2008 Laurent Gomila (laurent.gom@gmail.com)
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

#ifndef SFML_SOUNDSTREAM_HPP
#define SFML_SOUNDSTREAM_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Thread.hpp>
#include <cstdlib>


namespace sf
{
////////////////////////////////////////////////////////////
/// SoundStream is a streamed sound, ie samples are acquired
/// while the sound is playing. Use it for big sounds that would
/// require hundreds of MB in memory (see sfMusic),
/// or for streaming sound from the network
////////////////////////////////////////////////////////////
class SFML_API SoundStream : private Thread, private Sound
{
public :

    using Sound::Status;
    using Sound::Stopped;
    using Sound::Paused;
    using Sound::Playing;
    using Sound::Pause;
    using Sound::SetPitch;
    using Sound::SetVolume;
    using Sound::SetPosition;
    using Sound::SetMinDistance;
    using Sound::SetAttenuation;
    using Sound::GetPitch;
    using Sound::GetVolume;
    using Sound::GetPosition;
    using Sound::GetMinDistance;
    using Sound::GetAttenuation;

    ////////////////////////////////////////////////////////////
    /// Structure defining a chunk of audio data to stream
    ////////////////////////////////////////////////////////////
    struct Chunk
    {
        const Int16* Samples;   ///< Pointer to the audio samples
        std::size_t  NbSamples; ///< Number of samples pointed by Samples
    };

    ////////////////////////////////////////////////////////////
    /// Virtual destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~SoundStream();

    ////////////////////////////////////////////////////////////
    /// Start playing the audio stream
    ///
    ////////////////////////////////////////////////////////////
    void Play();

    ////////////////////////////////////////////////////////////
    /// Stop playing the audio stream
    ///
    ////////////////////////////////////////////////////////////
    void Stop();

    ////////////////////////////////////////////////////////////
    /// Return the number of channels (1 = mono, 2 = stereo)
    ///
    /// \return Number of channels
    ///
    ////////////////////////////////////////////////////////////
    unsigned int GetChannelsCount() const;

    ////////////////////////////////////////////////////////////
    /// Get the stream sample rate
    ///
    /// \return Stream frequency (number of samples per second)
    ///
    ////////////////////////////////////////////////////////////
    unsigned int GetSampleRate() const;

    ////////////////////////////////////////////////////////////
    /// Get the status of the stream (stopped, paused, playing)
    ///
    /// \return Current status of the sound
    ///
    ////////////////////////////////////////////////////////////
    Status GetStatus() const;

protected :

    ////////////////////////////////////////////////////////////
    /// Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundStream();

    ////////////////////////////////////////////////////////////
    /// Set the audio stream parameters, you must call it before Play()
    ///
    /// \param ChannelsCount : Number of channels
    /// \param SampleRate :    Sample rate
    ///
    ////////////////////////////////////////////////////////////
    void Initialize(unsigned int ChannelsCount, unsigned int SampleRate);

private :

    ////////////////////////////////////////////////////////////
    /// /see Thread::Run
    ///
    ////////////////////////////////////////////////////////////
    virtual void Run();

    ////////////////////////////////////////////////////////////
    /// Called when the sound restarts
    ///
    /// \return If false is returned, the playback is aborted
    ///
    ////////////////////////////////////////////////////////////
    virtual bool OnStart();

    ////////////////////////////////////////////////////////////
    /// Called each time new audio data is needed to feed the stream
    ///
    /// \param Data : New chunk of data to stream
    ///
    /// \return True to continue playback, false to stop
    ///
    ////////////////////////////////////////////////////////////
    virtual bool OnGetData(Chunk& Data) = 0;

    ////////////////////////////////////////////////////////////
    /// Clean up stream internal resources
    ///
    ////////////////////////////////////////////////////////////
    void CleanUp();

    enum {BuffersCount = 3};

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    bool          myIsStreaming;           ///< Streaming state (true = playing, false = stopped)
    unsigned int  myBuffers[BuffersCount]; ///< Sound buffers used to store temporary audio data
    unsigned int  myChannelsCount;         ///< Number of channels (1 = mono, 2 = stereo, ...)
    unsigned int  mySampleRate;            ///< Frequency (samples / second)
    unsigned long myFormat;                ///< Format of the internal sound buffers
};

} // namespace sf


#endif // SFML_SOUNDSTREAM_HPP
