#include "IAudioFile.h"
#include "OAudioFile.h"

#include "catch.hpp"

#include <iostream>

auto file_namesO = std::vector<std::string>{
    TEST_FILE_DIR "/a.aif",
    TEST_FILE_DIR "/b.wav",
    TEST_FILE_DIR "/c.aiff",
    TEST_FILE_DIR "/d.wav",
    TEST_FILE_DIR "/e.wav",
    TEST_FILE_DIR "/f.wav",
    TEST_FILE_DIR "/g.aiff",
    TEST_FILE_DIR "/h.aiff",
};


TEST_CASE("audio file can be written", "[OAudioFile]") {
    using namespace HISSTools;
    OAudioFile oAudioFile;
    IAudioFile iAudioFile;
    REQUIRE(! oAudioFile.getIsError());

    REQUIRE(oAudioFile.getSamplingRate() == 0);
    REQUIRE(oAudioFile.getChannels() == 0);
    REQUIRE(oAudioFile.getFrames() == 0);
    REQUIRE(! oAudioFile.isOpen());

    SECTION("calling OAudioFile::open returns no errors") {
        oAudioFile.open(file_namesO[0], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt8, 1, 44100.0);
        CHECK(! oAudioFile.getIsError());
        oAudioFile.open(file_namesO[1], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt16, 2, 32000.0);
        CHECK(! oAudioFile.getIsError());
        oAudioFile.open(file_namesO[2], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt24, 3, 96000.0);
        CHECK(! oAudioFile.getIsError());
        oAudioFile.open(file_namesO[3], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt32, 4, 192000.0);
        CHECK(! oAudioFile.getIsError());
    }

    SECTION("fields of OAudioFile are set correctly") {
        auto run_test = [&oAudioFile] (const std::string& i, BaseAudioFile::FileType type, BaseAudioFile::PCMFormat format, uint16_t numChans,
                                       double sr) {

            oAudioFile.open(i, type, format, numChans, sr);
            CHECK(! oAudioFile.getIsError());

            REQUIRE(oAudioFile.getSamplingRate() == sr);
                REQUIRE(oAudioFile.getFileType() == type);
                REQUIRE(oAudioFile.getPCMFormat() == format);
                REQUIRE(oAudioFile.getChannels() == numChans);
                REQUIRE(oAudioFile.getFrames() == 0);
                REQUIRE(oAudioFile.getPosition() == 0);
                REQUIRE(oAudioFile.isOpen());

                oAudioFile.close();
        };

        run_test(file_namesO[0], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt8, 1, 44100.0);
        run_test(file_namesO[1], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt16, 2, 32000.0);
        run_test(file_namesO[2], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt24, 3, 96000.0);
        run_test(file_namesO[3], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt32, 4, 192000.0);
    }

    SECTION("can write some data out") {

        auto run_test = [&oAudioFile] (const std::string& i, BaseAudioFile::FileType type, BaseAudioFile::PCMFormat format, uint16_t numChans,
                                       double sr) {

            oAudioFile.open(i, type, format, numChans, sr);
            CHECK(! oAudioFile.getIsError());

            uint32_t srr = std::round(sr);

            std::vector<double> sineWave;
            sineWave.resize(sr);

            for (auto i = 0; i < numChans; i++)
            {
                for (auto j = 0; j < srr; j++)
                    sineWave[j] = 0.8 * sin(440.0 * (i + 1) * 2.0 * M_PI * j / sr);

                oAudioFile.seek();
                oAudioFile.writeChannel(sineWave.data(), srr, i);
            }

            CHECK(!oAudioFile.getIsError());
            REQUIRE(oAudioFile.getFrames() == srr);

            oAudioFile.close();
        };

        run_test(file_namesO[0], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt8, 1, 44100.0);
        run_test(file_namesO[1], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt16, 2, 32000.0);
        run_test(file_namesO[2], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt24, 3, 96000.0);
        run_test(file_namesO[3], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt32, 4, 192000.0);
        run_test(file_namesO[4], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileFloat32, 2, 48000.0);
        run_test(file_namesO[5], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileFloat64, 4, 44100.0);
        run_test(file_namesO[6], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileFloat32, 1, 48000.0);
        run_test(file_namesO[7], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileFloat64, 3, 44100.0);

    SECTION("can read fields BACK out correctly") {

        auto run_test = [&iAudioFile] (const std::string& i, BaseAudioFile::FileType type, BaseAudioFile::PCMFormat format, uint16_t numChans,
                                       double sr) {

            iAudioFile.open(i);
            REQUIRE(! iAudioFile.getIsError());

            uint32_t srr = std::round(sr);

            REQUIRE(iAudioFile.getSamplingRate() == sr);
            REQUIRE(iAudioFile.getFileType() == type);
            REQUIRE(iAudioFile.getPCMFormat() == format);
            REQUIRE(iAudioFile.getChannels() == numChans);
            REQUIRE(iAudioFile.getFrames() == srr);
            REQUIRE(iAudioFile.getPosition() == 0);
            REQUIRE(iAudioFile.isOpen());

            iAudioFile.close();
        };

        run_test(file_namesO[0], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt8, 1, 44100.0);
        run_test(file_namesO[1], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt16, 2, 32000.0);
        run_test(file_namesO[2], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileInt24, 3, 96000.0);
        run_test(file_namesO[3], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileInt32, 4, 192000.0);
        run_test(file_namesO[4], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileFloat32, 2, 48000.0);
        run_test(file_namesO[5], BaseAudioFile::kAudioFileWAVE, BaseAudioFile::kAudioFileFloat64, 4, 44100.0);
        run_test(file_namesO[6], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileFloat32, 1, 48000.0);
        run_test(file_namesO[7], BaseAudioFile::kAudioFileAIFC, BaseAudioFile::kAudioFileFloat64, 3, 44100.0);

        }
    }
}
