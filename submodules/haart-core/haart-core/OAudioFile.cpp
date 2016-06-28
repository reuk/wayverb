#include "OAudioFile.h"

#include <cmath>
#include <vector>

namespace HISSTools
{
    template <typename T> T paddedLength(T length)
    {
        return length + (length & 0x1);
    }

    uint32_t lengthAsPString(const char* string)
    {
        uint32_t length = static_cast<uint32_t>(strlen(string));
        return ((length + 1) & 0x1) ? length + 2 : length + 1;
    }

    OAudioFile::OAudioFile()
    {
        close();
    }

    OAudioFile::OAudioFile(const std::string& i, FileType type, PCMFormat format,
                           uint16_t channels, double sr)
    {
        open(i, type, format, channels, sr);
    }

    OAudioFile::OAudioFile(const std::string& i, FileType type, PCMFormat format,
                           uint16_t channels, double sr, Endianness e)
    {
        open(i, type, format, channels, sr, e);
    }

    OAudioFile::~OAudioFile()
    {
        close();
    }

    void OAudioFile::open(const std::string& i, FileType type, PCMFormat format,
                          uint16_t channels, double sr)
    {
        open(i, type, format, channels, sr,
             type == kAudioFileWAVE ? kAudioFileLittleEndian : kAudioFileBigEndian);
    }

    void OAudioFile::open(const std::string& i, FileType type, PCMFormat format,
                          uint16_t channels, double sr, Endianness endianness)
    {
        close();
        mFile.open(i.c_str(), std::ios_base::binary);

        seekInternal(0);

        if (isOpen() && positionInternal() == 0)
        {
            setFileType(type == kAudioFileAIFF ? kAudioFileAIFC : type);
            setPCMFormat(format);
            setHeaderEndianness(
                getFileType() == kAudioFileWAVE ? endianness : kAudioFileBigEndian);
            setAudioEndianness(endianness);
            setSamplingRate(sr);
            setChannels(channels);
            setFrames(0);
            setPCMOffset(0);

            if (getFileType() == kAudioFileWAVE)
                writeWaveHeader();
            else
                writeAIFCHeader();
        }
    }

    void OAudioFile::close()
    {
        mFile.close();
        BaseAudioFile::close();
    }

    bool OAudioFile::isOpen()
    {
        return mFile.is_open();
    }

    void OAudioFile::seek(FrameCount position)
    {
        if (getPCMOffset() != 0)
            seekInternal(getPCMOffset() + getFrameByteCount() * position);
    }

    OAudioFile::FrameCount OAudioFile::getPosition()
    {
        if (getPCMOffset())
            return static_cast<FrameCount>((positionInternal() - getPCMOffset())
                                           / getFrameByteCount());

        return 0;
    }

    OAudioFile::ByteCount OAudioFile::getHeaderSize() const
    {
        return getPCMOffset() - 8;
    }

    void OAudioFile::writeInterleaved(const double* input, FrameCount numFrames)
    {
        writeAudio(input, numFrames);
    }

    void OAudioFile::writeInterleaved(const float* input, FrameCount numFrames)
    {
        writeAudio(input, numFrames);
    }

    void OAudioFile::writeChannel(const double* input, FrameCount numFrames,
                                  uint16_t channel)
    {
        writeAudio(input, numFrames, channel);
    }

    void OAudioFile::writeChannel(const float* input, FrameCount numFrames,
                                  uint16_t channel)
    {
        writeAudio(input, numFrames, channel);
    }

    OAudioFile::ByteCount OAudioFile::positionInternal()
    {
        return mFile.tellp();
    }

    bool OAudioFile::seekInternal(ByteCount position)
    {
        mFile.clear();
        mFile.seekp(position, std::ios_base::beg);
        return positionInternal() == position;
    }

    bool OAudioFile::seekRelativeInternal(ByteCount offset)
    {
        if (!offset)
            return true;
        mFile.clear();
        ByteCount newPosition = positionInternal() + offset;
        mFile.seekp(newPosition, std::ios_base::cur);
        return positionInternal() == newPosition;
    }

