#pragma once

#include "tensor.hpp"
namespace manis {

    /// @brief 像素(转换)类型
    typedef enum {
        PIXEL_CONVERT_SHIFT = 16,
        PIXEL_FORMAT_MASK = 0x0000ffff,
        PIXEL_CONVERT_MASK = 0xffff0000,
        
        // add by fxz 用第15位来判断结果是否需要反转，用于wwarp_pixels_resize
        PIXEL_FLIP_SHIFT = 15,

        PIXEL_RGB       = 1,
        PIXEL_BGR       = (1 << 1),
        PIXEL_GRAY      = (1 << 2),
        PIXEL_RGBA      = (1 << 3),
        PIXEL_BGRA      = (1 << 4),

        PIXEL_RGB2BGR   = PIXEL_RGB | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
        PIXEL_RGB2GRAY  = PIXEL_RGB | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
        
        PIXEL_BGR2RGB   = PIXEL_BGR | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
        PIXEL_BGR2GRAY  = PIXEL_BGR | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
        
        PIXEL_GRAY2RGB  = PIXEL_GRAY | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
        PIXEL_GRAY2BGR  = PIXEL_GRAY | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
        
        PIXEL_RGBA2RGB  = PIXEL_RGBA | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
        PIXEL_RGBA2BGR  = PIXEL_RGBA | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
        PIXEL_BGRA2BGR  = PIXEL_BGRA | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
        PIXEL_RGBA2GRAY = PIXEL_RGBA | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
        PIXEL_RGBA2BGRA = PIXEL_RGBA | (PIXEL_BGRA << PIXEL_CONVERT_SHIFT),

        PIXEL_RGBA2BGR_FLIP = PIXEL_RGBA2BGR | ( 1 << PIXEL_FLIP_SHIFT ),
        PIXEL_BGRA2BGR_FLIP = PIXEL_BGRA2BGR | ( 1 << PIXEL_FLIP_SHIFT )
    } PixelConvType;

    /// @brief 数据排列方式转换 TODO: 当前均只支持浮点的转换
    void MANIS_EXPORT NCHW2NHWC(const float* src, float* dst, int b, int h, int w, int c);
    void MANIS_EXPORT NHWC2NCHW(const float* src, float* dst, int b, int h, int w, int c);

    void MANIS_EXPORT NHWCToNCHWC4(const float* src, float* dst, size_t area, size_t depth);
    void MANIS_EXPORT NCHWC4ToNHWC(const float* src, float* dst, size_t area, size_t depth);

    void MANIS_EXPORT NCHWToNCHWC4(const float* src, float* dst, size_t area, size_t depth);
    void MANIS_EXPORT NCHWC4ToNCHW(const float* src, float* dst, size_t area, size_t depth);
    void MANIS_EXPORT NCHWC4ToNCHWINT8(const int8_t *src, int8_t *dst, size_t area, size_t depth);

    Tensor MANIS_EXPORT NCHW2NHWC(const Tensor& in);
    Tensor MANIS_EXPORT NHWC2NCHW(const Tensor& in);

    Tensor MANIS_EXPORT NCHWToNCHWC4(const Tensor& in);
    Tensor MANIS_EXPORT NCHWC4ToNCHW(const Tensor& in);

    /// @brief GL 外设时, 根据纹理信息创建 Tensor 的接口
    /// @param in: texture_coord_trans: 纹理坐标指向的内存地址将直接引用, 请注意保持和 tensor 的生命周期一致 
    Tensor MANIS_EXPORT FromTextures(int first_texture, int second_texture, float* texture_coord_trans);
    Tensor MANIS_EXPORT FromTextures(int texture, int width, int height, float alpha, float* texture_coord_trans);

    /// @brief GL 外设时, 从 Tensor 中获取纹理信息
    /// @param out: texture_coord_trans: 返回的纹理坐标指向的内存地址将由 tensor 持有并释放, 请注意保持和 tensor 的生命周期一致 
    bool MANIS_EXPORT ToTextures(const Tensor& tensor, int& first_texture, int& second_texture, float** texture_coord_trans);
    bool MANIS_EXPORT ToTextures(const Tensor& tensor, int& texture, int& width, int& height, float& alpha, float** texture_coord_trans);

    /// @brief WEBGL 外设时, 根据输入信息创建 Tensor 的接口
    /// @param in: type 当前仅支持: PIXEL_RGBA2BGR
    /// @param in: pixels 图片像素值
    /// @param in: width 图片宽
    /// @param in: height 图片高
    /// @param in: mean_vals 数据的 减因子
    /// @param in: norm_vals 数据的 乘比例因子
    /// @param in: matrix 相似变换矩阵 2 x 3
    Tensor MANIS_EXPORT CreateTensor(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride, const float* mean_vals, const float* norm_vals, const float* matrix = nullptr);

    /// @brief 对两个tensor进行点乘，仅支持数据为浮点型
    Tensor MANIS_EXPORT DotMultiply(const Tensor& t1, const Tensor& t2);
    
    namespace nchw {

        /// @brief 归一化操作: Y_channel[i] = (X_channel[i] - mean_vals[channel]) * norm_vals[channel]
        /// @param in: mean_vals/norm_vals: 地址将直接引用, 请注意保持和 tensor 的生命周期一致
        void MANIS_EXPORT SubstractMeanNormalize(Tensor& in, const float* mean_vals, const float* norm_vals);
        void MANIS_EXPORT SubstractMeanNormalize(Tensor& in, const uint8_t* mean_vals, const uint8_t* norm_vals);
        
