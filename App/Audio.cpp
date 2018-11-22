//
// Created by Taylor Whatley on 2018-10-15.
//

#include "App.h"

#ifndef CPU_ONLY
#ifndef NO_AUDIO
namespace Nem {
    bool Audio::init() {
        alcMakeContextCurrent(context);

        vector<ALbyte> data = vector<ALbyte>(100000);

        ALbyte repeat[] = {
                -100, -120, -100, 0, 0, 100, 120, 100
        };

        for (int a = 0; a < data.size(); a++) {
            data[a] = repeat[a % sizeof(repeat)];
        }

        alGenBuffers(1, &testBuffer);
        alBufferData(testBuffer, AL_FORMAT_MONO8, &data[0], (ALsizei)data.size(), 1000);

        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, testBuffer);
        alSourcePlay(source);

        return true;
    }
    void Audio::loop() {
        while (!stopExecution) {

        }
    }

    void Audio::close() {
        alDeleteSources(1, &source);
        alDeleteBuffers(1, &testBuffer);
    }

    void Audio::exec() {
        if (init()) loop();
        close();
    }
    void Audio::stopExec() { stopExecution = true; }

    Audio::Audio(APU* nApu) : apu(nApu) {
        device = alcOpenDevice(nullptr);
        if (!device) throw CouldNotInitializeAudioException();
        context = alcCreateContext(device, nullptr);
    }

    Audio::~Audio() {
        alcDestroyContext(context);
        alcCloseDevice(device);
    }
}
#endif
#endif