cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
set(EXTRA_COMPONENT_DIRS "./components/")
# idf_build_set_property(MINIMAL_BUILD ON)
project(esp32p4-lcd)
