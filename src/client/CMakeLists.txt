aux_source_directory(. SRC_LIST)
# 创建可执行文件
add_executable(ChatClient ${SRC_LIST})   

target_link_libraries(ChatClient pthread)
