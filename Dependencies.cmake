# Parameters:
#   - TARGET
# Exports:
#   - THIRD_PARTY_INCLUDE_DIR
#   - THIRD_PARTY_LIBRARY_DIR

include(ExternalProject)
include(GNUInstallDirs)

add_dependencies(camlog libjpeg libexif)
set(THIRD_PARTY_INSTALL_DIR ${CMAKE_BINARY_DIR}/third_party/install)

### libjpeg-turbo from GitHub
ExternalProject_Add(libjpeg
  GIT_REPOSITORY        https://github.com/libjpeg-turbo/libjpeg-turbo.git
  GIT_TAG               20ade4d # 3.1.0
  SOURCE_DIR            ${CMAKE_BINARY_DIR}/third_party/libjpeg-src
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DENABLE_SHARED=OFF
    -DENABLE_STATIC=ON
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  UPDATE_DISCONNECTED   1 # prevent re-download of git repo
)

### libexif from GitHub (Autotools-based)
ExternalProject_Add(libexif
  GIT_REPOSITORY        https://github.com/libexif/libexif.git
  GIT_TAG               e99f147 # v0.6.25
  SOURCE_DIR            ${CMAKE_BINARY_DIR}/third_party/libexif-src
  CONFIGURE_COMMAND autoreconf -i && ./configure --prefix=${THIRD_PARTY_INSTALL_DIR} --host=${TARGET} --target=${TARGET}
  BUILD_COMMAND         make
  INSTALL_COMMAND       make install 
  BUILD_IN_SOURCE       1
  UPDATE_DISCONNECTED   1 # prevent re-download of git repo
)

# Provide include/lib paths to parent scope
set(THIRD_PARTY_INCLUDE_DIR ${THIRD_PARTY_INSTALL_DIR}/include CACHE PATH "Include dir for dependencies")
set(THIRD_PARTY_LIBRARY_DIR ${THIRD_PARTY_INSTALL_DIR}/lib CACHE PATH "Library dir for dependencies")
