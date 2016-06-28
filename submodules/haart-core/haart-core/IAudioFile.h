#ifndef _HISSTOOLS_IAUDIOFILE_
#define _HISSTOOLS_IAUDIOFILE_

#include "BaseAudioFile.h"
#include <fstream>

namespace HISSTools
{
    // FIX - check types, errors and returns
    
    class IAudioFile : public BaseAudioFile
    {
        enum AiffTag
        {
            AIFC_TAG_UNKNOWN = 0x0,
            AIFC_TAG_VERSION = 0x1,
            AIFC_TAG_COMMON = 0x2,
            AIFC_TAG_AUDIO = 0x4
        };
        enum AifcCompression
        {
            AIFC_COMPRESSION_UNKNOWN,
            AIFC_COMPRESSION_NONE,
            AIFC_COMPRESSION_LITTLE_ENDIAN,
            AIFC_COMPRESSION_FLOAT
        };

    public:
        
        // Constructor and Destructor
        
        IAudioFile(const std::string& = std::string());
        ~IAudioFile();
        
        // File Open / Close
        
        void open(const std::string& i);
        void close();
        bool isOpen();
        
        // File Position
        
        void seek(FrameCount position = 0);
        FrameCount getPosition();

        // File Reading
        
        void readRaw(void* output, FrameCount numFrames);

        void readInterleaved(double* output, FrameCount numFrames);
        void readInterleaved(float* output, FrameCount numFrames);

        void readChannel(double* output, FrameCount numFrames,
                         uint16_t channel);
        void readChannel(float* output, FrameCount numFrames, uint16_t channel);

    private:
        
        //  Internal File Handling
        
        bool readInternal(char* buffer, ByteCount numBytes);
        bool seekInternal(ByteCount position);
        bool advanceInternal(ByteCount offset);
        ByteCount positionInternal();

        //  Extracting Single Values
        
        uint64_t getU64(const char* bytes, Endianness fileEndianness) const;
        uint32_t getU32(const char* bytes, Endianness fileEndianness) const;
        uint32_t getU24(const char* bytes, Endianness fileEndianness) const;
        uint32_t getU16(const char* bytes, Endianness fileEndianness) const;

        //  Conversion
        
        double extendedToDouble(const char* bytes) const;
        template <class T> void u32ToOutput(T* output, uint32_t value);
        template <class T> void float32ToOutput(T* output, uint32_t value);
        template <class T> void float64ToOutput(T* output, uint64_t value);

        //  Chunk Reading
        
        static bool matchTag(const char* a, const char* b);
        bool readChunkHeader(char* tag, uint32_t& chunkSize);
        bool findChunk(const char* searchTag, uint32_t& chunkSize);
        bool readChunk(char* data, uint32_t readSize, uint32_t chunkSize);

        //  PCM Format Helpers
        
        static Error findPCMFormat(uint16_t, NumberFormat, PCMFormat&);
        void setPCMFormat(uint16_t bitDepth, NumberFormat format);

        //  AIFF Helpers
        
        bool getAIFFChunkHeader(AiffTag& enumeratedTag, uint32_t& chunkSize);
        AifcCompression getAIFCCompression(const char* tag) const;

        //  Parse Headers
        
        void parseHeader();
        void parseAIFFHeader(const char* fileSubtype);
        void parseWaveHeader(const char* fileType);

        //  Internal Typed Audio Read
        
        template <class T>
        void readAudio(T* output, FrameCount numFrames, int32_t channel = -1);
         
        //  Data
        
        std::ifstream mFile;
        char *mBuffer;
    };
}

#endif