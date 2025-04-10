# 指定工程最低需要使用哪一个版本的cmake工具进行配置编译
cmake_minimum_required(VERSION 3.12)

# 指定工程名字
project(xf_project)

# 指定c编译器的绝对路径 CMAKE_C_COMPILER是一个变量
set(CMAKE_C_COMPILER "/home/book/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc" CACHE STRING "Updated variable" FORCE)

# 指定自己链接的库
SET(GCC_USER_LINK_FLAGS    "-lm -lfreetype -lpthread -ljpeg") # 自己用到的库链接

# 指定inlcude目录
include_directories(include)
include_directories(ui)

# 去指定目录下执行该目录下的cmake配置文件
# 其实就是去lvgl目录下把.c编译成.o文件 连接成动态库
add_subdirectory(ui/lvgl)
add_subdirectory(ui/lv_drivers)

# 设置PAGE变量，然后这个变量PAGE会等于 "page/*.c"
file(GLOB_RECURSE PAGE "page/*.c")
file(GLOB_RECURSE TEST_DRV "test/*.c")

# 指定编译哪些源文件
add_executable(${PROJECT_NAME} "sys/main.c" "ui/ui.c" ${PAGE} ${TEST_DRV})

# 指定可执行文件在链接是链接的哪些库 GCC_USER_LINK_FLAGS是刚才设置的变量
target_link_libraries(${PROJECT_NAME} PRIVATE ${GCC_USER_LINK_FLAGS})
# 因此 把lvgl生成的的动态库链接到本项目的可执行文件中去
target_link_libraries(${PROJECT_NAME} PRIVATE lvgl lvgl::examples lvgl::drivers)
