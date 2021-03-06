CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/cmake/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/tools/cmake/")

PROJECT(meteoio)
# Version of library
SET(VERSION_MAJOR 2 CACHE INTERNAL "")
SET(VERSION_MINOR 4 CACHE INTERNAL "")
SET(VERSION_PATCH 2 CACHE INTERNAL "")

SET(POPCC popcc)
SET(LIBPREFIX "lib") #make sure ALL plugins are libXXX for ALL plateforms
SET(CMAKE_IMPORT_LIBRARY_PREFIX "lib") #for Visual C++ import libraries

###########################################################
#platform specific. First, compiler specific flags, then system specific flags
OPTION(BUILD_SHARED_LIBS "Set to ON to build dynamic libraries" ON)
IF(WIN32)
	SET(BUILD_STATIC_LIBS ON) #otherwise the lib files are not generated
ELSE(WIN32)
	OPTION(BUILD_STATIC_LIBS "Set to ON to build static libraries" OFF)
ENDIF(WIN32)
INCLUDE("${CMAKE_SOURCE_DIR}/tools/cmake/BuildVersion.cmake")
BuildVersion()

IF(MSVC)
	#SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs"  FORCE)
	SET(WARNINGS "/W4")
	#SET(EXTRA_WARNINGS "/Wp64") #/Wall
	SET(OPTIM "/O2 /DNDEBUG /MD /DNOSAFECHECKS")
	SET(ARCH_OPTIM "/arch:SSE2")
	SET(ARCH_SAFE "")
	SET(DEBUG "/Z7 /Od /D__DEBUG /MDd")
	LIST(APPEND CFLAGS " /D_CRT_SECURE_NO_WARNINGS /EHsc") #Za: strict ansi EHsc: handle c++ exceptions
	SET(_VERSION "/D_VERSION=\\\"${_versionString}\\\"")
	IF(BUILD_SHARED_LIBS)
		ADD_DEFINITIONS(/DMIO_DLL)
	ENDIF(BUILD_SHARED_LIBS)
	IF(GUI_EXCEPTIONS)
		SET(MSG_BOX "/DMESG_BOX")
	ENDIF(GUI_EXCEPTIONS)
ELSE(MSVC)
	#we consider that all other compilers support "-" options and silently ignore what they don't know
	SET(WARNINGS "-Wall -Wno-long-long")
	SET(PROFILING "-pg -fprofile-arcs")
	SET(DEEP_WARNINGS "-Wunused-value -Wshadow -Wpointer-arith -Wconversion -Winline -Wdisabled-optimization") #-Wfloat-equal -Wpadded
	IF(POPC)
		SET(EXTRA_WARNINGS "-Wextra -Wno-ignored-qualifiers")
	ELSE(POPC)
		SET(EXTRA_WARNINGS "-Wextra -pedantic -Weffc++ ${DEEP_WARNINGS}") #-Weffc++
	ENDIF(POPC)
	IF(CMAKE_COMPILER_IS_GNUCXX AND NOT ANDROID)
		SET(EXTRA_WARNINGS "${EXTRA_WARNINGS} -ansi") #Android does not like ansi...
		IF(WIN32) #for gcc on windows
			SET(CMAKE_SHARED_LINKER_FLAGS "--enable-auto-import")
		ENDIF(WIN32)
	ENDIF(CMAKE_COMPILER_IS_GNUCXX AND NOT ANDROID)
	SET(OPTIM "-g -O3 -DNDEBUG -DNOSAFECHECKS") #-flto for gcc>4.5?
	EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	IF (GCC_VERSION VERSION_GREATER 4.2 OR GCC_VERSION VERSION_EQUAL 4.2)
		SET(ARCH_OPTIM  "-march=native -mtune=native")
	ENDIF()
	SET(ARCH_SAFE  "")
	SET(DEBUG "-g3 -O0 -D__DEBUG")
	SET(_VERSION "-D_VERSION=\\\"${_versionString}\\\"")
	#IF(BUILD_SHARED_LIBS)
	#	ADD_DEFINITIONS(-DMIO_DLL)
	#ENDIF(BUILD_SHARED_LIBS)
ENDIF(MSVC)

#The following pre-processor variables are automatically defined:
# __linux, __GNUC__, __CYGWIN__, _WIN32, __MINGW32__, __APPLE__
IF(UNIX)
	IF(APPLE)
		SET(DLL_EXT "dylib*")
		SET(STAT_EXT "a")
		SET(EXTRA_WARNINGS "${EXTRA_WARNINGS} -Wno-sign-conversion") #for now, keep clang quiet
		SET(CMAKE_SHARED_LINKER_FLAGS "-framework CoreFoundation")
		#set(CMAKE_OSX_SYSROOT iphoneos2.2.1) #for IOS
		#set(CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_BIT)) #for IOS
	ELSE(APPLE)
		SET(DLL_EXT "so*")
		SET(STAT_EXT "a")
	ENDIF(APPLE)
