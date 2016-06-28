#include "IAudioFile.h"
#include "catch.hpp"

#include <iostream>

auto file_names = std::vector<std::string>{
    TEST_FILE_DIR "/8000_sweep.aifc",
    TEST_FILE_DIR "/8000_sweep.aiff",
    TEST_FILE_DIR "/8000_sweep.wav",
    TEST_FILE_DIR "/32000_sweep.aifc",
    TEST_FILE_DIR "/32000_sweep.aiff",
    TEST_FILE_DIR "/32000_sweep.wav",
    TEST_FILE_DIR "/44100_sweep.aifc",
    TEST_FILE_DIR "/44100_sweep.aiff",
    TEST_FILE_DIR "/44100_sweep.wav",
    TEST_FILE_DIR "/48000_sweep.aifc",
    TEST_FILE_DIR "/48000_sweep.aiff",
    TEST_FILE_DIR "/48000_sweep.wav",
    TEST_FILE_DIR "/96000_sweep.aifc",
    TEST_FILE_DIR "/96000_sweep.aiff",
    TEST_FILE_DIR "/96000_sweep.wav",
};

TEST_CASE("audio file can be opened", "[IAudioFile]") {
    using namespace HISSTools;
    IAudioFile iAudioFile;
    REQUIRE(! iAudioFile.getIsError());

    REQUIRE(iAudioFile.getSamplingRate() == 0);
    REQUIRE(iAudioFile.getChannels() == 0);
    REQUIRE(iAudioFile.getFrames() == 0);
    REQUIRE(! iAudioFile.isOpen());

    SECTION("calling IAudioFile::open returns no errors") {
        for (const auto & i : file_names) {
            iAudioFile.open(i);
            REQUIRE(! iAudioFile.getIsError());
            iAudioFile.close();
        }
    }

    SECTION("fields of IAudioFile are set correctly") {
        auto run_test = [&iAudioFile] (const std::vector<std::string>& coll,
                                       double sr) {
            for (const auto & i : coll) {
                iAudioFile.open(i);
                CHECK(! iAudioFile.getIsError());

                REQUIRE(iAudioFile.getSamplingRate() == sr);
                REQUIRE(iAudioFile.getChannels() == 1);
                REQUIRE(iAudioFile.getFrames() == sr);
                REQUIRE(iAudioFile.getPosition() == 0);
                REQUIRE(iAudioFile.isOpen());

                iAudioFile.close();
            }
        };

        run_test({file_names[0], file_names[1], file_names[2]}, 8000);
        run_test({file_names[3], file_names[4], file_names[5]}, 32000);
        run_test({file_names[6], file_names[7], file_names[8]}, 44100);
        run_test({file_names[9], file_names[10], file_names[11]}, 48000);
        run_test({file_names[12], file_names[13], file_names[14]}, 96000);
    }

    SECTION("can read some data out") {
        auto run_test = [&iAudioFile] (const std::vector<std::string>& coll,
                                       double sr) {
            for (const auto & i : coll) {
                iAudioFile.open(i);
                CHECK(! iAudioFile.getIsError());

                std::streampos orig_pos = iAudioFile.getPosition();
                std::vector<float> fdat(sr);
                iAudioFile.readInterleaved(fdat.data(), sr);
                std::streampos new_pos = iAudioFile.getPosition();

                REQUIRE(new_pos == orig_pos + std::streampos(sr));

                iAudioFile.close();

                iAudioFile.open(i);
                CHECK(! iAudioFile.getIsError());

                orig_pos = iAudioFile.getPosition();
                std::vector<double> ddat(sr);
                iAudioFile.readInterleaved(ddat.data(), sr);
                new_pos = iAudioFile.getPosition();

                REQUIRE(new_pos == orig_pos + std::streampos(sr));

                iAudioFile.close();
            }
        };
        run_test({file_names[0], file_names[1], file_names[2]}, 8000);
        run_test({file_names[3], file_names[4], file_names[5]}, 32000);
        run_test({file_names[6], file_names[7], file_names[8]}, 44100);
        run_test({file_names[9], file_names[10], file_names[11]}, 48000);
        run_test({file_names[12], file_names[13], file_names[14]}, 96000);
    }
}

