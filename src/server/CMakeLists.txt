# 指定源文件
aux_source_directory(./db/ DB_LIST)
aux_source_directory(./model/ MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(. SRC_LIST)
# 创建可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})   
# 查找并链接 cc_muduo 库
find_library(CC_MUDUO_LIBRARY NAMES cc_muduo PATHS /usr/lib)
if(CC_MUDUO_LIBRARY)
    target_link_libraries(ChatServer PRIVATE ${CC_MUDUO_LIBRARY} mysqlclient pthread hiredis)
else()
    message(FATAL_ERROR "cc_muduo library not found")
endif()
