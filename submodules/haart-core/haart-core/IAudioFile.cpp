#include "IAudioFile.h"

#include <cmath>
#include <cassert>
#include <vector>

#define WORK_LOOP_SIZE 1024

namespace HISSTools
{
    // Length Helper

    template <typename T> T paddedLength(T length)
    {
        return length + (length & 0x1);
    }

    // Constructor and Decstructor

    IAudioFile::IAudioFile(const std::string& i) : mBuffer(NULL)
    {
        open(i);
    }

    IAudioFile::~IAudioFile()
    {
        close();
    }

    // File Opening / Close

    void IAudioFile::open(const std::string& i)
    {
        close();

        if (!i.empty())
        {
            mFile.open(i.c_str(), std::ios_base::binary);
            parseHeader();
            mBuffer = new char[WORK_LOOP_SIZE * getFrameByteCount()];
            seek();
        }
    }

    void IAudioFile::close()
    {
        mFile.close();
        BaseAudioFile::close();
        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = NULL;
        }
    }

    bool IAudioFile::isOpen()
    {
        return mFile.is_open();
    }

    // File Position

    void IAudioFile::seek(FrameCount position)
    {
        seekInternal(getPCMOffset() + getFrameByteCount() * position);
    }

    IAudioFile::FrameCount IAudioFile::getPosition()
    {
        if (getPCMOffset())
            return static_cast<FrameCount>((positionInternal() - getPCMOffset()) / getFrameByteCount());

        return 0;
    }

    // File Reading

    void IAudioFile::readRaw(void* output, FrameCount numFrames)
    {
        readInternal(static_cast<char *>(output), getFrameByteCount() * numFrames);
    }

    void IAudioFile::readInterleaved(double* output, FrameCount numFrames)
    {
        readAudio(output, numFrames);
    }

    void IAudioFile::readInterleaved(float* output, FrameCount numFrames)
    {
        readAudio(output, numFrames);
    }

    void IAudioFile::readChannel(double* output, FrameCount numFrames,
                                 uint16_t channel)
    {
        readAudio(output, numFrames, channel);
    }

    void IAudioFile::readChannel(float* output, FrameCount numFrames,
                                 uint16_t channel)
    {
        readAudio(output, numFrames, channel);
    }

    //  Internal File Handling

    bool IAudioFile::readInternal(char* buffer,
                                                ByteCount numBytes)
    {
        mFile.clear();
        mFile.read(buffer, numBytes);

       return static_cast<ByteCount>(mFile.gcount()) == numBytes;
    }

    bool IAudioFile::seekInternal(ByteCount position)
    {
        mFile.clear();
        mFile.seekg(position, std::ios_base::beg);
        return positionInternal() == position;
    }

    bool IAudioFile::advanceInternal(ByteCount offset)
    {
        return seekInternal(positionInternal() + offset);
    }

    IAudioFile::ByteCount IAudioFile::positionInternal()
    {
        return mFile.tellg();
    }

    //  Extracting Single Values

    uint64_t IAudioFile::getU64(const char* b,
                                Endianness fileEndianness) const
    {
        const unsigned char* bytes
        = reinterpret_cast<const unsigned char*>(b);
        if (fileEndianness == kAudioFileBigEndian)
            return ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48)
            | ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32)
            | ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16)
            | ((uint64_t)bytes[6] << 8) | (uint64_t)bytes[7];
        else
            return ((uint64_t)bytes[7] << 56) | ((uint64_t)bytes[6] << 48)
            | ((uint64_t)bytes[5] << 40) | ((uint64_t)bytes[4] << 32)
            | ((uint64_t)bytes[3] << 24) | ((uint64_t)bytes[2] << 16)
            | ((uint64_t)bytes[1] << 8) | (uint64_t)bytes[0];
    }

    uint32_t IAudioFile::getU32(const char* b,
                                Endianness fileEndianness) const
    {
        const unsigned char* bytes
        = reinterpret_cast<const unsigned char*>(b);
        if (fileEndianness == kAudioFileBigEndian)
            return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8)
            | bytes[3];
        else
            return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8)
            | bytes[0];
    }

    uint32_t IAudioFile::getU24(const char* b,
                                Endianness fileEndianness) const
    {
        const unsigned char* bytes
        = reinterpret_cast<const unsigned char*>(b);
        if (fileEndianness == kAudioFileBigEndian)
            return (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
        else
            return (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
    }

    uint32_t IAudioFile::getU16(const char* b,
                                Endianness fileEndianness) const
    {
        const unsigned char* bytes
        = reinterpret_cast<const unsigned char*>(b);
        if (fileEndianness == kAudioFileBigEndian)
            return (bytes[0] << 8) | bytes[1];
        else
            return (bytes[1] << 8) | bytes[0];
    }

    // Conversion

    double IAudioFile::extendedToDouble(const char* bytes) const
    {
        // Get double from big-endian ieee 80 bit extended floating point
        // format

        bool sign = getU16(bytes, kAudioFileBigEndian) & 0x8000;
        int32_t exponent = getU16(bytes, kAudioFileBigEndian) & 0x777F;
        uint32_t hiSignificand = getU32(bytes + 2, kAudioFileBigEndian);
        uint32_t loSignificand = getU32(bytes + 6, kAudioFileBigEndian);

        double value;

        // Special handlers for zeros and infs / NaNs (in either case the
        // file is probably useless as the sampling rate *should* have a
        // sensible value)

        if (!exponent && !hiSignificand && !loSignificand) return 0.0;

        if (exponent == 0x777F) return HUGE_VAL;

        exponent -= 16383;
        value = ldexp(static_cast<double>(hiSignificand), exponent - 31);
        value += ldexp(static_cast<double>(loSignificand),
                       exponent - (31 + 32));

        if (sign) value = -value;

        return value;
    }

    template <class T>
    void IAudioFile::u32ToOutput(T* output, uint32_t value)
    {
        *output = *reinterpret_cast<int32_t*>(&value)
        * (T)0x1.0fp-31; // 0.00000000046566128730773925;
    }

    template <class T>
    void IAudioFile::float32ToOutput(T* output, uint32_t value)
    {
        *output = *reinterpret_cast<float*>(&value);
    }

    template <class T>
    void IAudioFile::float64ToOutput(T* output, uint64_t value)
    {
        *output = *reinterpret_cast<double*>(&value);
    }

     //  Chunk Reading

    bool IAudioFile::matchTag(const char* a, const char* b)
    {
        return (strncmp(a, b, 4) == 0);
    }

    bool IAudioFile::readChunkHeader(char* tag, uint32_t& chunkSize)
    {
        char header[8] = {};

        if (!readInternal(header, 8)) return false;

        strncpy(tag, header, 4);
        chunkSize = getU32(header + 4, getHeaderEndianness());

        return true;
    }

    bool IAudioFile::findChunk(const char* searchTag, uint32_t& chunkSize)
    {
        char tag[4] = {};

        while (readChunkHeader(tag, chunkSize))
        {
            if (matchTag(tag, searchTag)) return true;

            if (!advanceInternal(paddedLength(chunkSize))) return false;
        }

        return false;
    }

    bool IAudioFile::readChunk(char* data, uint32_t readSize,
                               uint32_t chunkSize)
    {
        if (readSize)
            if (readSize > chunkSize || !readInternal(data, readSize))
                return false;

        if (!advanceInternal(paddedLength(chunkSize) - readSize))
            return false;

        return true;
    }

    // PCM Format Helpers

    IAudioFile::Error IAudioFile::findPCMFormat(uint16_t bitDepth,
                                                NumberFormat format,
                                                PCMFormat& ret)
    {
        int fileFormat = -1;

        if (format == kAudioFileInt)
        {
            if (bitDepth == 8) fileFormat = kAudioFileInt8;
            if (bitDepth == 16) fileFormat = kAudioFileInt16;
            if (bitDepth == 24) fileFormat = kAudioFileInt24;
            if (bitDepth == 32) fileFormat = kAudioFileInt32;
        }

        if (format == kAudioFileFloat)
        {
            if (bitDepth == 32) fileFormat = kAudioFileFloat32;
            if (bitDepth == 64) fileFormat = kAudioFileFloat64;
        }

        if (fileFormat == -1) return ERR_FILE_UNSUPPORTED_PCM_FORMAT;

        ret = static_cast<PCMFormat>(fileFormat);

        return ERR_NONE;
    }

    void IAudioFile::setPCMFormat(uint16_t bitDepth, NumberFormat format)
    {
        PCMFormat ret = static_cast<PCMFormat>(-1);
        Error error = findPCMFormat(bitDepth, format, ret);
        if (error == ERR_NONE)
        {
            BaseAudioFile::setPCMFormat(ret);
        }
        setErrorBit(error);
    }

    // AIFF Helpers

    bool IAudioFile::getAIFFChunkHeader(AiffTag& enumeratedTag,
                                        uint32_t& chunkSize)
    {
        char tag[4];

        enumeratedTag = AIFC_TAG_UNKNOWN;

        if (!readChunkHeader(tag, chunkSize)) return false;

        if (matchTag(tag, "FVER")) enumeratedTag = AIFC_TAG_VERSION;

        if (matchTag(tag, "COMM")) enumeratedTag = AIFC_TAG_COMMON;

        if (matchTag(tag, "SSND")) enumeratedTag = AIFC_TAG_AUDIO;

        return true;
    }

    IAudioFile::AifcCompression
    IAudioFile::getAIFCCompression(const char* tag) const
    {
        // FIX - finish this work (twos/sowt must be 16 bit?)

        if (matchTag(tag, "NONE")) return AIFC_COMPRESSION_NONE;

        if (matchTag(tag, "twos")) return AIFC_COMPRESSION_NONE;

        if (matchTag(tag, "sowt")) return AIFC_COMPRESSION_LITTLE_ENDIAN;

        if (matchTag(tag, "fl32")) return AIFC_COMPRESSION_FLOAT;

        if (matchTag(tag, "FL32")) return AIFC_COMPRESSION_FLOAT;

        if (matchTag(tag, "fl64")) return AIFC_COMPRESSION_FLOAT;

        if (matchTag(tag, "FL64")) return AIFC_COMPRESSION_FLOAT;

        return AIFC_COMPRESSION_UNKNOWN;
    }

    //  Parse Headers

    void IAudioFile::parseHeader()
    {
        char chunk[12] = {}, fileType[4] = {}, fileSubtype[4] = {};

        // `Read file header

        if (!readInternal(chunk, 12))
        {
            setErrorBit(ERR_FILE_BAD_FORMAT);
            return;
        }

        // Get file type and subtype

        strncpy(fileType, chunk, 4);
        strncpy(fileSubtype, chunk + 8, 4);

        // AIFF or AIFC

        if (matchTag(fileType, "FORM")
            && (matchTag(fileSubtype, "AIFF")
                || matchTag(fileSubtype, "AIFC")))
            return parseAIFFHeader(fileSubtype);

        // WAVE file format

        if ((matchTag(fileType, "RIFF") || matchTag(fileType, "RIFX"))
            && matchTag(fileSubtype, "WAVE"))
            return parseWaveHeader(fileType);

        // No known format found

        setErrorBit(ERR_FILE_UNKNOWN_FORMAT);
    }

    void IAudioFile::parseAIFFHeader(const char* fileSubtype)
    {
        AiffTag tag;

        uint32_t formatValid = AIFC_TAG_COMMON | AIFC_TAG_AUDIO;
        uint32_t formatCheck = 0;
        char chunk[22];
        uint32_t chunkSize;
        uint16_t bitDepth;
        NumberFormat format;

        setHeaderEndianness(kAudioFileBigEndian);

        if (matchTag(fileSubtype, "AIFC"))
        {
            setFileType(kAudioFileAIFC);

            // Require a version chunk

            formatValid |= AIFC_TAG_VERSION;
        }

        // Iterate over chunks

        while (getAIFFChunkHeader(tag, chunkSize))
        {
            formatCheck |= tag;

            switch (tag)
            {
                case AIFC_TAG_VERSION:

                    // Read format number and check for the correct version of
                    // the AIFC specification

                    if (!readChunk(chunk, 4, chunkSize))
                    {
                        setErrorBit(ERR_FILE_BAD_FORMAT);
                        return;
                    }
                    if (getU32(chunk, getHeaderEndianness())
                        != AIFC_CURRENT_SPECIFICATION)
                    {
                        setErrorBit(ERR_AIFC_WRONG_VERSION);
                        return;
                    }

                    break;

                case AIFC_TAG_COMMON:

                    // Read common chunk (at least 18 bytes and up to 22)

                    if (!readChunk(chunk,
                                   (chunkSize > 22)
                                   ? 22
                                   : ((chunkSize < 18) ? 18 : chunkSize),
                                   chunkSize))
                    {
                        setErrorBit(ERR_FILE_BAD_FORMAT);
                        return;
                    }

                    // Retrieve relevant data (AIFF or AIFC) and set AIFF
                    // defaults

                    setChannels(getU16(chunk + 0, getHeaderEndianness()));
                    setFrames(getU32(chunk + 2, getHeaderEndianness()));
                    bitDepth = getU16(chunk + 6, getHeaderEndianness());
                    setSamplingRate(extendedToDouble(chunk + 8));

                    format = kAudioFileInt;
                    setAudioEndianness(kAudioFileBigEndian);

                    // If there are no frames then it is not required for there
                    // to be an audio (SSND) chunk

                    if (!getFrames()) formatCheck |= AIFC_TAG_AUDIO;

                    if (matchTag(fileSubtype, "AIFC"))
                    {
                        // Set parameters based on format

                        switch (getAIFCCompression(chunk + 18))
                        {
                            case AIFC_COMPRESSION_NONE:
                                break;
                            case AIFC_COMPRESSION_LITTLE_ENDIAN:
                                setAudioEndianness(kAudioFileLittleEndian);
                                break;
                            case AIFC_COMPRESSION_FLOAT:
                                format = kAudioFileFloat;
                                break;
                            default:
                                setErrorBit(ERR_AIFC_UNSUPPORTED_FORMAT);
                                return;
                        }
                    }
                    else
                        setFileType(kAudioFileAIFF);

                    setPCMFormat(bitDepth, format);
                    if (getIsError())
                    {
                        return;
                    }

                    break;

                case AIFC_TAG_AUDIO:

                    // Audio data starts 8 bytes after this point in the file (2
                    // x 32-bit values) + offset dealt with below

                    setPCMOffset(positionInternal() + 8);

                    if (!readChunk(chunk, 4, chunkSize))
                    {
                        setErrorBit(ERR_FILE_BAD_FORMAT);
                        return;
                    }

                    // Account for offset value (ignore block size value that
                    // comes after that)

                    setPCMOffset(
                                 getPCMOffset()
                                 + getU32(chunk, getHeaderEndianness()));

                    break;

                case AIFC_TAG_UNKNOWN:

                    // Read no data, but update the file position

                    if (!readChunk(NULL, 0, chunkSize))
                    {
                        setErrorBit(ERR_FILE_BAD_FORMAT);
                        return;
                    }

                    break;
            }
        }

        // Check that all relevant chunks were found

        if ((~formatCheck) & formatValid)
        {
            setErrorBit(ERR_FILE_BAD_FORMAT);
            return;
        }
    }

    void IAudioFile::parseWaveHeader(const char* fileType)
    {
        char chunk[16];
        uint32_t chunkSize;

        // Check endianness

        setHeaderEndianness(matchTag(fileType, "RIFX")
                            ? kAudioFileBigEndian
                            : kAudioFileLittleEndian);
        setAudioEndianness(getHeaderEndianness());

        // Search for the format chunk and read first 16 bytes (ignored any
        // extended header info)

        if (!(findChunk("fmt ", chunkSize)
              && readChunk(chunk, 16, chunkSize)))
        {
            setErrorBit(ERR_FILE_BAD_FORMAT);
            return;
        }

        // Check for supported formats

        if (getU16(chunk, getHeaderEndianness()) != 0x0001
            && getU16(chunk, getHeaderEndianness()) != 0x0003)
        {
            setErrorBit(ERR_WAVE_UNSUPPORTED_FORMAT);
            return;
        }

        // Retrieve relevant data

        NumberFormat format
        = getU16(chunk + 0, getHeaderEndianness()) == 0x0003
        ? kAudioFileFloat
        : kAudioFileInt;
        setChannels(getU16(chunk + 2, getHeaderEndianness()));
        setSamplingRate(getU32(chunk + 4, getHeaderEndianness()));
        uint16_t bitDepth = getU16(chunk + 14, getHeaderEndianness());

        // Set PCM Format

        setPCMFormat(bitDepth, format);
        if (getIsError())
        {
            return;
        }

        // Search for the data chunk and retrieve frame size and file offset
        // to audio data

        if (!findChunk("data", chunkSize))
        {
            setErrorBit(ERR_FILE_BAD_FORMAT);
            return;
        }

        setFrames(chunkSize / getFrameByteCount());
        setPCMOffset(positionInternal());
        setFileType(kAudioFileWAVE);
    }

    //  Internal Typed Audio Read

    template <class T>
    void IAudioFile::readAudio(T* output, FrameCount numFrames, int32_t channel)
    {
        // Calculate sizes

        uint16_t numChannels = (channel < 0) ? getChannels() : 1;
        uint16_t channelStep = (channel < 0) ? 1 : getChannels();
        uint32_t byteDepth = getByteDepth();
        uintptr_t byteStep = byteDepth * channelStep;

        channel = std::max(channel, 0);

        while (numFrames)
        {
            FrameCount loopFrames = numFrames > WORK_LOOP_SIZE ? WORK_LOOP_SIZE : numFrames;
            uintptr_t loopSamples = loopFrames * numChannels;
            uintptr_t j = channel * byteDepth;

            // Read raw

            readRaw(mBuffer, loopFrames);

            // Move to Output

            switch (getPCMFormat())
            {
                case kAudioFileInt8:
                    for (uintptr_t i = 0; i < loopSamples; i++, j += byteStep)
                        u32ToOutput(output + i, mBuffer[j] << 24);
                    break;

                case kAudioFileInt16:
                    for (uintptr_t i = 0; i < loopSamples; i++, j += byteStep)
                        u32ToOutput(
                                    output + i,
                                    getU16(mBuffer + j, getAudioEndianness())
                                    << 16);
                    break;

                case kAudioFileInt24:
                    for (uintptr_t i = 0; i < loopSamples; i++, j += byteStep)
                        u32ToOutput(
                                    output + i,
                                    getU24(mBuffer + j, getAudioEndianness())
                                    << 8);
                    break;

            case kAudioFileInt32:
                for (size_t i = 0; i < loopSamples; i++, j += byteStep)
                    u32ToOutput(output + i, getU32(mBuffer + j,
                                                   getAudioEndianness()));
                break;

            case kAudioFileFloat32:
                for (size_t i = 0; i < loopSamples; i++, j += byteStep)
                    float32ToOutput(output + i, getU32(mBuffer + j,
                                                       getAudioEndianness()));
                break;

            case kAudioFileFloat64:
                for (size_t i = 0; i < loopSamples; i++, j += byteStep)
                    float64ToOutput(output + i, getU64(mBuffer + j,
                                                       getAudioEndianness()));
                break;
            }

            numFrames -= loopFrames;
            output += loopSamples;
        }
    }
}
