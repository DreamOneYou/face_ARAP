#pragma once

/// @file utility.h

#include <stdlib.h>

#include "export.h"
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// @addgroup utility 工具函数
/// @{

/// @brief 通过人脸点裁图，只要人脸点个数与人脸库输出的一致，内部自动转正图像
/// @param [in] src         输入图，只支持常规图像
/// @param [in] src_width   输入宽
/// @param [in] src_height  输入高
/// @param [in] src_channel 图像通道数
/// @param [in] src_stride  输入stride
/// @param [out] dst         输出地址， 用户分配内存
/// @param [in] landmarks   人脸点，按照xyxy排列，且连续
/// @param [in] warp_landmarks 人脸点变换到裁图的坐标
/// @param [in] matrix        2x3变换矩阵，用来变换人脸点到裁图上
/// @param [in] num_of_landmarks 人脸点个数
/// @param [in] padding_left  向左扩边比例，相对与人脸的宽度
/// @param [in] padding_top   向上扩边比例，相对于人脸的高度
/// @param [in] padding_right 向右扩边比例，相对于人脸的宽度
/// @param [in] padding_bottom 向下扩边比例，相对于人脸点的高度
MTFACE_API void mtface_warp_by_landmark(
    const unsigned char* src, int src_width, int src_height, int src_channel, int src_stride,
    unsigned char* dst, int dst_width, int dst_height, int dst_stride,
    const float* landmarks, size_t num_of_landmarks, float matrix[2 * 3],
    float padding_left, float padding_top, float padding_right, float padding_bottom
);

/// @brief 通过外包框裁图（不转正），参数与 mtface_warp_by_landmarks 类似
MTFACE_API void mtface_warp_by_box(
    const unsigned char* src, int src_width, int src_height, int src_channel, int src_stride,
    unsigned char* dst, int dst_width, int dst_height, int dst_stride,
    mtface_rect_t bbox, float matrix[2 * 3],
    float padding_left, float padding_top, float padding_right, float padding_bottom
);

MTFACE_API void mtface_transform_landmark(const float* input, float* output, size_t num_of_landmarks, const float matrix[2*3]);

// @brief 校验模型数据是否完整
MTFACE_API void mtface_model_check(const char* data, const size_t length, size_t* out);
/// @}

///
/// @example TestWarp.cpp
/// 
/// 人脸裁图示例
///


#ifdef __cplusplus
}
#endif