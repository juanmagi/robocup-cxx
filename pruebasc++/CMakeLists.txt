cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(prueba LANGUAGES CXX)
project(websocket-server LANGUAGES CXX)
project(websocket-client LANGUAGES CXX)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(prueba
    prueba.cpp
    CPruebaPython.cpp
    CWSCliente.cpp
    CPyWP.cpp
    )

add_executable(websocket-server
    websocket_server_sync.cpp
    )

add_executable(websocket-client
    websocket_client_sync.cpp
    )

include_directories(boost/include)

#target_link_libraries(AAGCloudWatcher indiclient ${ZLIB_LIBRARY} ${NOVA_LIBRARIES})
target_link_libraries(prueba 
    )

target_link_libraries(websocket-server 
    /pruebas-robocup/boost/lib/libboost_date_time.a
    /pruebas-robocup/boost/lib/libboost_regex.a
    /pruebas-robocup/boost/lib/libboost_serialization.a
    /pruebas-robocup/boost/lib/libboost_system.a
    /pruebas-robocup/boost/lib/libboost_thread.a
)

target_link_libraries(websocket-client 
    /pruebas-robocup/boost/lib/libboost_date_time.a
    /pruebas-robocup/boost/lib/libboost_regex.a
    /pruebas-robocup/boost/lib/libboost_serialization.a
    /pruebas-robocup/boost/lib/libboost_system.a
    /pruebas-robocup/boost/lib/libboost_thread.a
)

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(Python3)
set(PYTHON_INCLUDE_PATH /usr/include/python3.7)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(CPruebaPython.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(CPruebaPython.i PROPERTIES SWIG_FLAGS "-includeall")
#SWIG_ADD_MODULE(CPruebaPython python CPruebaPython.i CPruebaPython.cpp)
SWIG_ADD_LIBRARY(CPruebaPython LANGUAGE python SOURCES CPruebaPython.i CPruebaPython.cpp)
SWIG_LINK_LIBRARIES(CPruebaPython ${PYTHON_LIBRARIES})

SET_SOURCE_FILES_PROPERTIES(CPyWP.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(CPyWP.i PROPERTIES SWIG_FLAGS "-includeall")
#SWIG_ADD_MODULE(CPruebaPython python CPruebaPython.i CPruebaPython.cpp)
SWIG_ADD_LIBRARY(CPyWP LANGUAGE python SOURCES CPyWP.i CPyWP.cpp CWSCliente.cpp)
SWIG_LINK_LIBRARIES(CPyWP ${PYTHON_LIBRARIES} )
