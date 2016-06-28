#ifndef _HISSTOOLS_OAUDIOFILE_
#define _HISSTOOLS_OAUDIOFILE_

#include "BaseAudioFile.h"
#include <fstream>

namespace HISSTools
{
    class OAudioFile : public BaseAudioFile
    {
    public:
        OAudioFile();
        OAudioFile(const std::string&, FileType, PCMFormat,
                   uint16_t channels, double sr);
        OAudioFile(const std::string&, FileType, PCMFormat,
                   uint16_t channels, double sr, Endianness);
        
        ~OAudioFile();

        void open(const std::string&, FileType, PCMFormat,
                  uint16_t channels, double sr);
        void open(const std::string&, FileType, PCMFormat,
                  uint16_t channels, double sr, Endianness);
        void close();
        bool isOpen();
        void seek(FrameCount position = 0);
        FrameCount getPosition();

        void writeInterleaved(const double* input, FrameCount numFrames);
        void writeInterleaved(const float* input, FrameCount numFrames);
        
        void writeChannel(const double* input, FrameCount numFrames,
                         uint16_t channel);
        void writeChannel(const float* input, FrameCount numFrames,
                         uint16_t channel);
        void writeRaw(const char *input, FrameCount numFrames) { writePCMData(input, numFrames); }
            
    protected:
        
        ByteCount getHeaderSize() const;

    private:
        
        ByteCount positionInternal();
        bool seekInternal(ByteCount position);
        bool seekRelativeInternal(ByteCount offset);
        bool writeInternal(const char* buffer, ByteCount bytes);
        
        bool putU64(uint64_t value, Endianness fileEndianness);
        bool putU32(uint32_t value, Endianness fileEndianness);
        bool putU24(uint32_t value, Endianness fileEndianness);
        bool putU16(uint32_t value, Endianness fileEndianness);
        bool putU08(uint32_t value);
        
        bool putPadByte();

        bool putExtended(double);
        bool putPString(const char* string);

        bool putTag(const char* tag);
        bool putChunk(const char* tag, uint32_t size);

        void writeWaveHeader();
        void writeAIFCHeader();

        uint32_t inputToU32(double input, int bitDepth);
        
        bool resize(FrameCount numFrames);
        bool updateHeader();
        template <class T> void writeAudio(const T* input, FrameCount numFrames,
                                           int32_t channel = -1);
        bool writePCMData(const char* input, FrameCount numFrames);

        const char *getCompressionTag();
        const char *getCompressionString();
        
        //  Data

        std::ofstream mFile;
    };
}

#endif /* _HISSTOOLS_OAUDIOFILE_ */