        /// @brief 通道拼接, 拷贝两个tensor的内存块进行拼接组成新tensor
        Tensor MANIS_EXPORT ConcatChannel(const Tensor& t1, const Tensor& t2);
        /// @brief 通道分离, 拷贝tensor的指定通道内存块
        Tensor MANIS_EXPORT GetChannel(const Tensor& t, uint32_t channel);

        /// @brief 将图片像素转成一个 nchw 排列方式的 Tensor
        /// @param in: type 当前仅支持4种通道转换: PIXEL_RGBA2BGR_FLIP, PIXEL_BGRA2BGR_FLIP, PIXEL_RGBA2BGR, PIXEL_BGRA2BGR
        /// @param in: pixels 图片像素值
        /// @param in: width 图片宽
        /// @param in: height 图片高
        /// @param in: dat 数据类型:float/uint8_t 暂无需求,预留项,暂不实现
        Tensor MANIS_EXPORT FromPixels(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, DataType dat = DATA_TYPE_FLOAT);
        
        /// @brief 将图片像素进行缩放, 然后转成一个 nchw 排列方式的 Tensor
        /// @param in: type 当前仅支持4种通道转换: PIXEL_RGBA2BGR_FLIP, PIXEL_BGRA2BGR_FLIP, PIXEL_RGBA2BGR, PIXEL_BGRA2BGR
        /// @param in: pixels 图片像素值
        /// @param in: width 图片宽
        /// @param in: height 图片高
		/// @param in: stride 输入图片的步长, 一般情况下为 width * 4
        /// @param in: target_width 缩放后的图片宽
        /// @param in: target_height 缩放后的图片高
        /// @param in: dat 数据类型:float/uint8_t 暂无需求,预留项,暂不实现
        Tensor MANIS_EXPORT FromPixelsResize(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t target_width, uint32_t target_height, DataType dat = DATA_TYPE_FLOAT);
        Tensor MANIS_EXPORT FromPixelsResize(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t stride, uint32_t target_width, uint32_t target_height, DataType dat = DATA_TYPE_FLOAT);

        /// @brief 将图片像素进行缩放, 再进行一次相似变换, 然后转成一个 nchw 排列方式的 Tensor
        /// @param in: type 当前仅支持4种通道转换: PIXEL_RGBA2BGR_FLIP, PIXEL_BGRA2BGR_FLIP, PIXEL_RGBA2BGR, PIXEL_BGRA2BGR
        /// @param in: pixels 图片像素值
        /// @param in: width 图片宽
        /// @param in: height 图片高
        /// @param in: stride 输入图片的步长, 一般情况下为 width * 4
        /// @param in: target_width 缩放后的图片宽
        /// @param in: target_height 缩放后的图片高
        /// @param in: matrix 相似变换矩阵 2 x 3
        Tensor MANIS_EXPORT FromPixelsWarpResize(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, int stride, uint32_t target_width, uint32_t target_height, const float* matrix);    // 仅支持浮点
    
        // 将不同的颜色空间通道分离并转换成float(裸数据方式)
        void MANIS_EXPORT from_rgb2bgr(float *bgr, const unsigned char *rgb, int width, int height);
        void MANIS_EXPORT from_rgb2gray(float *gray, const unsigned char *rgb, int width, int height);
        void MANIS_EXPORT from_bgr2gray(float *gray, const unsigned char *bgr, int width, int height);
        void MANIS_EXPORT from_gray2rgb(float *rgb, const unsigned char *gray, int width, int height);
        void MANIS_EXPORT from_rgba2rgb(float *rgb, const unsigned char *rgba, int width, int height);
        void MANIS_EXPORT from_rgba2bgr(float *bgr, const unsigned char *rgba, int width, int height);
        void MANIS_EXPORT from_rgba2gray(float *gray, const unsigned char *rgba, int width, int height);
        void MANIS_EXPORT from_rgba2bgra(float *bgra, const unsigned char *rgba, int width, int height);
        void MANIS_EXPORT from_rgb(float *out, const unsigned char *rgb, int width, int height);
        void MANIS_EXPORT from_gray(float *out, const unsigned char *gray, int width, int height);
        void MANIS_EXPORT from_rgba(float *out, const unsigned char *rgba, int width, int height);
        void MANIS_EXPORT warp_rgba2bgr(float *out, const unsigned char *pixels, int w, int h, int stride,
                                    int target_width, int target_height, const float *matrix);
        void MANIS_EXPORT warp_bgra2bgr(float *out, const unsigned char *pixels, int w, int h, int stride,
                                    int target_width, int target_height, const float *matrix);
        void MANIS_EXPORT warp_rgba2bgr_flip(float *out, const unsigned char *pixels, int w, int h, int stride,
                            int target_width, int target_height, const float *matrix);
        void MANIS_EXPORT warp_bgra2bgr_flip(float *out, const unsigned char *pixels, int w, int h, int stride,
                                    int target_width, int target_height, const float *matrix);
        
        void MANIS_EXPORT substract_mean_normalize(float* data, int c, int w, int h, const float *mean_vals, const float *norm_vals);
    } //namespace nchw

    // namespace nhwc {

    //     /// @brief 将图片像素转成一个 nhwc 排列方式的 Tensor
    //     Tensor MANIS_EXPORT FromPixels(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, DataType dat = DATA_TYPE_FLOAT);
    //     Tensor MANIS_EXPORT FromPixelsResize(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t target_width, uint32_t target_height, DataType dat = DATA_TYPE_FLOAT);
    //     Tensor MANIS_EXPORT FromPixelsWarpResize(PixelConvType type, const uint8_t* pixels, uint32_t width, uint32_t height, int stride, uint32_t target_width, uint32_t target_height, const float* matrix);    // 仅支持浮点
    
    // } //namespace nchw

} //namespace manis