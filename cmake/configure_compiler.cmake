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

# Select the compiler specific cmake file
set(MSVC_ID "MSVC")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/clang.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/gcc.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL ${MSVC_ID})
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/msvc.cmake)
else()
  message(FATAL_ERROR "Unknown compiler!")
endif()

set(BUILD_SHARED_LIBS ON)
