cmake_minimum_required(VERSION 3.0)
project(APUE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Werror")

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
add_executable(9_2 ${CHAPTER9_SRC}/9_2.c) 

# chapter10
set(CHAPTER10_SRC "../chapter10/")
add_executable(10_1 ${CHAPTER10_SRC}/10_1.c) 
add_executable(10_2 ${CHAPTER10_SRC}/10_2.c) 
add_executable(10_3 ${CHAPTER10_SRC}/10_3.c) 
add_executable(10_5 ${CHAPTER10_SRC}/10_5.c) 
add_executable(10_6 ${CHAPTER10_SRC}/10_6.c) 
add_executable(10_7 ${CHAPTER10_SRC}/10_7.c) 
add_executable(10_9 ${CHAPTER10_SRC}/10_9.c) 
add_executable(10_10 ${CHAPTER10_SRC}/10_10.c) 
add_executable(10_11 ${CHAPTER10_SRC}/10_11.c) 
add_executable(10_12 ${CHAPTER10_SRC}/10_12.c) 
add_executable(timer_test ${CHAPTER10_SRC}/timer_test.c) 
target_link_libraries(timer_test rt)
add_executable(timer_test2 ${CHAPTER10_SRC}/timer_test2.c) 

# chapter11
set(CHAPTER11_SRC "../chapter11/")
add_executable(11_1 ${CHAPTER11_SRC}/11_1.c) 
target_link_libraries(11_1 pthread)
add_executable(11_2 ${CHAPTER11_SRC}/11_2.c) 
target_link_libraries(11_2 pthread)
add_executable(11_3 ${CHAPTER11_SRC}/11_3.c) 
target_link_libraries(11_3 pthread)
add_executable(11_5 ${CHAPTER11_SRC}/11_5.c) 
target_link_libraries(11_5 pthread)

# chapter12
set(CHAPTER12_SRC "../chapter12/")
add_executable(12_1 ${CHAPTER12_SRC}/12_1.c) 
target_link_libraries(12_1 pthread)
add_executable(12_2 ${CHAPTER12_SRC}/12_2.c) 
target_link_libraries(12_2 pthread)
add_executable(12_6 ${CHAPTER12_SRC}/12_6.c) 
target_link_libraries(12_6 pthread)

# chapter13
set(CHAPTER13_SRC "../chapter13/")
add_executable(13_4 ${CHAPTER13_SRC}/13_4.c) 

# chapter14
set(CHAPTER14_SRC "../chapter14/")
add_executable(14_1 ${CHAPTER14_SRC}/14_1.c) 
add_executable(14_5 ${CHAPTER14_SRC}/14_5.c) 
add_executable(14_6 ${CHAPTER14_SRC}/14_6.c) 
add_executable(14_7 ${CHAPTER14_SRC}/14_7.c) 
add_executable(14_8 ${CHAPTER14_SRC}/14_8.c) 
target_link_libraries(14_8 rt)
add_executable(14_10 ${CHAPTER14_SRC}/14_10.c) 

# chapter15
set(CHAPTER15_SRC "../chapter15/")
add_executable(15_3 ${CHAPTER15_SRC}/15_3.c) 
add_executable(15_7 ${CHAPTER15_SRC}/15_7.c) 
add_executable(15_8 ${CHAPTER15_SRC}/15_8.c) 
add_executable(15_8_2 ${CHAPTER15_SRC}/15_8_2.c) 
add_executable(15_10 ${CHAPTER15_SRC}/15_10.c) 
add_executable(15_12 ${CHAPTER15_SRC}/15_12.c) 
add_executable(15_15 ${CHAPTER15_SRC}/15_15.c) 
add_executable(15_16 ${CHAPTER15_SRC}/15_16.c) 
add_executable(15_17 ${CHAPTER15_SRC}/15_17.c) 
add_executable(15_18 ${CHAPTER15_SRC}/15_18.c) 
target_link_libraries(15_18 pthread)

# chapter16
set(CHAPTER16_SRC "../chapter16/")
add_executable(16_1 ${CHAPTER16_SRC}/16_1.c) 
add_executable(16_2 ${CHAPTER16_SRC}/16_2.c) 
add_executable(16_test ${CHAPTER16_SRC}/16_test.c) 
add_executable(16_3_client ${CHAPTER16_SRC}/16_3_client.c) 
add_executable(16_3_server ${CHAPTER16_SRC}/16_3_server.c) 

# chapter17
set(CHAPTER17_SRC "../chapter17/")
add_executable(17_1_sender ${CHAPTER17_SRC}/17_1_sender.c) 
add_executable(17_1_receiver ${CHAPTER17_SRC}/17_1_receiver.c) 
target_link_libraries(17_1_receiver pthread)
add_executable(send_recv_msg_test ${CHAPTER17_SRC}/send_recv_msg_test.c) 
add_executable(17_2 ${CHAPTER17_SRC}/17_2.c ${CHAPTER17_SRC}/common.h ${CHAPTER17_SRC}/common.c) 
add_executable(17_4 ${CHAPTER17_SRC}/17_4.c) 
add_executable(17_7 ${CHAPTER17_SRC}/17_7.c ${CHAPTER17_SRC}/common.h ${CHAPTER17_SRC}/common.c) 

# chapter18
set(CHAPTER18_SRC "../chapter18/")
add_executable(18_1 ${CHAPTER18_SRC}/18_1.c) 

# chapter19
set(CHAPTER19_SRC "../chapter19/")
add_executable(pty_test ${CHAPTER19_SRC}/pty_test.c) 
add_executable(pty ${CHAPTER19_SRC}/pty.c) 
add_executable(19_9 ${CHAPTER19_SRC}/19_9.c) 
add_executable(19_9_callee ${CHAPTER19_SRC}/19_9_callee.c) 

