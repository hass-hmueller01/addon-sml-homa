# compiler/linker flags
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-pedantic)
    add_definitions(-D_FORTIFY_SOURCE=2)
    #add_definitions(-fPIE -pie)
    add_definitions(-Wl,-z,relro,-z,now)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
        add_definitions(-fstack-protector-strong)
    endif()
    if(OPTION_USE_GCOV)
        add_definitions(-g -O0 --coverage)
    endif()
    if(OPTION_USE_GPROF)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
    endif()
endif()

# targets
add_executable(sml2mqtt "")

# search paths
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${LIBSML_INCLUDE_DIRS}
    ${LIBMOSQUITTOPP_INCLUDE_DIRS})

# sources/headers
target_sources(sml2mqtt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SML.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MqttClient.cpp)

# compiler/linker flags
set_target_properties(sml2mqtt PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})
target_link_libraries(sml2mqtt
    pthread
    yaml-cpp
    ${LIBSML_LIBRARIES}
    ${LIBMOSQUITTOPP_LIBRARIES})

# install
install(
    TARGETS sml2mqtt
    DESTINATION ${CMAKE_INSTALL_SBINDIR})
if(OPTION_WITH_SYSTEMD)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/sml2mqtt.service
        DESTINATION lib/systemd/system)
endif(OPTION_WITH_SYSTEMD)
install(CODE "
   if(NOT EXISTS \"/etc/sml2mqtt.yaml\")
       file(INSTALL \"${CMAKE_CURRENT_SOURCE_DIR}/sml2mqtt.yaml\" DESTINATION \"/etc\" FILE_PERMISSIONS OWNER_WRITE OWNER_READ)
   else()
       message(STATUS \"Not-Installing: /etc/sml2mqtt.yaml - File already exists!\")
   endif()
")
