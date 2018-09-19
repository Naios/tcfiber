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
add_executable(test-ai
  ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

target_link_libraries(test-ai
  PRIVATE
    fib-lib-base
  PUBLIC
    fib)

set_target_properties(test-ai
  PROPERTIES
    FOLDER "test")

add_test(NAME fib-ai-tests
         COMMAND test-ai)
