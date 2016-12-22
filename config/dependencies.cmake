# This is where we download, configure and build all the dependencies.
# There's not a silver bullet here: every road leads to pain.
# This is the road of least pain (with the worst metaphors).
#
# For each library, we ask it to build a static lib (where appropriate) and to
# install into the build directory.
# The advantage of this approach is that we can deal with cmake and autotools
# builds in much the same way.
# Also cmake can magically download stuff from nearly anywhere which is neat.

include(ExternalProject)
find_package(Git REQUIRED)

set(DEPENDENCY_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dependencies)

set_directory_properties(PROPERTIES EP_PREFIX ${DEPENDENCY_INSTALL_PREFIX})

# Some dependencies depend on one-another - this ensures that we look for those
# dependencies within the project, *before* looking in the normal system
# location.
set(CMAKE_MODULE_PATH ${DEPENDENCY_INSTALL_PREFIX} ${CMAKE_MODULE_PATH})

set(GLOBAL_DEPENDENCY_CMAKE_FLAGS "-DCMAKE_MODULE_PATH=${DEPENDENCY_INSTALL_PREFIX}" "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")

set(ENV_SETTINGS "MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")

set(CUSTOM_MAKE_COMMAND
    BUILD_COMMAND make ${ENV_SETTINGS} 
)

# glm ##########################################################################

ExternalProject_Add(
    glm_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch 0.9.8.1 https://github.com/g-truc/glm.git glm_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> 
)

# assimp #######################################################################

ExternalProject_Add(
    assimp_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch v3.3.1 https://github.com/assimp/assimp.git assimp_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_STATIC_LIB=ON -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ZLIB=ON -DCMAKE_CXX_VISIBILITY_PRESET=hidden -DCMAKE_VISIBILITY_INLINES_HIDDEN=ON
)

add_library(assimp UNKNOWN IMPORTED)
add_dependencies(assimp assimp_external) 
set_property(TARGET assimp PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libassimp.a)

add_library(zlibstatic UNKNOWN IMPORTED)
add_dependencies(zlibstatic assimp_external) 
set_property(TARGET zlibstatic PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libzlibstatic.a)

# fftw3 float ##################################################################

ExternalProject_Add(
    fftwf_external
    URL http://fftw.org/fftw-3.3.5.tar.gz
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --enable-float --enable-shared=no
    ${CUSTOM_MAKE_COMMAND}
)

add_library(fftwf UNKNOWN IMPORTED)
add_dependencies(fftwf fftwf_external) 
set_property(TARGET fftwf PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libfftw3f.a)

# sndfile ######################################################################

ExternalProject_Add(
    sndfile_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch 1.0.27 https://github.com/erikd/libsndfile.git sndfile_external
    CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh && <SOURCE_DIR>/configure --disable-external-libs --prefix=<INSTALL_DIR> --enable-shared=no
    ${CUSTOM_MAKE_COMMAND}
)

add_library(sndfile UNKNOWN IMPORTED)
add_dependencies(sndfile sndfile_external) 
set_property(TARGET sndfile PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libsndfile.a)

# samplerate ###################################################################

# The default libsamplerate distribution uses a deprecated path to Carbon, so
# we have to manually patch it to modify the path before building.

ExternalProject_Add(
    samplerate_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 https://github.com/erikd/libsamplerate.git samplerate_external
    PATCH_COMMAND ${GIT_EXECUTABLE} apply ${CMAKE_SOURCE_DIR}/config/fix_carbon.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh && <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-dependency-tracking --enable-shared=no
    ${CUSTOM_MAKE_COMMAND}
)

add_library(samplerate UNKNOWN IMPORTED)
add_dependencies(samplerate samplerate_external) 
set_property(TARGET samplerate PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libsamplerate.a)

# gtest ########################################################################

ExternalProject_Add(
    gtest_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch release-1.8.0 https://github.com/google/googletest.git gtest_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -Dgtest_hide_internal_symbols=on
)

add_library(gtest UNKNOWN IMPORTED)
add_dependencies(gtest gtest_external) 
set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libgtest.a)

# cereal #######################################################################

ExternalProject_Add(
    cereal_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch v1.2.1 https://github.com/USCiLab/cereal.git cereal_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DJUST_INSTALL_CEREAL=ON
)

# itpp #########################################################################

ExternalProject_Add(
    fftw_external
    URL http://fftw.org/fftw-3.3.5.tar.gz
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
    ${CUSTOM_MAKE_COMMAND}
)

add_library(fftw UNKNOWN IMPORTED)
add_dependencies(fftw fftw_external)
set_property(TARGET fftw PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libfftw3.a)

# The only tag, r4.3.0, has a bunch of build warnings, so we just get the most
# recent one and hope that it's less broken and compatible.
# We also hobble the library by hiding some packages from it, which makes
# linking a bit simpler.

ExternalProject_Add(
    itpp_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 git://git.code.sf.net/p/itpp/git itpp_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DITPP_SHARED_LIB=off -DHTML_DOCS=off #-DCMAKE_DISABLE_FIND_PACKAGE_BLAS=on -DCMAKE_DISABLE_FIND_PACKAGE_LAPACK=on
)

add_dependencies(itpp_external fftw_external)

add_library(itpp UNKNOWN IMPORTED)
add_dependencies(itpp itpp_external)
set_property(TARGET itpp PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libitpp_static.a)

find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)
set(ITPP_LIBRARIES itpp fftw ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})

# opencl #######################################################################

ExternalProject_Add(
    opencl_cpp_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch v2.0.10 https://github.com/KhronosGroup/OpenCL-CLHPP.git opencl_cpp_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>/include -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF
)

find_package(OpenCL REQUIRED)

################################################################################
# APP STUFF ####################################################################
################################################################################

# modern_gl_utils ##############################################################

ExternalProject_Add(
    modern_gl_utils_external
    DOWNLOAD_COMMAND ${GIT_EXECUTABLE} clone --depth 1 https://github.com/reuk/modern_gl_utils.git modern_gl_utils_external
    CMAKE_ARGS ${GLOBAL_DEPENDENCY_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

add_dependencies(modern_gl_utils_external glm_external)

add_library(modern_gl_utils UNKNOWN IMPORTED)
add_dependencies(modern_gl_utils modern_gl_utils_external)
set_property(TARGET modern_gl_utils PROPERTY IMPORTED_LOCATION ${DEPENDENCY_INSTALL_PREFIX}/lib/libmodern_gl_utils.a)
