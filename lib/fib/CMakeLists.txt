# Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
add_library(fib STATIC
  # Public headers for convenience
  ${CMAKE_SOURCE_DIR}/include/Async.h
  ${CMAKE_SOURCE_DIR}/include/AsyncImpl.h
  ${CMAKE_SOURCE_DIR}/include/Await.h
  ${CMAKE_SOURCE_DIR}/include/Awaitable.h
  ${CMAKE_SOURCE_DIR}/include/Event.h
  ${CMAKE_SOURCE_DIR}/include/Future.h
  ${CMAKE_SOURCE_DIR}/include/Fiber.h
  ${CMAKE_SOURCE_DIR}/include/FiberPool.h
  ${CMAKE_SOURCE_DIR}/include/StackReference.h
  ${CMAKE_SOURCE_DIR}/include/StackListReference.h
  ${CMAKE_SOURCE_DIR}/include/IntrusivePtr.h
  ${CMAKE_SOURCE_DIR}/include/AsyncCreatureAI.h
  ${CMAKE_SOURCE_DIR}/include/Traverse.h
  ${CMAKE_SOURCE_DIR}/include/WhenAll.h
  ${CMAKE_SOURCE_DIR}/include/WhenAny.h
  # Private sources and headers
  ${CMAKE_CURRENT_LIST_DIR}/Fiber.cpp
  ${CMAKE_CURRENT_LIST_DIR}/FiberPool.cpp
)

target_include_directories(fib
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(fib
  PUBLIC
    boost
)

target_compile_options(fib
  PUBLIC
    # $<$<CXX_COMPILER_ID:MSVC>:/GL>
    $<$<CXX_COMPILER_ID:MSVC>:/EHs>
)

set_target_properties(fib
  PROPERTIES
    FOLDER "lib")
