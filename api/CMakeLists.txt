cmake_minimum_required(VERSION 3.15)

project(
    "waifu_api"
    VERSION 0.1.0
    LANGUAGES CXX
)

option(${PROJECT_NAME}_INSTALL "Generate the install target" ON)

include(cmake/CPM.cmake)

cpmfindpackage(NAME "net" GITHUB_REPOSITORY "Xminent/net" GIT_TAG "main")

cpmaddpackage(
    NAME
    "nlohmann_json"
    VERSION
    "3.11.1"
    URL
    "https://github.com/nlohmann/json/releases/download/v3.11.1/include.zip"
    URL_HASH
    "SHA256=9c15ca7806f863872452bfbc85fee6d1c9868117e3ea5e00a204ae961a2e1ae7"
)

add_library(nlohmann_json INTERFACE)
target_include_directories(
    nlohmann_json
    INTERFACE $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>
)

set(${PROJECT_NAME}_TARGETS ${PROJECT_NAME} nlohmann_json)
set(SEPARATE_DEPENDENCIES "")

if(net_ADDED)
    list(APPEND ${PROJECT_NAME}_TARGETS net)
else()
    list(APPEND SEPARATE_DEPENDENCIES "net")
endif()

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE net::net nlohmann_json)

if(${PROJECT_NAME}_INSTALL)
    include(GNUInstallDirs)

    install(
        TARGETS ${${PROJECT_NAME}_TARGETS}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    install(DIRECTORY ${nlohmann_json_SOURCE_DIR}/include/nlohmann
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    set(CPACK_PACKAGE_VENDOR Xminent)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A networking library for C++")
    set(CPACK_PACKAGE_DESCRIPTION "A networking library for C++")
    set(CPACK_PACKAGE_CONTACT "mail@xminent.com")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "A networking library for C++")

    if(WIN32)
        set(CPACK_GENERATOR ZIP)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CPACK_GENERATOR "DEB;RPM")
    else()
        set(CPACK_GENERATOR "TBZ2")
    endif()

    include(CPack)
endif()