    bool OAudioFile::writeInternal(const char* buffer, ByteCount bytes)
    {
        mFile.clear();
        mFile.write(buffer, bytes);
        return mFile.good();
    }

    bool OAudioFile::putU64(uint64_t value, Endianness fileEndianness)
    {
        unsigned char bytes[8];

        if (fileEndianness == kAudioFileBigEndian)
        {
            bytes[0] = (value >> 56) & 0xFF;
            bytes[1] = (value >> 48) & 0xFF;
            bytes[2] = (value >> 40) & 0xFF;
            bytes[3] = (value >> 32) & 0xFF;
            bytes[4] = (value >> 24) & 0xFF;
            bytes[5] = (value >> 16) & 0xFF;
            bytes[6] = (value >> 8) & 0xFF;
            bytes[7] = (value >> 0) & 0xFF;
        }
        else
        {
            bytes[0] = (value >> 0) & 0xFF;
            bytes[1] = (value >> 8) & 0xFF;
            bytes[2] = (value >> 16) & 0xFF;
            bytes[3] = (value >> 24) & 0xFF;
            bytes[4] = (value >> 32) & 0xFF;
            bytes[5] = (value >> 40) & 0xFF;
            bytes[6] = (value >> 48) & 0xFF;
            bytes[7] = (value >> 56) & 0xFF;
        }

        return writeInternal(reinterpret_cast<const char*>(bytes), 8);
    }

    bool OAudioFile::putU32(uint32_t value, Endianness fileEndianness)
    {
        unsigned char bytes[4];

        if (fileEndianness == kAudioFileBigEndian)
        {
            bytes[0] = (value >> 24) & 0xFF;
            bytes[1] = (value >> 16) & 0xFF;
            bytes[2] = (value >> 8) & 0xFF;
            bytes[3] = (value >> 0) & 0xFF;
        }
        else
        {
            bytes[0] = (value >> 0) & 0xFF;
            bytes[1] = (value >> 8) & 0xFF;
            bytes[2] = (value >> 16) & 0xFF;
            bytes[3] = (value >> 24) & 0xFF;
        }

        return writeInternal(reinterpret_cast<const char*>(bytes), 4);
    }

    bool OAudioFile::putU24(uint32_t value, Endianness fileEndianness)
    {
        unsigned char bytes[3];

        if (fileEndianness == kAudioFileBigEndian)
        {
            bytes[0] = (value >> 16) & 0xFF;
            bytes[1] = (value >> 8) & 0xFF;
            bytes[2] = (value >> 0) & 0xFF;
        }
        else
        {
            bytes[0] = (value >> 0) & 0xFF;
            bytes[1] = (value >> 8) & 0xFF;
            bytes[2] = (value >> 16) & 0xFF;
        }

        return writeInternal(reinterpret_cast<const char*>(bytes), 3);
    }

    bool OAudioFile::putU16(uint32_t value, Endianness fileEndianness)
    {
        unsigned char bytes[2];

        if (fileEndianness == kAudioFileBigEndian)
        {
            bytes[0] = (value >> 8) & 0xFF;
            bytes[1] = (value >> 0) & 0xFF;
        }
        else
        {
            bytes[0] = (value >> 0) & 0xFF;
            bytes[1] = (value >> 8) & 0xFF;
        }

        return writeInternal(reinterpret_cast<const char*>(bytes), 2);
    }

    bool OAudioFile::putU08(uint32_t value)
    {
        unsigned char byte;

        byte = value & 0xFF;

        return writeInternal(reinterpret_cast<const char*>(&byte), 1);
    }

    bool OAudioFile::putPadByte()
    {
        char padByte = 0;
        return writeInternal(&padByte, 1);
    }

    bool OAudioFile::putChunk(const char* tag, uint32_t size)
    {
        return writeInternal(tag, 4) & putU32(size, getHeaderEndianness());
    }

