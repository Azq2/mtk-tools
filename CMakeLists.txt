cmake_minimum_required(VERSION 2.8)

project (mtk_bootimg) 

set (SOURCES
	src/main.cpp
)
set (HEADERS
	
)

set (${CXXFLAGS} "${CXXFLAGS} -g ")

# include_directories (include)
add_executable (mtk_bootimg ${SOURCES} ${HEADERS})
