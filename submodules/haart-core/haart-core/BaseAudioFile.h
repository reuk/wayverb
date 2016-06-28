#ifndef _HISSTOOLS_BASEAUDIOFILE_
#define _HISSTOOLS_BASEAUDIOFILE_

#include <stdint.h>
#include <string>
#include <fstream>
#include <vector>

namespace HISSTools
{
    class BaseAudioFile
    {
        
    public:

        typedef uint32_t FrameCount;
        typedef uintptr_t ByteCount;
        
        enum FileType
        {
            kAudioFileNone,
            kAudioFileAIFF,
            kAudioFileAIFC,
            kAudioFileWAVE
        };
        enum PCMFormat
        {
            kAudioFileInt8,
            kAudioFileInt16,
            kAudioFileInt24,
            kAudioFileInt32,
            kAudioFileFloat32,
            kAudioFileFloat64
        };
        enum Endianness
        {
            kAudioFileLittleEndian,
            kAudioFileBigEndian
        };
        enum NumberFormat
        {
            kAudioFileInt,
            kAudioFileFloat
        };

        enum Error
        {
            ERR_NONE = 0,

            ERR_MEM_COULD_NOT_ALLOCATE = 1 << 0,

            ERR_FILE_ERROR = 1 << 1,
            ERR_FILE_COULDNT_OPEN = 1 << 2,
            ERR_FILE_BAD_FORMAT = 1 << 3,
            ERR_FILE_UNKNOWN_FORMAT = 1 << 4,
            ERR_FILE_UNSUPPORTED_PCM_FORMAT = 1 << 5,

            ERR_AIFC_WRONG_VERSION = 1 << 6,
            ERR_AIFC_UNSUPPORTED_FORMAT = 1 << 7,

            ERR_WAVE_UNSUPPORTED_FORMAT = 1 << 8,

            ERR_FILE_COULDNT_WRITE = 1 << 9,
        };

        enum AiffVersion
        {
            AIFC_CURRENT_SPECIFICATION = 0xA2805140
        };
        
        BaseAudioFile();
        virtual ~BaseAudioFile();
    
        FileType getFileType() const;
        PCMFormat getPCMFormat() const;
        Endianness getHeaderEndianness() const;
        Endianness getAudioEndianness() const;
        double getSamplingRate() const;
        uint16_t getChannels() const;
        FrameCount getFrames() const;
        uint16_t getBitDepth() const;
        uint16_t getByteDepth() const;
        ByteCount getFrameByteCount() const;
        NumberFormat getNumberFormat() const;

        static std::string getErrorString(Error error);
        static std::vector<Error> extractErrorsFromFlags(int flags);
        std::vector<Error> getErrors() const;
        
        int getErrorFlags() const;
        bool getIsError() const;
        void clearErrorFlags();

        static uint16_t findBitDepth(PCMFormat);
        static NumberFormat findNumberFormat(PCMFormat);

        virtual void close();
        virtual bool isOpen() = 0;
        virtual void seek(FrameCount position) = 0;
        virtual FrameCount getPosition() = 0;

    protected:
        
        ByteCount getPCMOffset() const;
        
        void setFileType(FileType);
        void setPCMFormat(PCMFormat);
        void setHeaderEndianness(Endianness);
        void setAudioEndianness(Endianness);
        void setSamplingRate(double);
        void setChannels(uint16_t);
        void setFrames(FrameCount);
        void setPCMOffset(ByteCount);

        void setErrorFlags(int flags);
        void setErrorBit(Error error);

    private:
        FileType mFileType;
        PCMFormat mPCMFormat;
        Endianness mHeaderEndianness;
        Endianness mAudioEndianness;

        double mSamplingRate;
        uint16_t mNumChannels;
        FrameCount mNumFrames;
        size_t mPCMOffset;

        int mErrorFlags;
    };
}

#endif 
