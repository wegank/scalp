
# remove "ILP-"-prefix
foreach(I IN LISTS BACKEND_TARGETS)
  string(REPLACE "ILP-" "" NI "${I}")
  list(APPEND BACKENDS ${NI})
endforeach()

# remove Dynamic
list(REMOVE_ITEM BACKENDS "Dynamic")

# generate and build all tests in this directory.
file(GLOB V "*.cpp")
foreach(I IN LISTS V)
  # build the test
  string(REGEX MATCH "([^/]*)\\.cpp$" T ${I})
  set(basename ${CMAKE_MATCH_1})
  add_executable(${basename} "${T}")
  target_link_libraries(${basename} ILP ILP-Dynamic)

  # add the test(s)
  string(REGEX MATCH ".*(_ALL)$" ALL ${basename})
  if(ALL STREQUAL "")
    foreach(TARGET IN LISTS BACKENDS)
      add_test(NAME "${basename}_${TARGET}"
               COMMAND "${CMAKE_BINARY_DIR}/src/tests/${basename}" "${TARGET}"
               WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endforeach()
  else()
    add_test(NAME "${basename}"
             COMMAND "${CMAKE_BINARY_DIR}/src/tests/${basename}"
             WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif()
endforeach()
