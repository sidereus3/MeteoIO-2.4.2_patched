INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/")

INCLUDE("${PROJECT_SOURCE_DIR}/meteoio/plugins/CMakeLists.txt")
INCLUDE("${PROJECT_SOURCE_DIR}/meteoio/meteolaws/CMakeLists.txt")
INCLUDE("${PROJECT_SOURCE_DIR}/meteoio/meteostats/CMakeLists.txt")
INCLUDE("${PROJECT_SOURCE_DIR}/meteoio/meteofilters/CMakeLists.txt")

IF(PROJ4)
	FIND_PATH(LIBPROJ_INCLUDE_DIR "proj_api.h")
	FIND_LIBRARY(LIBPROJ proj)
	INCLUDE_DIRECTORIES(${LIBPROJ_INCLUDE_DIR})
	IF(MSVC)
		ADD_DEFINITIONS(/DPROJ4) #it looks like some VC++ versions don't support -D syntax
	ELSE(MSVC)
		ADD_DEFINITIONS(-DPROJ4)
	ENDIF(MSVC)
ENDIF(PROJ4)

IF(DATA_QA)
	IF(MSVC)
		ADD_DEFINITIONS(/DDATA_QA) #it looks like some VC++ versions don't support -D syntax
	ELSE(MSVC)
		ADD_DEFINITIONS(-DDATA_QA)
	ENDIF(MSVC)
ENDIF(DATA_QA)

IF(POPC)
	#FIND_PACKAGE(Popc REQUIRED)
	SET(popc_sources marshal_meteoio.cc)
	SET(CMAKE_CXX_COMPILER popcc) #this does not work, the user still has to do it manually
	#INCLUDE(CMakeForceCompiler)
	#CMAKE_FORCE_CXX_COMPILER(popcc POPCC)
	SET(POPC_EXT "popc")
ENDIF(POPC)

SET(meteoio_sources
	IOManager.cc
	Matrix.cc
	IOPlugin.cc
	MeteoProcessor.cc
	ResamplingAlgorithms.cc
	ResamplingAlgorithms2D.cc
	Meteo1DInterpolator.cc
	InterpolationAlgorithms.cc
	DataGenerator.cc
	GeneratorAlgorithms.cc
	Date.cc
	Timer.cc
	Grid2DObject.cc
	IOHandler.cc
	Coords.cc
	Graphics.cc
	Meteo2DInterpolator.cc
	BufferedIOHandler.cc
	DEMObject.cc
	Grid3DObject.cc
	IOInterface.cc
	StationData.cc
	Config.cc
	IOExceptions.cc
	IOUtils.cc
	FileUtils.cc
	MeteoData.cc
	plugins/libsmet.cc
	${plugins_sources}
	${meteolaws_sources}
	${meteostats_sources}
	${meteofilters_sources}
	${popc_sources}
)
IF(GUI_EXCEPTIONS)
	IF(MSVC)
		ADD_DEFINITIONS(/DMSG_BOX)
	ELSE(MSVC)
		ADD_DEFINITIONS(-DMSG_BOX)
	ENDIF(MSVC)
	IF(UNIX AND NOT APPLE AND NOT CYGWIN)
		FIND_PACKAGE(X11)
		IF(X11_FOUND)
			INCLUDE_DIRECTORIES(SYSTEM ${X11_INCLUDE_DIR})
			SET(GUI_LIBS ${X11_LIBRARIES})
			LIST(APPEND meteoio_sources MessageBoxX11.cc)
		ELSE(X11_FOUND)
			SET(GUI_EXCEPTIONS) #unsets the variable
		ENDIF(X11_FOUND)
	ENDIF(UNIX AND NOT APPLE AND NOT CYGWIN)
ENDIF(GUI_EXCEPTIONS)

#shared library.
IF(BUILD_SHARED_LIBS)
	SET(SHAREDNAME ${PROJECT_NAME}${POPC_EXT})
	ADD_LIBRARY(${SHAREDNAME} ${meteoio_sources})
	TARGET_LINK_LIBRARIES(${SHAREDNAME} ${plugin_libs} ${LIBPROJ} ${Popc_LIBRARIES} ${EXTRA_LINK_FLAGS} ${GUI_LIBS})
	SET_TARGET_PROPERTIES(${SHAREDNAME} PROPERTIES
		PREFIX "${LIBPREFIX}"
		LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
		CLEAN_DIRECT_OUTPUT 1
		VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
		SOVERSION "${VERSION_MAJOR}"
	)
	INSTALL(TARGETS ${SHAREDNAME}
		RUNTIME DESTINATION lib
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		COMPONENT libraries
	)
ENDIF(BUILD_SHARED_LIBS)

#static library. Please keep in mind that for Windows, a dll is a runtime
IF(BUILD_STATIC_LIBS)
	SET(STATICNAME ${PROJECT_NAME}_STATIC)
	SET(STATICLIBNAME ${PROJECT_NAME}${POPC_EXT})
	ADD_LIBRARY(${STATICNAME} STATIC ${meteoio_sources})
	TARGET_LINK_LIBRARIES(${STATICNAME} ${plugin_libs} ${LIBPROJ} ${Popc_LIBRARIES} ${EXTRA_LINK_FLAGS} ${GUI_LIBS})
	SET_TARGET_PROPERTIES(${STATICNAME} PROPERTIES
		PREFIX "${LIBPREFIX}"
		LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
		CLEAN_DIRECT_OUTPUT 1
		OUTPUT_NAME ${STATICLIBNAME}
	)
	INSTALL(TARGETS ${STATICNAME}
		RUNTIME DESTINATION lib
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		COMPONENT libraries
	)
ENDIF(BUILD_STATIC_LIBS)

IF(WIN32) #handle the special case of WIN32 needing import libraries
	FILE(GLOB_RECURSE importlibs "lib/libmeteo*.lib")
	FILE(GLOB_RECURSE dlls "lib/libmeteo*.dll") #bug: otherwise, dlls are missing
	INSTALL(FILES ${dlls} ${importlibs} DESTINATION lib COMPONENT libraries)
ENDIF(WIN32)
