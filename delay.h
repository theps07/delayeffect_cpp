#include "AudioFile.h"

class Delay
{
private:
public:
    AudioFile<double> input; // Input audio file data
    AudioFile<double> output;
    AudioFile<double>::AudioBuffer delayBuffer; // Delay buffer
    std::string in_fileName;                    // Input filename
    float maxDelayTime = 1.0;
    double leftDelayTime;     // Left delaytime
    double rightDelayTime;    // Left delaytime
    double feedback;          // Feedback for both channels
    int nTaps;                // Number of taps for multi-tap mode
    double dryMix;            // Amount of dry signal
    std::string mode;         // Mode selector
    std::string out_fileName; // Filename to save output to
    enum Mode                 // Available modes
    {
        STEREO,
        PING_PONG,
        MULTI_TAP
    };
    Mode currentMode; // Current mode

    void initialize(std::string *fileName)
    {
        // Function to load audio file data from filename
        this->input.load(*fileName);
        this->output.setAudioBufferSize(this->input.getNumChannels(),
                                        this->input.getNumSamplesPerChannel() * 2);
        this->in_fileName = *fileName;
    };

    void setParams()
    {
        // Function to collect params for various modes
        std::cout << "Enter delay time for left channel (<= 1s): ";
        std::cin >> this->leftDelayTime;

        std::cout << "Enter delay time for right channel (<= 1s): ";
        std::cin >> this->rightDelayTime;

        std::cout << "Enter feedback for both channels (< 0.99): ";
        std::cin >> this->feedback;

        std::cout << "Enter dry mix (< 1): ";
        std::cin >> this->dryMix;

        std::cout << "Enter mode ('s' for stereo, 'p' for ping-pong, 'm' for multitap): ";
        std::cin >> this->mode;

        if (this->mode == "s")
        {
            this->currentMode = STEREO;
        }
        else if (this->mode == "p")
        {
            this->currentMode = PING_PONG;
        }
        else if (this->mode == "m")
        {
            this->currentMode = MULTI_TAP;
        }

        if (this->currentMode == MULTI_TAP)
        {
            std::cout << "Enter number of taps: ";
            std::cin >> this->nTaps;
        }

        std::cout << "Enter output filename: ";
        std::cin >> this->out_fileName;
    };

    void processStereo()
    {
        // Function for stereo delay
        double wetMix = 1 - this->dryMix;

        // Variables set externally
        int sampleRate = this->input.getSampleRate();           // Audio samplerate
        int numFrames = this->output.getNumSamplesPerChannel(); // Number of samples
        int numChannels = this->output.getNumChannels();        // Number of channels

        int delayBufLength = int(floor(2 * this->maxDelayTime * sampleRate)); // Delay buffer length

        this->delayBuffer.resize(2); // Set to two channels
        // Set number of samples per channel
        this->delayBuffer[0].resize(delayBufLength);
        this->delayBuffer[1].resize(delayBufLength);

        // Vectors to hold delay time and feedback for both channels
        std::vector<double> delayTime{this->leftDelayTime, this->rightDelayTime};

        int l_dpr = 0; // Read pointer (left channel)
        int r_dpr = 0; // Read pointer (right channel)
        std::vector<int> dpr{l_dpr, r_dpr};

        int l_dpw = int(dpr[0] + floor(delayTime[0] * sampleRate)); // Write pointer (left channel)
        int r_dpw = int(dpr[1] + floor(delayTime[1] * sampleRate)); // Write pointer (right channel)
        std::vector<int> dpw{l_dpw, r_dpw};

        for (int frames = 0; frames < numFrames; frames++)
        {
            for (int chans = 0; chans < numChannels; chans++)
            {
                double in_;
                if (frames < this->input.getNumSamplesPerChannel())
                {
                    in_ = this->input.samples[chans][frames];
                }
                else
                {
                    in_ = 0.0;
                }
                double out_ = 0.0;

                // The output is the input plus the contents of the
                // delay buffer (weighted by the mix levels).
                out_ = this->dryMix * in_ + wetMix * this->delayBuffer[chans][dpr[chans]];

                // Store the current information in the delay buffer.
                // this->delayBuffer[chans][dpr[chans]] is the delay
                // sample we just read, i.e. what came out of the buffer.
                // this->delayBuffer[chans][dpw[chans]] is what we
                // write to the buffer, i.e. what goes in.
                this->delayBuffer[chans][dpw[chans]] = in_ + this->delayBuffer[chans][dpr[chans]] * this->feedback;

                // Circular delay buffer
                if (dpr[chans]++ >= delayBufLength)
                    dpr[chans] = 0;

                if (dpw[chans]++ >= delayBufLength)
                    dpw[chans] = 0;

                // Store output sample in buffer, replacing input
                this->output.samples[chans][frames] = out_;
            }
        }

        // Save output
        this->output.save(this->out_fileName, AudioFileFormat::Wave);
        return;
    };