ELSE(UNIX)
	IF(WIN32)
		SET(DLL_EXT "dll")
		SET(STAT_EXT "a")
	ENDIF(WIN32)
ENDIF(UNIX)

IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "release" CACHE STRING "Choose the type of build, options are: debug release coverage cxxflags" FORCE)
	SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
ENDIF(NOT CMAKE_BUILD_TYPE)

###########################################################
#targets providing SETs of compiler options
IF(NOT DEST)
	SET(DEST "safe" CACHE STRING "Choose safe, optimized, hera" FORCE)
ENDIF(NOT DEST)

IF (DEST STREQUAL "hera")
	SET(ARCH  "-march=amdfam10")
ENDIF(DEST STREQUAL "hera")

IF (DEST STREQUAL "safe")
	SET(ARCH  "${ARCH_SAFE}")
ENDIF(DEST STREQUAL "safe")

IF(DEST STREQUAL "optimized")
	SET(ARCH  "${ARCH_OPTIM}")
ENDIF(DEST STREQUAL "optimized")

#build for use with POPC in parallel objects
SET(POPC OFF CACHE BOOL "Compile for POPC ON or OFF")

#show exception messages in a graphical message box
SET(GUI_EXCEPTIONS OFF CACHE BOOL "Show a message box with exceptions texts ON or OFF")

###########################################################
#finally, SET compile flags
IF(POPC)
	#SET(POPC_FLAGS "-cxxmain -Dcerr=\\\"cout\\\"")
	SET(POPC_FLAGS "-cxxmain")
ENDIF(POPC)

SET(CMAKE_CXX_FLAGS "${POPC_FLAGS} ${_VERSION} ${ARCH}" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE "${OPTIM}" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_DEBUG "${DEBUG} ${WARNINGS} ${EXTRA_WARNINGS}" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_CXXFLAGS "$ENV{CXXFLAGS}" CACHE STRING "" FORCE)
MARK_AS_ADVANCED(FORCE CMAKE_CXX_FLAGS_CXXFLAGS)

#Add subdirectory, so they can inherit all variables defined above
ADD_SUBDIRECTORY(meteoio/plugins)
ADD_SUBDIRECTORY(meteoio/meteolaws)
ADD_SUBDIRECTORY(meteoio/meteofilters)
ADD_SUBDIRECTORY(meteoio)
###########################################################
#plugins
SET(PLUGIN_A3DIO ON CACHE BOOL "Compilation A3DIO ON or OFF")
SET(PLUGIN_ARCIO ON CACHE BOOL "Compilation ARCIO ON or OFF")
SET(PLUGIN_ARPSIO ON CACHE BOOL "Compilation ARPSIO ON or OFF")
SET(PLUGIN_BORMAIO OFF CACHE BOOL "Compilation BORMAIO ON or OFF")
SET(PLUGIN_COSMOXMLIO OFF CACHE BOOL "Compilation COSMOXMLIO ON or OFF")
SET(PLUGIN_GEOTOPIO ON CACHE BOOL "Compilation GEOTOPIO ON or OFF")
SET(PLUGIN_GRASSIO ON CACHE BOOL "Compilation GRASSIO ON or OFF")
SET(PLUGIN_GRIBIO OFF CACHE BOOL "Compilation GRIBIO ON or OFF")
SET(PLUGIN_GSNIO OFF CACHE BOOL "Compilation GSNIO ON or OFF")
SET(PLUGIN_IMISIO OFF CACHE BOOL "Compilation IMISIO ON or OFF")
SET(PLUGIN_NETCDFIO OFF CACHE BOOL "Compilation NETCDFIO ON or OFF")
SET(PLUGIN_PGMIO ON CACHE BOOL "Compilation PGMIO ON or OFF")
SET(PLUGIN_PNGIO OFF CACHE BOOL "Compilation PNGIO ON or OFF")
SET(PLUGIN_PSQLIO OFF CACHE BOOL "Compilation PSQLIO ON or OFF")
SET(PLUGIN_SMETIO ON CACHE BOOL "Compilation SMETIO ON or OFF")
SET(PLUGIN_SNIO ON CACHE BOOL "Compilation SNIO ON or OFF")
SET(PROJ4 OFF CACHE BOOL "Use PROJ4 for the class MapProj ON or OFF")
SET(DATA_QA OFF CACHE BOOL "Data Quality Assurance outputs ON or OFF")