    bool OAudioFile::putTag(const char* tag)
    {
        return writeInternal(tag, 4);
    }

// FIX - REPLACE

#define FloatToUnsigned(f) ((uint32_t)(((int32_t)(f - 2147483648.0)) + 2147483647L) + 1)

    void _af_convert_to_ieee_extended(double num, unsigned char* bytes)
    {
        int sign;
        int expon;
        double fMant, fsMant;
        uint32_t hiMant, loMant;

        if (num < 0)
        {
            sign = 0x8000;
            num *= -1;
        }
        else
        {
            sign = 0;
        }

        if (num == 0)
        {
            expon = 0;
            hiMant = 0;
            loMant = 0;
        }
        else
        {
            fMant = frexp(num, &expon);
            if ((expon > 16384) || !(fMant < 1))
            { /* Infinity or NaN */
                expon = sign | 0x7FFF;
                hiMant = 0;
                loMant = 0; /* infinity */
            }
            else
            { /* Finite */
                expon += 16382;
                if (expon < 0)
                { /* denormalized */
                    fMant = ldexp(fMant, expon);
                    expon = 0;
                }
                expon |= sign;
                fMant = ldexp(fMant, 32);
                fsMant = floor(fMant);
                hiMant = FloatToUnsigned(fsMant);
                fMant = ldexp(fMant - fsMant, 32);
                fsMant = floor(fMant);
                loMant = FloatToUnsigned(fsMant);
            }
        }

        bytes[0] = expon >> 8;
        bytes[1] = expon;
        bytes[2] = hiMant >> 24;
        bytes[3] = hiMant >> 16;
        bytes[4] = hiMant >> 8;
        bytes[5] = hiMant;
        bytes[6] = loMant >> 24;
        bytes[7] = loMant >> 16;
        bytes[8] = loMant >> 8;
        bytes[9] = loMant;
    }


    bool OAudioFile::putExtended(double value)
    {
        unsigned char bytes[10];

        _af_convert_to_ieee_extended(value, bytes);

        return writeInternal(reinterpret_cast<const char*>(bytes), 10);
    }

    bool OAudioFile::putPString(const char* string)
    {
        char length = strlen(string);
        bool success = true;

        success &= writeInternal(&length, 1);
        success &= writeInternal(string, length);

        // Pad byte if necessary (number of bytes written so far is odd)

        if ((length + 1) & 0x1) success &= putPadByte();

        return success;
    }

    void OAudioFile::writeWaveHeader()
    {
        bool success = true;

        // File Header

        if (getHeaderEndianness() == kAudioFileLittleEndian)
            success &= putChunk("RIFF", 36);
        else
            success &= putChunk("RIFX", 36);
        success &= putTag("WAVE");

        // Format Chunk

        success &= putChunk("fmt ", 16);
        success &= putU16(getNumberFormat() == kAudioFileInt ? 0x1 : 0x3, getHeaderEndianness());
        success &= putU16(getChannels(), getHeaderEndianness());
        success &= putU32(getSamplingRate(), getHeaderEndianness());
        // Bytes per second
        success &= putU32(getSamplingRate() * getFrameByteCount(), getHeaderEndianness());
        // Block alignment
        success &= putU16(static_cast<uint16_t>(getFrameByteCount()), getHeaderEndianness());
        success &= putU16(getBitDepth(), getHeaderEndianness());

        // Data Chunk (empty)

        success &= putChunk("data", 0);
        setPCMOffset(positionInternal());

        if (!success) setErrorBit(ERR_FILE_COULDNT_WRITE);
    }

    const char* OAudioFile::getCompressionTag()
    {
        // FIX - doesn't deal with little endian... (return type)?
        // "sowt"
        
        switch (getPCMFormat())
        {
            case kAudioFileInt8:
            case kAudioFileInt16:
            case kAudioFileInt24:
            case kAudioFileInt32:
                return "NONE";
            case kAudioFileFloat32:
                return "fl32";
            case kAudioFileFloat64:
                return "fl64";
        }
    }

