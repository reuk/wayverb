#include "BaseAudioFile.h"

namespace HISSTools
{
    BaseAudioFile::BaseAudioFile()
    {
        close();
    }

    BaseAudioFile::~BaseAudioFile()
    {
    }

    void BaseAudioFile::close()
    {
        setFileType(kAudioFileNone);
        BaseAudioFile::setPCMFormat(kAudioFileInt8);
        setHeaderEndianness(kAudioFileLittleEndian);
        setAudioEndianness(kAudioFileLittleEndian);
        setSamplingRate(0);
        setChannels(0);
        setFrames(0);
        setPCMOffset(0);
        clearErrorFlags();
    }

    BaseAudioFile::FileType BaseAudioFile::getFileType() const
    {
        return mFileType;
    }

    BaseAudioFile::PCMFormat BaseAudioFile::getPCMFormat() const
    {
        return mPCMFormat;
    }
    
    BaseAudioFile::Endianness BaseAudioFile::getHeaderEndianness() const
    {
        return mHeaderEndianness;
    }
    
    BaseAudioFile::Endianness BaseAudioFile::getAudioEndianness() const
    {
        return mAudioEndianness;
    }
    
    double BaseAudioFile::getSamplingRate() const
    {
        return mSamplingRate;
    }
    
    uint16_t BaseAudioFile::getChannels() const
    {
        return mNumChannels;
    }
    
    BaseAudioFile::FrameCount BaseAudioFile::getFrames() const
    {
        return mNumFrames;
    }
    
    BaseAudioFile::ByteCount BaseAudioFile::getPCMOffset() const
    {
        return mPCMOffset;
    }
    
    uint16_t BaseAudioFile::getBitDepth() const
    {
        return findBitDepth(getPCMFormat());
    }
    
    uint16_t BaseAudioFile::getByteDepth() const
    {
        return getBitDepth() / 8;
    }
    
    BaseAudioFile::ByteCount BaseAudioFile::getFrameByteCount() const
    {
        return getChannels() * getByteDepth();
    }
    
    BaseAudioFile::NumberFormat BaseAudioFile::getNumberFormat() const
    {
        return findNumberFormat(getPCMFormat());
    }

    int BaseAudioFile::getErrorFlags() const
    {
        return mErrorFlags;
    }
    
    std::string BaseAudioFile::getErrorString(Error error)
    {
        switch (error)
        {
            case ERR_NONE:
                return "no error";
            case ERR_MEM_COULD_NOT_ALLOCATE:
                return "mem could not allocate";
            case ERR_FILE_ERROR:
                return "file error";
            case ERR_FILE_COULDNT_OPEN:
                return "file couldn't open";
            case ERR_FILE_BAD_FORMAT:
                return "file bad format";
            case ERR_FILE_UNKNOWN_FORMAT:
                return "file unknown format";
            case ERR_FILE_UNSUPPORTED_PCM_FORMAT:
                return "file unsupported pcm format";
            case ERR_AIFC_WRONG_VERSION:
                return "aifc wrong version";
            case ERR_AIFC_UNSUPPORTED_FORMAT:
                return "aifc unsupported format";
            case ERR_WAVE_UNSUPPORTED_FORMAT:
                return "wave unsupported format";
            case ERR_FILE_COULDNT_WRITE:
                return "file couldn't write";
        }
    }
    std::vector<BaseAudioFile::Error> BaseAudioFile::extractErrorsFromFlags(int flags)
    {
        std::vector<Error> ret;
        for (int i = 0; i != sizeof(int) * 4; i++)
        {
            if (flags & (1 << i))
                ret.push_back(static_cast<Error>(i));
        }
        return ret;
    }
    
    std::vector<BaseAudioFile::Error> BaseAudioFile::getErrors() const
    {
        return extractErrorsFromFlags(getErrorFlags());
    }

    bool BaseAudioFile::getIsError() const
    {
        return mErrorFlags != ERR_NONE;
    }
    
    void BaseAudioFile::clearErrorFlags() {
        setErrorFlags(ERR_NONE);
    }

    void BaseAudioFile::setFileType(FileType i)
    {
        mFileType = i;
    }
    
    void BaseAudioFile::setPCMFormat(PCMFormat i)
    {
        mPCMFormat = i;
    }
    
    void BaseAudioFile::setHeaderEndianness(Endianness i)
    {
        mHeaderEndianness = i;
    }
    
    void BaseAudioFile::setAudioEndianness(Endianness i)
    {
        mAudioEndianness = i;
    }
    
    void BaseAudioFile::setSamplingRate(double i)
    {
        mSamplingRate = i;
    }
    
    void BaseAudioFile::setChannels(uint16_t i)
    {
        mNumChannels = i;
    }
    
    void BaseAudioFile::setFrames(FrameCount i)
    {
        mNumFrames = i;
    }
    
    void BaseAudioFile::setPCMOffset(ByteCount i)
    {
        mPCMOffset = i;
    }

    void BaseAudioFile::setErrorFlags(int flags)
    {
        mErrorFlags = flags;
    }

    void BaseAudioFile::setErrorBit(Error error)
    {
        mErrorFlags |= error;
    }

    uint16_t BaseAudioFile::findBitDepth(PCMFormat i)
    {
        switch (i)
        {
            case kAudioFileInt8:
                return 8;
            case kAudioFileInt16:
                return 16;
            case kAudioFileInt24:
                return 24;
            case kAudioFileInt32:
                return 32;
            case kAudioFileFloat32:
                return 32;
            case kAudioFileFloat64:
                return 64;
        }
    }

    BaseAudioFile::NumberFormat BaseAudioFile::findNumberFormat(PCMFormat i)
    {
        switch (i)
        {
            case kAudioFileInt8:
            case kAudioFileInt16:
            case kAudioFileInt24:
            case kAudioFileInt32:
                return kAudioFileInt;

            case kAudioFileFloat32:
            case kAudioFileFloat64:
                return kAudioFileFloat;
        }
    }
}
