cmake_minimum_required(VERSION 3.0.0)
project(robocupcxx VERSION 0.1.0)
project(test_movil VERSION 0.1.0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    include_directories(includeDebug)
    set(EJECUTABLE_robocupcxx robocupcxxDEB)
    set(EJECUTABLE_test_movil test_movilDEB)
ELSE()
    include_directories(includeRelease)
    set(EJECUTABLE_robocupcxx robocupcxx)
    set(EJECUTABLE_test_movil test_movil)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

include(CTest)
enable_testing()

set(BOOST_LIBRARY ${CMAKE_SOURCE_DIR}/boost/lib)
set(LOG4CXX_LIBRARY ${CMAKE_SOURCE_DIR}/log4cxx/lib/liblog4cxx.so)
set(PIGPIO_LIBRARY ${CMAKE_SOURCE_DIR}/pigpio/lib/libpigpiod_if2.so)

add_executable(robocupcxx main.cpp CConfig.cpp)
add_executable(test_movil test_movil.cpp CConfig.cpp)

include_directories(boost/include
        log4cxx/include
        pigpio/include
        )

target_link_libraries(robocupcxx
    ${LOG4CXX_LIBRARY}
    ${PIGPIO_LIBRARY}
    ${BOOST_LIBRARY}/libboost_date_time.a    
    ${BOOST_LIBRARY}/libboost_regex.a    
    ${BOOST_LIBRARY}/libboost_serialization.a    
    ${BOOST_LIBRARY}/libboost_system.a    
    ${BOOST_LIBRARY}/libboost_thread.a    
)

target_link_libraries(test_movil 
    ${LOG4CXX_LIBRARY}
    ${PIGPIO_LIBRARY}
    /usr/lib/arm-linux-gnueabihf/libserial.so
    )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_command(
        TARGET robocupcxx POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/robocupcxx
        ${CMAKE_SOURCE_DIR}/bin/${EJECUTABLE_robocupcxx}
        )

add_custom_command(
        TARGET test_movil POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/test_movil
        ${CMAKE_SOURCE_DIR}/bin/${EJECUTABLE_test_movil}
        )