###########################################################
#for the install target
FILE(GLOB hfiles "meteoio/*.h")
FILE(GLOB hpluginfiles "meteoio/plugins/*.h")
FILE(GLOB hlawsfiles "meteoio/meteolaws/*.h")
FILE(GLOB hfilterfiles "meteoio/meteofilters/*.h")
FILE(GLOB hstatsfiles "meteoio/meteostats/*.h")

IF(DEST MATCHES "par")
	FILE(GLOB phfiles "${CMAKE_CURRENT_SOURCE_DIR}/meteoio/*.ph")
	FILE(GLOB modules "${CMAKE_BINARY_DIR}/lib/*.module")
	INSTALL(FILES ${modules} DESTINATION lib/meteoio COMPONENT libraries)
ENDIF(DEST MATCHES "par")
INSTALL(FILES ${hfiles} ${phfiles} DESTINATION include/meteoio COMPONENT headers)
INSTALL(FILES ${hpluginfiles} DESTINATION include/meteoio/plugins COMPONENT headers)
INSTALL(FILES ${hlawsfiles} DESTINATION include/meteoio/meteolaws COMPONENT headers)
INSTALL(FILES ${hfilterfiles} DESTINATION include/meteoio/meteofilters COMPONENT headers)
INSTALL(FILES ${hstatsfiles} DESTINATION include/meteoio/meteostats COMPONENT headers)

IF(WIN32) #only having it in meteoio/CMake... is not enough...
	FILE(GLOB_RECURSE importlibs "lib/libmeteo*.lib")
	FILE(GLOB_RECURSE dlls "lib/libmeteo*.dll")
	INSTALL(FILES ${dlls} ${importlibs} DESTINATION lib COMPONENT libraries)
ENDIF(WIN32)
IF(APPLE) #only having it in meteoio/CMake... is not enough...
	FILE(GLOB_RECURSE dlls "lib/libmeteo*.dylib")
	INSTALL(FILES ${dlls} DESTINATION lib COMPONENT libraries)
ENDIF(APPLE)