    const char* OAudioFile::getCompressionString()
    {
        
        // FIX - doesn't deal with little endian... (return type)?
        // "little endian"
        
        switch (getPCMFormat())
        {
            case kAudioFileInt8:
            case kAudioFileInt16:
            case kAudioFileInt24:
            case kAudioFileInt32:
                return "not compressed";
            case kAudioFileFloat32:
                return "32-bit floating point";
            case kAudioFileFloat64:
                return "64-bit floating point";
        }
    }

    void OAudioFile::writeAIFCHeader()
    {
        bool success = true;

        // Set file type, data size offset frames and header size

        uint32_t headerSize = 62 + lengthAsPString(getCompressionString());

        // File Header

        success &= putChunk("FORM", headerSize);
        success &= putTag("AIFC");

        // FVER Chunk

        success &= putChunk("FVER", 4);
        success &= putU32(AIFC_CURRENT_SPECIFICATION, getHeaderEndianness());

        // COMM Chunk

        success &= putChunk("COMM", 22 + lengthAsPString(getCompressionString()));
        success &= putU16(getChannels(), getHeaderEndianness());
        success &= putU32(getFrames(), getHeaderEndianness());
        success &= putU16(getBitDepth(), getHeaderEndianness());
        success &= putExtended(getSamplingRate());
        success &= putTag(getCompressionTag());
        success &= putPString(getCompressionString());

        // SSND Chunk (zero size)

        success &= putChunk("SSND", 8);

        // Set offset and block size to zero

        success &= putU32(0, getHeaderEndianness());
        success &= putU32(0, getHeaderEndianness());

        // Set offset to PCM Data

        setPCMOffset(positionInternal());

        if (!success)
            setErrorBit(ERR_FILE_COULDNT_WRITE);
    }


    bool OAudioFile::updateHeader()
    {
        FrameCount endFrame = getPosition();

        bool success = true;

        if (endFrame > getFrames())
        {
            // Calculate new data length and end of data

            setFrames(endFrame);

            ByteCount dataBytes = (getFrameByteCount() * getFrames());
            ByteCount dataEnd = positionInternal();

            // Write padding byte if relevant

            if (dataBytes & 0x1) success &= putPadByte();

            // Update chunk size for file and audio data and frames for aifc
            // and reset position to the end of the data

            if (getFileType() == kAudioFileWAVE)
            {
                success &= seekInternal(4);
                success &= putU32(
                    static_cast<uint32_t>(getHeaderSize() + paddedLength(dataBytes)),
                    getHeaderEndianness());
                success &= seekInternal(getPCMOffset() - 4);
                success &= putU32(static_cast<uint32_t>(dataBytes),
                                  getHeaderEndianness());
            }
            else
            {
                success &= seekInternal(4);
                success &= putU32(
                    static_cast<uint32_t>(getHeaderSize() + paddedLength(dataBytes)),
                    getHeaderEndianness());
                success &= seekInternal(34);
                success &= putU32(static_cast<uint32_t>(getFrames()),
                                  getHeaderEndianness());
                success &= seekInternal(getPCMOffset() - 12);
                success &= putU32(static_cast<uint32_t>(dataBytes) + 8,
                                  getHeaderEndianness());
            }

            // Return to end of data

            success &= seekInternal(dataEnd);
        }

        return success;
    }

    bool OAudioFile::resize(FrameCount numFrames)
    {
        bool success = true;

        if (numFrames > getFrames())
        {
            // Calculate new data length and end of data

            ByteCount dataBytes = (getFrameByteCount() * getFrames());
            ByteCount endBytes = (getFrameByteCount() * numFrames);
            ByteCount dataEnd = positionInternal();

            seek(getFrames());

            for (ByteCount i = 0; i < endBytes - dataBytes; i++)
                success &= putPadByte();

            // Return to end of data

            success &= seekInternal(dataEnd);
        }

        return success;
    }


