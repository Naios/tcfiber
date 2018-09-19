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

add_library(fib-dep-base INTERFACE)
target_link_libraries(fib-dep-base
  INTERFACE
    fib-project-base)

target_compile_options(fib-dep-base
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
      /W0
      /wd4244
      /wd4267>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
      -w>)

target_compile_definitions(fib-dep-base
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
      _SCL_SECURE_NO_WARNINGS
      _CRT_SECURE_NO_WARNINGS>)