    void processPingPong()
    {
        double wetMix = 1 - this->dryMix;

        // Variables set externally
        int sampleRate = this->input.getSampleRate();           // Audio samplerate
        int numFrames = this->output.getNumSamplesPerChannel(); // Number of samples
        int numChannels = this->output.getNumChannels();        // Number of channels

        int delayBufLength = int(floor(2 * this->maxDelayTime * sampleRate)); // Delay buffer length
        AudioFile<double>::AudioBuffer delayBuffer;                           // Delay buffer readd

        delayBuffer.resize(2); // Set to two channels
        // Set number of samples per channel
        delayBuffer[0].resize(delayBufLength);
        delayBuffer[1].resize(delayBufLength);

        // Vectors to hold delay time and feedback for both channels
        std::vector<double> delayTime{this->leftDelayTime, this->rightDelayTime};

        int l_dpr = 0; // Read pointer (left channel)
        int r_dpr = 0; // Read pointer (right channel)
        std::vector<int> dpr{l_dpr, r_dpr};

        int l_dpw = int(dpr[0] + floor(delayTime[0] * sampleRate)); // Write pointer (left channel)
        int r_dpw = int(dpr[1] + floor(delayTime[1] * sampleRate)); // Write pointer (right channel)
        std::vector<int> dpw{l_dpw, r_dpw};

        for (int frames = 0; frames < numFrames; frames++)
        {
            for (int chans = 0; chans < numChannels; chans++)
            {
                double in_ = frames < this->input.getNumSamplesPerChannel()
                                 ? this->input.samples[chans][frames]
                                 : 0.0;

                double out_ = 0.0;

                // The output is the input plus the contents of the
                // delay buffer (weighted by the mix levels).
                out_ = this->dryMix * in_ + wetMix * delayBuffer[chans][dpr[chans]];

                // Store the current information in the delay buffer.
                // delayBuffer[chans][dpr[chans]] is the delay
                // sample we just read, i.e. what came out of the buffer.
                // delayBuffer[chans][dpw[chans]] is what we
                // write to the buffer, i.e. what goes in.
                //  Write channel 1 (R) data to channel 0 (L) and vice-versa
                delayBuffer[chans][dpw[chans]] = in_ + delayBuffer[-1 * chans + 1][dpr[chans]] * this->feedback;

                // Circular buffer
                if (++dpr[chans] >= delayBufLength)
                    dpr[chans] = 0;

                if (++dpw[chans] >= delayBufLength)
                    dpw[chans] = 0;

                // Store output sample in buffer, replacing input
                this->output.samples[chans][frames] = out_;
            }
        }

        // Save output
        this->output.save(this->out_fileName, AudioFileFormat::Wave);
        return;
    };

    void processMultiTap()
    {
        double wetMix = 1 - this->dryMix;

        // Variables set externally
        int sampleRate = this->input.getSampleRate();           // Audio samplerate
        int numFrames = this->output.getNumSamplesPerChannel(); // Number of samples
        int numChannels = this->output.getNumChannels();        // Number of channels

        int delayBufLength = int(floor(2 * this->maxDelayTime * sampleRate)); // Delay buffer length

        this->delayBuffer.resize(2); // Set to two channels
        // Set number of samples per channel
        this->delayBuffer[0].resize(delayBufLength);
        this->delayBuffer[1].resize(delayBufLength);

        // Vectors to hold delay time and feedback for both channels
        std::vector<double> delayTime{this->leftDelayTime, this->rightDelayTime};

        // 2-D vector to hold multiple read pointers (taps)
        std::vector<std::vector<int>> tapPtr;

        // tapPtr constructor
        for (int taps = 0; taps < this->nTaps; taps++)
        {
            std::vector<int> element;

            for (int chans = 0; chans < numChannels; chans++)
            {
                element.push_back(int(floor(taps * sampleRate * (delayTime[chans] / this->nTaps))));
            }

            tapPtr.push_back(element);
        }

        int l_dpw = int(floor(delayTime[0] * sampleRate)); // Write pointer (left channel)
        int r_dpw = int(floor(delayTime[1] * sampleRate)); // Write pointer (right channel)
        std::vector<int> dpw{l_dpw, r_dpw};

        for (int frames = 0; frames < numFrames; frames++)
        {
            for (int chans = 0; chans < numChannels; chans++)
            {
                double in_ = frames < this->input.getNumSamplesPerChannel()
                                 ? this->input.samples[chans][frames]
                                 : 0.0;
                double dBuf_ = in_;
                double out_ = this->dryMix * in_;

                // The output is the input plus the contents of the
                // delay buffer (weighted by the mix levels).
                for (int tap = 0; tap < this->nTaps; tap++)
                {
                    // Store the current information in the delay buffer.
                    // delayBuffer[chans][tapPtr[tap][chans]] is the delay
                    // sample we just read, i.e. what came out of the buffer.
                    // Data is read at diffrent read pointers and added to out_.
                    // dBuf is what we write to the buffer, i.e. what goes in.
                    out_ += wetMix * delayBuffer[chans][tapPtr[tap][chans]];
                    dBuf_ += delayBuffer[chans][tapPtr[tap][chans]] * (this->feedback / this->nTaps);

                    if (++tapPtr[tap][chans] >= delayBufLength)
                        tapPtr[tap][chans] = 0;
                }

                delayBuffer[chans][dpw[chans]] = dBuf_;

                if (++dpw[chans] >= delayBufLength)
                    dpw[chans] = 0;

                // Store output sample in buffer, replacing input
                this->output.samples[chans][frames] = out_;
            }
        }

        this->output.save(this->out_fileName, AudioFileFormat::Wave);
        return;
    };

    void process()
    {
        switch (this->currentMode)
        {
        case STEREO:
            this->processStereo();
            break;

        case PING_PONG:
            this->processPingPong();
            break;
        case MULTI_TAP:
            this->processMultiTap();
        }
    };
};