    bool OAudioFile::writePCMData(const char* input, FrameCount numFrames)
    {
        // Write audio

        bool success = writeInternal(input, getFrameByteCount() * numFrames);

        // Update number of frames

        success &= updateHeader();

        return success;
    }

    uint32_t OAudioFile::inputToU32(double input, int bitDepth)
    {
        // FIX - issues of value (note the value is 2^31 - 1 below)

        input = std::min(std::max(input, -1.0), 1.0);

        return static_cast<uint32_t>(
            round(input * static_cast<double>(1 << (bitDepth - 1))));
    }

    template <class T>
    void OAudioFile::writeAudio(const T* input, FrameCount numFrames, int32_t channel)
    {
        bool success = true;

        uint32_t byteDepth = getByteDepth();
        uintptr_t inputSamples = (channel < 0) ? getChannels() * numFrames : numFrames;
        ByteCount offset = (channel < 0) ? 0 : channel * byteDepth;
        ByteCount byteStep = (channel < 0) ? 0 : getFrameByteCount() - (byteDepth + offset);
        FrameCount endFrame = getPosition() + numFrames;
        //ByteCount counter = positionInternal();
        
        // FIX - the slowest thing is seeking in the file, so that seems like a bad plan - it might be better to read in chunks overwrite locally and then write back the chunk
        
        // Write zeros to channels if necessary (multichannel files written one
        // channel at a time)

        if (channel >= 0 && getChannels() > 1)
            success &= resize(endFrame);

        // Write audio data

        switch (getPCMFormat())
        {
        case kAudioFileInt8:
            for (uintptr_t i = 0; i < inputSamples; i++)
            {
                seekRelativeInternal(offset);
                putU08(inputToU32(input[i], 8));
                seekRelativeInternal(byteStep);
            }
            break;

        case kAudioFileInt16:
            for (uintptr_t i = 0; i < inputSamples; i++)
            {
                seekRelativeInternal(offset);
                putU16(inputToU32(input[i], 16), getAudioEndianness());
                seekRelativeInternal(byteStep);
            }
            break;

        case kAudioFileInt24:
            for (uintptr_t i = 0; i < inputSamples; i++)
            {
                seekRelativeInternal(offset);
                putU24(inputToU32(input[i], 24), getAudioEndianness());
                seekRelativeInternal(byteStep);
            }
            break;

        case kAudioFileInt32:
            for (uintptr_t i = 0; i < inputSamples; i++)
            {
                /*counter += offset;
                seekInternal(counter);
                putU32(inputToU32(input[i], 32), getAudioEndianness());
                counter += 4;
                counter += byteStep;
                seekInternal(counter);*/

                seekRelativeInternal(offset);
                putU32(inputToU32(input[i], 32), getAudioEndianness());
                seekRelativeInternal(byteStep);
            }
            break;

        case kAudioFileFloat32:
            for (uintptr_t i = 0; i < inputSamples; i++)
            {
                /*counter += offset;
                seekInternal(counter);
                float value = input[i];
                putU32(*reinterpret_cast<uint32_t*>(&value), getAudioEndianness());
                counter += 4;
                counter += byteStep;
                seekInternal(counter);
                */
                
                seekRelativeInternal(offset);
                float value = input[i];
                putU32(*reinterpret_cast<uint32_t*>(&value), getAudioEndianness());
                seekRelativeInternal(byteStep);
            }
            break;

        case kAudioFileFloat64:
            for (uintptr_t i = 0; i < inputSamples; i++)
            {
                seekRelativeInternal(offset);
                double value = input[i];
                putU64(*reinterpret_cast<uint64_t*>(&value), getAudioEndianness());
                seekRelativeInternal(byteStep);
            }
            break;
        }

        // Update number of frames

        success &= updateHeader();

        // return success;
    }
}
