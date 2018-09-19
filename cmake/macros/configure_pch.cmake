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
if (MSVC)
  # This workaround was verified to be required on MSVC (2017)
  set(COTIRE_PCH_MEMORY_SCALING_FACTOR 500)
endif()

include(${CMAKE_SOURCE_DIR}/dep/cotire/cotire/CMake/cotire.cmake)

set(IDLE_USE_PCH ON)

function(configure_pch TARGET_NAME_LIST PCH_HEADER)

  if (NOT IDLE_USE_PCH)
    return()
  endif()

  # Use the header for every target
  foreach(TARGET_NAME ${TARGET_NAME_LIST})
    # Disable unity builds
    set_target_properties(${TARGET_NAME} PROPERTIES COTIRE_ADD_UNITY_BUILD OFF)

    # Set the prefix header
    set_target_properties(${TARGET_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${PCH_HEADER})

    # Workaround for cotire bug: https://github.com/sakra/cotire/issues/138
    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 14)
  endforeach()

  cotire(${TARGET_NAME_LIST})
endfunction(configure_pch)
