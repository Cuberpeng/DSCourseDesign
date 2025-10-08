# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\DSCourseDesign_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\DSCourseDesign_autogen.dir\\ParseCache.txt"
  "DSCourseDesign_autogen"
  )
endif()
