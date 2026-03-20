# As Rigid As Possible Surface deformation

An implementation of ARAP deformation using libigl. 
实现鼻子和人脸自动修复

# 拉取第三方库依赖
git submodule update --init
## Compile

libigl should be installed in the parent folder of where this repository has been cloned. 

Compile this project using the standard cmake routine:

    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .

This should find and build the dependencies and create an `face-profiling_bin` binary.
##
face-profiling_bin 文件路径： build/examples/face-profiling_bin
## Run
    
From within the `build` directory issue one of those commands, or any variant thereof :

    cd examples
    <!-- ./face-profiling_bin /Users/wpx/Documents/data/nose_validata/inputs_v1/obj_50/IMG_6265.JPG -->
    ./face-profiling_bin /Users/wpx/Documents/data/整理后2d_3d_人脸属性_人脸点/inputs/image/小鼻子.jpeg /Users/wpx/Documents/data/整理后2d_3d_人脸属性_人脸点/inputs/jsons/小鼻子.jpeg.json
    
## 效果
![before](https://github.com/DreamOneYou/face_ARAP/blob/main/data/testDATA/before/%E4%B8%AD%E8%83%A1.jpg)
![after](https://github.com/DreamOneYou/face_ARAP/blob/main/data/testDATA/after/%E4%B8%AD%E8%83%A1.jpg)
