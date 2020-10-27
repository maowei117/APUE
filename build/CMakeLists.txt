cmake_minimum_required(VERSION 3.0)
project(APUE)

# chapter2
set(CHAPTER2_SRC "../chapter2/")
add_executable(2_3 ${CHAPTER2_SRC}/2_3.c) 

# chapter3
set(CHAPTER3_SRC "../chapter3/")
add_executable(3_2 ${CHAPTER3_SRC}/3_2.c) 
add_executable(3_4 ${CHAPTER3_SRC}/3_4.c) 
add_executable(3_6 ${CHAPTER3_SRC}/3_6.c) 

# chapter4
set(CHAPTER4_SRC "../chapter4/")
add_executable(4_1 ${CHAPTER4_SRC}/4_1.c) 
add_executable(4_6 ${CHAPTER4_SRC}/4_6.c) 
add_executable(make_hole_file ${CHAPTER4_SRC}/make_hole_file.c) 