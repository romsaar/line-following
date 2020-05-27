cmake_minimum_required(VERSION 2.8)
project( followlines )
find_package( OpenCV REQUIRED )
add_executable( followlines main.cpp )
target_link_libraries( followlines ${OpenCV_LIBS} )

