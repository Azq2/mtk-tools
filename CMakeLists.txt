cmake_minimum_required(VERSION 2.8)

project (mtk_boot_tools) 

set (R ${PROJECT_SOURCE_DIR})

set (SOURCES
	src/common.cpp
)
set (HEADERS
	include/common.hpp
)

set (SOURCES_unpackbootimg
	src/mtk_unpackbootimg.cpp
)
set (SOURCES_container
	src/mtk_container.cpp
)
set (HEADERS_unpackbootimg
	include/mtk_unpackbootimg.hpp
)
set (HEADERS_container
	include/mtk_container.hpp
)

set (${CXXFLAGS} "${CXXFLAGS} -g ")

include_directories (include)
add_executable (mtk_unpackbootimg ${SOURCES_unpackbootimg} ${SOURCES} ${HEADERS} ${HEADERS_unpackbootimg})
add_executable (mtk_container ${SOURCES_container} ${SOURCES} ${HEADERS} ${HEADERS_container})

INSTALL(TARGETS mtk_unpackbootimg mtk_container
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