ADD_CUSTOM_TARGET(distclean make clean
	COMMAND cmake -E remove CMakeCache.txt CPackSourceConfig.cmake CPackConfig.cmake CTestTestfile.cmake DartConfiguration.tcl cmake_install.cmake cmake_uninstall.cmake install_manifest.txt Makefile
	COMMAND cmake -E remove "lib/libmeteo*.${DLL_EXT}" "lib/libmeteo*.${STAT_EXT}" "lib/libmeteo*.lib"
	COMMAND cmake -E remove */CMakeCache.txt */CTestTestfile.cmake */cmake_install.cmake
	COMMAND cmake -E remove */*/CMakeCache.txt */*/CTestTestfile.cmake */*/cmake_install.cmake
	COMMAND cmake -E remove_directory CMakeFiles
	COMMAND cmake -E remove_directory Testing
	COMMAND cmake -E remove_directory meteoio/CMakeFiles
	COMMAND cmake -E remove_directory meteoio/meteofilters/CMakeFiles
	COMMAND cmake -E remove_directory meteoio/meteolaws/CMakeFiles
	COMMAND cmake -E remove_directory meteoio/plugins/gsn/CMakeFiles
	COMMAND cmake -E remove_directory meteoio/plugins/CMakeFiles
	COMMAND cmake -E remove_directory tests/CMakeFiles
	COMMAND cmake -E remove_directory tests/Testing
	COMMAND cmake -E remove_directory tests/meteo_reading_interpol/CMakeFiles
	COMMAND cmake -E remove_directory tests/meteo_reading_interpol/Testing
	COMMAND cmake -E remove_directory tests/meteo_reading_no_interpol/CMakeFiles
	COMMAND cmake -E remove_directory tests/meteo_reading_no_interpol/Testing
	COMMAND cmake -E remove_directory tests/sun/CMakeFiles
	COMMAND cmake -E remove_directory tests/sun/Testing
	COMMAND cmake -E remove tests/sun/generated_results.txt
	COMMAND cmake -E remove_directory tests/dem_reading/CMakeFiles
	COMMAND cmake -E remove_directory tests/dem_reading/Testing
	COMMAND cmake -E remove tests/dem_reading/AZI.asc tests/dem_reading/DEM.asc tests/dem_reading/SLOPE.asc
	COMMAND cmake -E remove_directory tests/2D_interpolations/CMakeFiles
	COMMAND cmake -E remove_directory tests/2D_interpolations/Testing
	COMMAND cmake -E remove tests/2D_interpolations/2009-01-19T12.00_HNW.asc tests/2D_interpolations/2009-01-19T12.00_RH.asc tests/2D_interpolations/2009-01-19T12.00_RSWR.asc tests/2D_interpolations/2009-01-19T12.00_TA.asc
)


###########################################################
#for the uninstall target
CONFIGURE_FILE(
	"${PROJECT_SOURCE_DIR}/tools/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#Build the documentation if doxygen is found
INCLUDE(FindDoxygen)
FIND_PACKAGE(Doxygen)
if (DOXYGEN_FOUND)
	ADD_CUSTOM_TARGET(doc "doxygen" ${PROJECT_SOURCE_DIR}/meteoio/config.dox)
ENDIF (DOXYGEN_FOUND)
#Install the documentation if the user desires it
SET(INSTALL_DOC OFF CACHE BOOL "Install documentation ON or OFF")
IF(INSTALL_DOC)
	IF(WIN32)
		SET(DOC_PATH "doc")
	ELSE(WIN32)
		SET(DOC_PATH "share/doc/${CMAKE_PROJECT_NAME}")
	ENDIF(WIN32)
	SET(DOC_FILES License.txt meteoio/icons/icon.ico doc/SMET_specifications.pdf)
	IF (DOXYGEN_FOUND)
		INSTALL(DIRECTORY doc/html DESTINATION ${DOC_PATH} COMPONENT documentation)
		LIST(APPEND DOC_FILES doc/MeteoIO_doc.html)
	ELSE (DOXYGEN_FOUND)
		MESSAGE("Doxygen has not been found and is needed to build online documentation. Please either install doxygen or download a documentation-only package!")
	ENDIF (DOXYGEN_FOUND)

	INSTALL(FILES ${DOC_FILES} DESTINATION ${DOC_PATH} COMPONENT documentation)
	INSTALL(DIRECTORY "doc/examples" DESTINATION ${DOC_PATH} COMPONENT examples PATTERN ".svn" EXCLUDE)
ENDIF(INSTALL_DOC)

###########################################################
## CPACK section
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION "Meteorological preprocessing library")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Meteorological Input/Output preprocessing library: it covers data retrieval, data filtering, data transformations and spatial interpolations. Please refer to the included documentation and the included code examples!")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/Welcome.txt")
SET(CPACK_PACKAGE_NAME "MeteoIO")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "MeteoIO")
SET(CPACK_PACKAGE_CONTACT "Mathias Bavay <Mathias.Bavay@wsl.ch>")
SET(CPACK_PACKAGE_VENDOR "WSL Institute for Snow and Avalanche Research")
SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/meteoio/icons\\\\icon.ico")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/Welcome.txt")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/Readme.txt")

IF (DEST MATCHES "par")
	SET(PACKAGE_EXTRA "-par")
ENDIF(DEST MATCHES "par")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
SET(VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#SET(CPACK_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
SET(CPACK_ARCH "i386")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}${PACKAGE_EXTRA}-${VERSION}-${CPACK_ARCH}")

#installation by components for formats that support it
#SET(CPACK_COMPONENTS_ALL_GROUPS_IN_ONE_PACKAGE 1)
SET(CPACK_COMPONENTS_ALL libraries headers documentation examples)
SET(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "MeteoIO library")
SET(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "The MeteoIO library itself")
SET(CPACK_COMPONENT_LIBRARIES_GROUP Runtime)
SET(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Header files")
SET(CPACK_COMPONENT_HEADERS_DESCRIPTION "Necessary C++ header files for developing applications using MeteoIO")
SET(CPACK_COMPONENT_HEADERS_GROUP Development)
SET(CPACK_COMPONENT_HEADERS_DEPENDS libraries)
IF(INSTALL_DOC)
	SET(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
	SET(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Documentation of the MeteoIO usage and API both for the end user and the developer using MeteoIO in his/her application")
	SET(CPACK_COMPONENT_DOCUMENTATION_GROUP Docs)
	SET(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
	SET(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "C++ Source code of various simple examples of applications with their example data")
	SET(CPACK_COMPONENT_EXAMPLES_DEPENDS libraries headers)
	SET(CPACK_COMPONENT_EXAMPLES_GROUP Docs)
	SET(CPACK_COMPONENT_DOCS_GROUP "Documentation")
	SET(CPACK_COMPONENT_GROUP_DOCS_DESCRIPTION "How to use MeteoIO as an end user and as a developer")
ENDIF(INSTALL_DOC)

SET(CPACK_COMPONENT_RUNTIME_GROUP "Runtime")
SET(CPACK_COMPONENT_GROUP_RUNTIME_DESCRIPTION "What you need to run an application based on MeteoIO")
SET(CPACK_COMPONENT_DEVELOPMENT_GROUP "Development")
SET(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "What you need to develop an application based on MeteoIO")

IF(UNIX)
	IF(APPLE)
		SET(CPACK_GENERATOR "PackageMaker")
		SET(CPACK_PACKAGING_INSTALL_PREFIX "/Applications/${CPACK_PACKAGE_NAME}")
	ELSE(APPLE)
		SET(CPACK_GENERATOR "DEB;RPM;TGZ")
		SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
		SET(CPACK_DEBIAN_PACKAGE_NAME "meteoio")
		SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
		SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6") #objdump -p lib/libmeteoio.so | grep NEEDED | tr -s ' ' | cut -d' ' -f3 | tr '\n' ','
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386") #dpkg --print-architecture
		SET(CPACK_RPM_PACKAGE_NAME "meteoio")
		SET(CPACK_RPM_PACKAGE_LICENSE "LPGLv3")
		SET(CPACK_RPM_PACKAGE_REQUIRES "libstdc++6") #simply libproj for OpenSuse
		SET(CPACK_RPM_PACKAGE_ARCHITECTURE "i386")
		SET(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
	ENDIF(APPLE)
ELSE(UNIX)
	IF(WIN32)
		SET(CPACK_GENERATOR "NSIS")
		SET(CPACK_NSIS_MUI_ICON "${CPACK_PACKAGE_ICON}")
		SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${VERSION}")
		SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}")
		IF(INSTALL_DOC)
			SET(CPACK_NSIS_MENU_LINKS
				"${DOC_PATH}/MeteoIO_doc.html" "MeteoIO documentation"
				"${DOC_PATH}/examples" "MeteoIO examples"
				"https://models.slf.ch/p/meteoio/" "Home page")
		ELSEIF(INSTALL_DOC)
			SET(CPACK_NSIS_MENU_LINKS
				"https://models.slf.ch/p/meteoio/" "Home page")
		ENDIF(INSTALL_DOC)
		#info for the add/uninstall software administration
		SET(CPACK_NSIS_INSTALLED_ICON_NAME "doc\\\\icon.ico")
		SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.slf.ch")
		SET(CPACK_NSIS_HELP_LINK "https://models.slf.ch/p/meteoio")
		SET(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
	ENDIF(WIN32)
ENDIF(UNIX)


SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${VERSION}" CACHE INTERNAL "tarball basename")
SET(CPACK_SOURCE_GENERATOR TGZ)
# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
SET(CPACK_SOURCE_IGNORE_FILES
	"~$"
	"\\\\.svn/"
	"/CMakeFiles/"
	"/CMakeCache*"
	"/cmake_install\\\\.cmake$"
	"/cmake_uninstall\\\\.cmake$"
	"^_CPack_.*/"
	"/CPack*"
	"\\\\.o$"
	"/m4/"
)

