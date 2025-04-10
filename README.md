# 基于IMX6ULL的应用驱动开发

本项目以IMX6ULL开发板为核心，结合多种硬件模块（如超声波测距、舵机控制、温湿度监测、红外遥控、数据存储、显示和图像采集等），开发了一个综合性的嵌入式应用系统。通过驱动开发和应用程序设计，实现了对各个模块的控制和数据交互，最终整合为一个功能丰富的APP。

- 为每个硬件模块编写Linux设备驱动，确保硬件与系统的通信。
- 使用Linux内核提供的框架（如I2C、SPI、PWM、GPIO等）实现驱动开发。
- 开发用户空间应用程序，通过系统调用与硬件驱动交互。
  - 超声波测距数据的读取与处理。
  - 舵机角度的精确控制。
  - 温湿度数据的采集与显示。
  - 红外遥控信号的解码与响应。
  - OLED屏幕的动态显示更新。
  - USB摄像头的图像采集与处理。
- 将各个模块的功能整合到一个统一的APP中，提供友好的用户界面。

## 环境搭建

创建项目目录

```bash
cd ~/
mkdir imx6ull
cd ./imx6ull/
```

设置git，如果未安装git,可以通过 sudo apt install git 来安装

```bash
git config --global user.email "邮箱"
git config --global user.name "用户名"
```

git下载交叉编译工具链和BSP(出现错误一般是网络问题，检查网络多试几次)

```BASH
git clone https://e.coding.net/codebug8/repo.git
mkdir -p 100ask_imx6ull-sdk && cd 100ask_imx6ull-sdk

../repo/repo init -u https://gitee.com/weidongshan/manifests.git -b linux-sdk -m imx6ull/100ask_imx6ull_linux4.9.88_release.xml --no-repo-verify

../repo/repo sync -j8

```

上述命令执行完毕的时候,ls一下，应该是这些文件

```bash
book@100ask:~/imx6ull/100ask_imx6ull-sdk$ ls
Buildroot_2020.02.x  Busybox_1.30.0  DevelopmentEnvConf  Linux-4.9.88  NoosProgramProject  ToolChain  Uboot-2017.03  Uboot-2018.03
```

编译构建本项目需要借助cmake工具来自动生成Makefile文件来make整个工程
使用apt安装的cmake版本比较旧，这里演示如何手工编译安装新版的cmake
需要自己手动到官网去下载一个较新的cmake源码压缩包

```BASH
cd ~/imx6ull
mkdir lib
cd ./lib
# 解压源码
tar -zxf cmake-3.30.2.tar.gz
cd ./cmake-3.30.2/

# 安装依赖库
sudo apt install openssl       
sudo apt install libssl-dev
# 配置源码，生成Makefile
./configure --prefix=$(pwd)/tmp 
# 编译
make -j8
# 安装
make install 
# 创建软链接
sudo ln -s $(pwd)/tmp/bin/cmake /usr/bin/cmake 
# 如果已经有旧版本的cmake了就需要先删除之前的软连接再重新创建新的软链接
sudo rm /usr/bin/cmake
```

2.配置环境变量

```BASH
vi ~/.bashrc
```

```BASH
export ARCH=arm
export CROSS_COMPILE=arm-buildroot-linux-gnueabihfexport- 
export PATH=$PATH:/home/book/imx6ull/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
```

修改完之后执行以下命令使其生效

```BASH
source ~/.bashrc
```

3.搭建源码编译环境

建一个放置源码的目录，后续项目的源码放在这里

```bash
cd ~/imx6ull
mkdir project
cd ./project
```

## 效果演示

