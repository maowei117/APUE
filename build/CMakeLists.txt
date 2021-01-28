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
add_executable(4_11 ${CHAPTER4_SRC}/4_11.c) 
add_executable(4_12 ${CHAPTER4_SRC}/4_12.c) 
add_executable(4_16 ${CHAPTER4_SRC}/4_16.c) 
add_executable(4_17 ${CHAPTER4_SRC}/4_17.c) 
add_executable(make_hole_file ${CHAPTER4_SRC}/make_hole_file.c) 
add_executable(my_unlink ${CHAPTER4_SRC}/unlink.c) 

# chapter5
set(CHAPTER5_SRC "../chapter5/")
add_executable(5_2 ${CHAPTER5_SRC}/5_2.c) 
add_executable(5_3 ${CHAPTER5_SRC}/5_3.c) 

# chapter6
set(CHAPTER6_SRC "../chapter6/")
add_executable(6_2 ${CHAPTER6_SRC}/6_2.c) 
add_executable(6_3 ${CHAPTER6_SRC}/6_3.c) 
add_executable(6_4 ${CHAPTER6_SRC}/6_4.c) 
add_executable(6_5 ${CHAPTER6_SRC}/6_5.c) 

# chapter7
set(CHAPTER7_SRC "../chapter7/")
add_executable(7_1 ${CHAPTER7_SRC}/7_1.c) 
add_executable(7_10 ${CHAPTER7_SRC}/7_10.c) 
add_executable(print_test ${CHAPTER7_SRC}/print_test.c) 

# chapter8
set(CHAPTER8_SRC "../chapter8/")
add_executable(8_1 ${CHAPTER8_SRC}/8_1.c) 
add_executable(8_2 ${CHAPTER8_SRC}/8_2.c) 
add_executable(8_3 ${CHAPTER8_SRC}/8_3.c) 
add_executable(8_6 ${CHAPTER8_SRC}/8_6.c) 
add_executable(8_7 ${CHAPTER8_SRC}/8_7.c) 
add_executable(print_args ${CHAPTER8_SRC}/print_args.c) 
add_executable(test_exec ${CHAPTER8_SRC}/test_exec.c) 

# chapter9
set(CHAPTER9_SRC "../chapter9/")
add_executable(gid_test ${CHAPTER9_SRC}/gid_test.c) 