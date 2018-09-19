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
cmake_minimum_required(VERSION 3.11)
project(fib VERSION 0.7.0 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
enable_testing()

include(cmake/CMakeLists.txt)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(dep)
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)