INCLUDE(CPack)

###########################################################
## CTest section
###########################################################

## Special build type configuration for Covarage Testing.
# Attantion: Coverage testing right now only works with g++ (or comercial tools)
## Add the tests itselfs
OPTION(BUILD_TESTING "Build the tests" OFF)
IF(BUILD_TESTING)

	# Coverage statistic rign now only supportet with GCC (GNUGXX)
	IF(CMAKE_COMPILER_IS_GNUCXX)
		OPTION(BUILD_TESTING_WITH_COVERAGE "Use gcc flags to make coverage tests" OFF)

		IF(BUILD_TESTING_WITH_COVERAGE)
			# force the correct flags to make coverage trought a build type
			SET(CMAKE_BUILD_TYPE coverage)
			SET(CMAKE_CXX_FLAGS_COVERAGE "-Wall -O0 -ftest-coverage -fprofile-arcs" CACHE STRING "" FORCE)
			MARK_AS_ADVANCED(FORCE CMAKE_CXX_FLAGS_COVERAGE)

			# Force debug mode, so that no optimisation is made to get "Correct" results of coverage testing
			SET(CMAKE_BUILD_TYPE "coverage" CACHE STRING "Choose the type of build, options are: debug release coverage " FORCE)
			SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

		ENDIF(BUILD_TESTING_WITH_COVERAGE)
	ENDIF(CMAKE_COMPILER_IS_GNUCXX)

	INCLUDE(CTest) # This makes ENABLE_TESTING() and gives support for Dashboard
	ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)
