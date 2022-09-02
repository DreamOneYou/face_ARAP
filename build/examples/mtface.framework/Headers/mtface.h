#pragma once

/// @file mtface.h

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "export.h"
#ifdef __cplusplus
extern "C"
{
#endif

    // exported types
    typedef struct mtface_image_t mtface_image_t;
    typedef struct mtface_feature_t mtface_feature_t;
    typedef struct mtface_options_t mtface_options_t;
    typedef struct mtface_detector_t mtface_detector_t;
    typedef struct mtface_tracker_t mtface_tracker_t;
    typedef struct mtface_recognition_t mtface_recognition_t;

    /// @addtogroup image 图像
    /// @{

    /// @brief 创建图像引用句柄

    /// @param [in, out] in_out  图像句柄，如果为空则创建新的句柄
    /// @param [in] width       宽
    /// @param [in] height      高
    /// @param [in] format      图像格式
    /// @param [in] orientation exif 方向
    /// @param [in] y           y 数据，对于 常规RGBA Gray 数据为像素数据
    /// @param [in] ystride     y 通道 行字节数
    /// @param [in] u           u或者uv 通道          如果没有传 nullptr
    /// @param [in] ustride     u或者uv 通道 行字节数  如果没有传 0
    /// @param [in] v           v 通道 如果没有传 nullptr
    /// @param [in] vstride     v 通道 行字节数 如果没有传 0
    /// @param [out] error      输出错误的，如果为空，则部署出
    ///
    /// @return 图像句柄
    ///         - 图像句柄
    ///         - nullptr: 发生错误，请查阅error范围值
    MTFACE_API mtface_image_t* mtface_image_adapt(
        mtface_image_t *in_out,
        int width, int height,
        mtface_pixel_format_t format, int orientation,
        unsigned char *y, int ystride,
        unsigned char *u, int ustride,
        unsigned char *v, int vstride,
        char **error);

    MTFACE_API void mtface_image_destroy(mtface_image_t *ptr);
    /// @} image

    /// @addtogroup options 选项配置
    /// @{
    /// 选项配置句柄通过 @ref mtface_detector_get_options 和 @ref mtface_tracker_get_options 创建, 需要调用 destroy 进行销毁
    
    /// @brief 销毁配置
    /// @param [in] ptr                 需要销毁的配置
    MTFACE_API void mtface_options_destroy(mtface_options_t *ptr);
    
    /// @brief 设置配置选项
    /// @param [in] key  配置选项 键 由于变参函数的某些类型转换行为，key 写成 int32_t 类型，
    ///                  在使用过程中仍然应该传  mtface_option_key_t 的 enum 值
    /// @param [in] ...  配置选项 值， 参见 @ref option-key
    MTFACE_API void mtface_options_set(mtface_options_t *ptr, int32_t key, ...);
    /// @} options

    /// @addtogroup feature 人脸位置、属性、识别信息、人脸3D信息
    /// @{
    
    /// @brief 创建特征容器
    /// @param [out]   error             错误信息
    /// @return                          创建好的特征容器
    MTFACE_API mtface_feature_t* mtface_feature_create(char **error);

    /// @brief 销毁特征容器
    /// @param [in] ptr                  需要销毁的特征容器
    MTFACE_API void mtface_feature_destroy(mtface_feature_t *ptr);

    /// @addtogroup geometry 人脸位置信息
    /// @{

    /// @brief 设置人脸特征容器的大小, 当需要输入外部数据给人脸库算法的时候需要用到
    MTFACE_API mtface_status_t mtface_feature_set_face_size(mtface_feature_t *ptr, size_t size, char **error);

    /// @brief 获取特征容器中人脸的个数
    MTFACE_API size_t mtface_feature_get_face_size(mtface_feature_t *ptr);

    /// @brief 获取特征容器中某个人脸的id
    MTFACE_API int mtface_feature_get_face_id(mtface_feature_t *ptr, size_t index);

    /// @brief 设置特征容器中某个人脸的id
    MTFACE_API void mtface_feature_set_face_id(mtface_feature_t *ptr, size_t index, size_t id);

    /// @brief 获取特征容器中某个人脸的得分
    MTFACE_API float mtface_feature_get_face_score(mtface_feature_t *ptr, size_t index);

    /// @brief 设置特征容器中某个人脸的得分
    MTFACE_API void mtface_feature_set_face_score(mtface_feature_t *ptr, size_t index, float score);
    
    /// @brief 获取特征容器中某个人脸的人脸框
    MTFACE_API mtface_rect_t mtface_feature_get_face_box(mtface_feature_t *ptr, size_t index);

    /// @brief 设置特征容器中某个人脸的人脸框位置
    MTFACE_API void mtface_feature_set_face_box(mtface_feature_t *ptr, size_t index, mtface_rect_t box);

    /// 特征中不一定有人脸点, 视用户设置的检测器决定

    /// @brief 获取特征容器中某个人脸的人脸点
    MTFACE_API const float *mtface_feature_get_face_landmark(mtface_feature_t *ptr, size_t index, size_t *out_number_of_landmarks);
    
    /// @brief 设置人脸点
    MTFACE_API void mtface_feature_set_face_landmark(mtface_feature_t *ptr, size_t index, const float *landmarks, size_t num_of_landmarks);
    
    /// @brief 获取人脸点包含的类型
    MTFACE_API int mtface_feature_get_face_landmark_contain(const mtface_feature_t *ptr, size_t index);

    /// @brief 获取特征容器中某个人脸的耳朵点
    /// @param [in]  ptr                            特征容器
    /// @param [in]  index                          人脸索引
    /// @param [in]  right                          提取哪只耳多点，false左耳，true右耳
    /// @param [out] out_number_of_ear_landmarks    耳朵点个数
    MTFACE_API const float *mtface_feature_get_face_ear_landmark(mtface_feature_t *ptr, size_t index, bool right, size_t *out_number_of_ear_landmarks);


    /// @brief 获取特征容器中某个人脸的头顶点
    /// @param [in]  ptr                            特征容器
    /// @param [in]  index                          人脸索引
    /// @param [out] out_number_of_head_landmarks    头顶点个数
    MTFACE_API const float *mtface_feature_get_face_head_landmark(mtface_feature_t *ptr, size_t index, size_t *out_number_of_head_landmarks);

    /// @brief 获取特征容器中某个人脸的人脸点可见性
    MTFACE_API const float *mtface_feature_get_face_landmark_visibility(mtface_feature_t *ptr, size_t index, size_t *out_visiblity_length);

    /// @brief 获取特征容器中某个人脸的姿态欧拉角
    MTFACE_API void mtface_feature_get_face_pose_euler(mtface_feature_t *ptr, size_t index, float out_pitch_yaw_roll[3]);

    /// @brief 设置特征容器中某个人脸的姿态欧拉角
    MTFACE_API void mtface_feature_set_face_pose_euler(mtface_feature_t *ptr, size_t index, const float in_pitch_yaw_roll[3]);

    /// @brief 设置特征容器中某个人脸的3d形状系数
    MTFACE_API void mtface_feature_set_face_identity_coef(mtface_feature_t *ptr, size_t index, const float *identity_coef, size_t num_of_identity_coef);

    /// @brief 设置特征容器中某个人脸的3d表情系数
    MTFACE_API void mtface_feature_set_face_expression_coef(mtface_feature_t *ptr, size_t index, const float *expression_coef, size_t num_of_expression_coef);
    
    /// @brief 获取嘴唇 mask
    /// @param [in] ptr 特征句柄
    /// @param [in] index 序号
    /// @param [out] out_width mask 宽
    /// @param [out] out_height mask 高
    /// @param [out] out_row_major_matrix mask 变换到原图的 2x3 行优先仿射变换矩阵, 用户分配内存
    /// @retval 非空指针，mask数据首地址
    /// @retval nullptr, 人脸没有mask数据
    MTFACE_API const uint8_t* mtface_feature_get_mouth_mask(mtface_feature_t* ptr, size_t index, size_t* out_width, size_t* out_height, float out_row_major_matrix[6]);
    MTFACE_API const uint8_t* mtface_feature_get_face_parsing(mtface_feature_t* ptr, size_t index, size_t* out_width, size_t* out_height, float out_row_major_matrix[6]);

    /// @brief 按方向旋转人脸特征
    /// @param [in] input               输入人脸特征句柄
    /// @param [out] output             输入人脸句柄, 如果为 nullptr, 则创建新的句柄, 从返回值中返回, 需要调用 mtface_feature_destroy 销毁
    /// @param [in] width                输入 ptr 所在图像的宽
    /// @param [in] height               输入 ptr 所在图像的高
    /// @param [in] src_orientation     ptr 所在图像的方向
    /// @param [in] dst_orientation     ptr 旋转的目标方向
    /// @retval  output if output != nullptr, else new mtface_feature_t 
    /// @retval  nullptr error occurs
    MTFACE_API mtface_feature_t* mtface_feature_rotate(const mtface_feature_t *input, mtface_feature_t *output, size_t width, size_t height, int src_orientation, int dst_orientation);

    /// @brief 缩放人脸特征
    /// @param [in] input               输入人脸特征句柄
    /// @param [out] output             输入人脸句柄, 如果为 nullptr, 则创建新的句柄, 从返回值中返回, 需要调用 mtface_feature_destroy 销毁
    /// @param [in] src_width           ptr 所在图像的宽
    /// @param [in] src_height          ptr 所在图像的高
    /// @param [in] dst_width           目标图像的宽
    /// @param [in] dst_height          目标图像的高
    /// @retval  output if output != nullptr, else new mtface_feature_t 
    /// @retval  nullptr error occurs
    MTFACE_API mtface_feature_t* mtface_feature_resize(const mtface_feature_t *input, mtface_feature_t *output, size_t src_width, size_t src_height, size_t dst_width, size_t dst_height);

    /// @brief 获取视频检测器中检测器的检测耗时
    MTFACE_API const uint32_t* mtface_feature_get_detector_timer(mtface_feature_t *ptr, const mtface_type_t type, uint32_t* out_count, char **error);
    /// @}

    /// @addtogroup attribute 人脸属性
    /// @{

    /// @brief 获取特征容器中某个人脸的指定属性
    /// @param [in]  ptr                人脸特征容器
    /// @param [in]  index              人脸索引
    /// @param [in]  attr_key           需要获取的属性键名
    /// @param [out] out_attr           输出的属性结果（概率向量）,用户分配内存
    /// @param [in]  attr_num           获取的属性数量
    /// @param [in]  error              错误信息
    /// @return                         执行是否成功
    MTFACE_API mtface_status_t mtface_feature_get_face_attribute(mtface_feature_t *ptr, size_t index,
                                                                 const mtface_attr_key_t *attr_key, float *out_attr, size_t attr_num, char **error);
    /// @}

    /// @addtogroup recognition 人脸识别
    /// @{
    
    /// @brief 获取特征容器中某个人脸的人脸识别（FR）特征容器
    /// 
    /// @return 人脸识别句柄，需要调用 @ref mtface_recognition_destroy 进行释放
    MTFACE_API mtface_recognition_t* mtface_feature_get_face_recognition(mtface_feature_t *ptr, size_t index);

    /// @brief 创建人脸识别（FR）特征容器
    MTFACE_API mtface_recognition_t* mtface_recognition_create(int version, const float *code, size_t length, char **error);

    /// @brief 获取人脸识别（FR）特征容器中的特征向量
    MTFACE_API const float *mtface_recognition_get_code(mtface_recognition_t *ptr, size_t *length);

    /// @brief 获取人脸识别（FR）特征容器中的版本号
    MTFACE_API int mtface_recognition_get_version(mtface_recognition_t *ptr);

    /// @brief 获取人脸识别（FR）的特征聚类id
    MTFACE_API int mtface_recognition_get_cluster_id(mtface_recognition_t *ptr);

    /// @brief 销毁人脸识别（FR）特征容器
    MTFACE_API void mtface_recognition_destroy(mtface_recognition_t *ptr);
    /// @}

    /// @addtogroup 3dfa 人脸3D信息
    /// @{

    /// @brief 获取特征容器中某个人脸的3D Indentity 系数
    MTFACE_API const float* mtface_feature_get_face3d_id_coef(mtface_feature_t *ptr, size_t index, size_t *out_number_of_coef);

    /// @brief 获取特征容器中某个人脸的3D expression 系数
    MTFACE_API const float* mtface_feature_get_face3d_exp_coef(mtface_feature_t *ptr, size_t index, size_t *out_number_of_coef);

    /// @brief 获取特征容器中某个人脸的3D Mesh
    MTFACE_API const float* mtface_feature_get_face3d_mesh(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D MVP 矩阵
    MTFACE_API void mtface_feature_get_face3d_mvp(mtface_feature_t *ptr, size_t index, float mvp[16]);

    /// @brief 获取特征容器中某个人脸的3D 欧拉角
    MTFACE_API void mtface_feature_get_face3d_euler(mtface_feature_t *ptr, size_t index, float euler[3]);

    /// @brief 获取特征容器中某个人脸的3D 平移向量
    MTFACE_API void mtface_feature_get_face3d_translation(mtface_feature_t *ptr, size_t index, float translation[3]);

    /// @brief 获取特征容器中某个人脸的3D 旋转矩阵
    MTFACE_API void mtface_feature_get_face3d_rotation(mtface_feature_t *ptr, size_t index, float rotation[9]);
     
    /// @brief 获取特征容器中某个人脸的3D 尺度
    MTFACE_API float mtface_feature_get_face3d_scale(mtface_feature_t *ptr, size_t index);
    /// @brief 获取特征容器中某个人脸的3D 模型矩阵
    MTFACE_API void mtface_feature_get_face3d_model_matrix(mtface_feature_t *ptr, size_t index, float model_matrix[16]);

    /// @brief 获取特征容器中某个人脸的3D 视图矩阵
    MTFACE_API void mtface_feature_get_face3d_view_matrix(mtface_feature_t *ptr, size_t index, float view_matrix[16]);

    /// @brief 获取特征容器中某个人脸的3D 投影矩阵
    MTFACE_API void mtface_feature_get_face3d_projection(mtface_feature_t *ptr, size_t index, float projection[16]);

    /// @brief 获取相机的fov
    MTFACE_API float mtface_feature_get_face3d_camera_fov(mtface_feature_t *ptr, size_t index);

    /// @brief 获取相机的near
    MTFACE_API float mtface_feature_get_face3d_camera_near(mtface_feature_t *ptr, size_t index);

    /// @brief 获取相机的far
    MTFACE_API float mtface_feature_get_face3d_camera_far(mtface_feature_t *ptr, size_t index);

    /// @brief 获取特征容器中某个人脸的3D 无表情下的人脸,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_neutral_face(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 法向量,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_face_normal(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 切线向量,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_face_tangent(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 副法线向量,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_face_binormal(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 顶点法向量,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_vertice_normal(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 顶点切线向量,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_vertice_tangent(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 顶点副法线向量,  顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const float* mtface_feature_get_face3d_vertice_binormal(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 纹理坐标,   顶点排列顺序:U1 V1 ... Un Vn
    MTFACE_API const float* mtface_feature_get_face3d_texture_coordinates(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);

    /// @brief 获取特征容器中某个人脸的3D 三角网格, 顶点排列顺序:X1 Y1 Z1 ... Xn Yn Zn
    MTFACE_API const uint16_t * mtface_feature_get_face3d_triangle_list(mtface_feature_t *ptr, size_t index, size_t *out_number_of_vertices);
    /// @} 3dfa
    /// @}
    /// @addtogroup detector 静态图检测
    /// @{

    /// @brief 创建图片检测器
    /// @param [out] error               错误信息
    /// @return                   创建的图片检测器
    /// - mtface_tracker_t*             视频检测器句柄
    /// - nullptr                       发生错误, 请查看 error 返回的错误信息!
    MTFACE_API mtface_detector_t* mtface_detector_create(char **error);

    /// @brief 销毁图片检测器
    /// @param [in] ptr                  需要销毁的图片检测器
    MTFACE_API void mtface_detector_destroy(mtface_detector_t *ptr);

    /// @brief 获取当前图片检测器的配置
    /// @param [in] ptr         图像检测器
    /// @param [out] out_opts   如果非空则返回获取的配置
    /// @param [out] error      错误信息，如果正常返回nullptr
    ///
    /// @return
    ///   - if out_opts == nullptr, then 返回新的配置句柄
    ///   - if out_opts != nullptr, then 返回 out_opts, out_opts 被复用
    ///   - nullptr 发生错误， 请检查 error 返回的错误信息
    MTFACE_API mtface_options_t* mtface_detector_get_options(mtface_detector_t *ptr, mtface_options_t *out_opts, char **error);

    /// @brief 对图片检测器应用新的配置
    /// @param [in] ptr                     图像检测器
    /// @param [in] opts    输入配置项
    /// @param [out] error                  错误信息，如果正常返回nullptr
    ///
    /// @return
    ///   - opts
    ///   - nullptr 发生错误， 请检查 error 返回的错误信息
    MTFACE_API mtface_options_t* mtface_detector_set_options(mtface_detector_t *ptr, mtface_options_t *opts, char **error);

    /// @brief 给图片检测器挂载指定的子检测器模型
    /// @param [in]  ptr                 图片检测器
    /// @param [in]  type                子检测器的类型
    /// @param [in]  buffer              载入的模型
    /// @param [in]  size                模型的字节数
    /// @param [in]  enable              是否激活挂载的子检测器
    /// @param [out] error               错误信息
    /// return                          执行是否成功
    MTFACE_API mtface_status_t mtface_detector_append_model_buffer(mtface_detector_t *ptr,
                                                                   mtface_type_t type,
                                                                   const char *buffer,
                                                                   size_t size,
                                                                   bool enable,
                                                                   char **error);

    /// @brief 给图片检测器挂载指定的子检测器模型
    /// @param [in]  ptr                 图片检测器
    /// @param [in]  type                子检测器的类型
    /// @param [in]  filename            模型的路径
    /// @param [in]  enable              是否激活挂载的子检测器
    /// @param [out] error               错误信息
    /// return                          执行是否成功
    MTFACE_API mtface_status_t mtface_detector_append_model_file(mtface_detector_t *ptr,
                                                                 mtface_type_t type,
                                                                 const char *filename,
                                                                 bool enable,
                                                                 char **error);

    /// @brief 激活静态图检测器的指定子检测器模型
    /// @param [in] ptr 静态图检测器
    /// @param [in] type 子检测器类型
    /// @param [in] enable  开/关
    /// @param [out] error 错误信息
    /// 
    /// @return 
    ///     - mtface_status_ok 成功
    ///     - mtface_status_err 失败，请检查 error 返回信息
    MTFACE_API mtface_status_t mtface_detector_enable_model(mtface_detector_t *ptr, mtface_type_t type, bool enable, char **error);

    /// @brief 卸载图片检测器的指定子检测器模型
    MTFACE_API mtface_status_t mtface_detector_remove_model(mtface_detector_t *ptr, mtface_type_t type, char **error);

    /// @brief 执行图片检测
    ///
    /// @param  [in] ptr 图片检测器
    /// @param  [in] image 图像句柄
    /// @param  [in] out 结果 如果为空，内部将创建新的句柄输出
    /// @param  [out] error 错误信息
    ///
    /// @return
    ///    - if out == nullptr, then return new mtface_feature_t handle
    ///    - if out != nullptr, then return out
    ///    - nullptr error occur, check message return from error
    MTFACE_API mtface_feature_t* mtface_detector_detect(mtface_detector_t *ptr, const mtface_image_t *const image, mtface_feature_t *out, char **error);
    /// @}

    /// @addtogroup tracker 视频跟踪
    /// @{

    /// @brief 使用默认配置，创建视频跟踪检测器
    /// 
    MTFACE_API mtface_tracker_t* mtface_tracker_create(char **error);

    /// @brief 销毁视频检测器
    /// @param ptr          [in]        需要销毁的视频检测器
    MTFACE_API void mtface_tracker_destroy(mtface_tracker_t *ptr);

    /// @brief 获取视频检测器的当前配置
    /// @param [in] ptr         图像检测器
    /// @param [out] out_opts   如果非空则返回获取的配置
    /// @param [out] error      错误信息，如果正常返回nullptr
    ///
    /// @return
    ///   - if out_opts == nullptr, then 返回新的配置句柄
    ///   - if out_opts != nullptr, then 返回 out_opts, out_opts 被复用
    ///   - nullptr 发生错误， 请检查 error 返回的错误信息
    MTFACE_API mtface_options_t* mtface_tracker_get_options(mtface_tracker_t *ptr, mtface_options_t *out_opts, char **error);

    /// @brief 对视频检测器应用新的配置
    /// @param [in] ptr                     图像检测器
    /// @param [in] opts    输入配置项， 返回旧的配置项
    /// @param [out] error                  错误信息，如果正常返回nullptr
    ///
    /// @return
    ///   - opts
    ///   - nullptr 发生错误， 请检查 error 返回的错误信息
    MTFACE_API mtface_options_t* mtface_tracker_set_options(mtface_tracker_t *ptr, mtface_options_t *opts, char **error);

    /// @brief 给视频检测器挂载指定的子检测器模型
    /// @param [in]  ptr                视频检测器
    /// @param [in]  type               子检测器的类型
    /// @param [in]  buffer             载入的模型
    /// @param [in]  size               模型的字节数
    /// @param [in]  enable             是否激活挂载的子检测器
    /// @param [out] error              错误信息
    /// return                          执行是否成功
    MTFACE_API mtface_status_t mtface_tracker_append_model_buffer(mtface_tracker_t *ptr,
                                                                  mtface_type_t type,
                                                                  const char *model,
                                                                  size_t size, bool enable, char **error);

    /// @brief 给视频检测器挂载指定的子检测器模型
    /// @param [in]  ptr                 视频检测器
    /// @param [in]  type                子检测器的类型
    /// @param [in]  filename            模型的路径
    /// @param [in]  enable              是否激活挂载的子检测器
    /// @param [out] error               错误信息
    /// return                          执行是否成功
    MTFACE_API mtface_status_t mtface_tracker_append_model_file(mtface_tracker_t *ptr,
                                                                mtface_type_t type,
                                                                const char *filename, bool enable, char **error);

    /// @brief 激活视频检测器的指定子检测器模型
    /// @param [in] ptr 视频检测器
    /// @param [in] type 子检测器类型
    /// @param [in] enable  开/关
    /// @param [out] error 错误信息
    /// 
    /// @return 
    ///     - mtface_status_ok 成功
    ///     - mtface_status_err 失败，请检查 error 返回信息
    MTFACE_API mtface_status_t mtface_tracker_enable_model(mtface_tracker_t *ptr, mtface_type_t type, bool enable, char **error);

    /// @brief 卸载视频检测器的指定子检测器模型
    MTFACE_API mtface_status_t mtface_tracker_remove_model(mtface_tracker_t *ptr, mtface_type_t type, char **error);

    /// @brief 执行视频检测
    MTFACE_API mtface_feature_t* mtface_tracker_detect(mtface_tracker_t *ptr, const mtface_image_t *const image, mtface_feature_t *out, char **error);

    /// @brief 重置视频检测器状态
    MTFACE_API mtface_status_t mtface_tracker_reset(mtface_tracker_t *ptr, char **error);
    /// @}

    /// @addtogroup face-recognition 人脸对比、聚类、检索
    /// @{
    /// @brief 人脸识别（FR）特征比较
    MTFACE_API float mtface_recognition_compare(const mtface_recognition_t *lhs, const mtface_recognition_t *rhs, char **error);

    /// @brief 人脸识别（FR）特征聚类
    MTFACE_API mtface_status_t mtface_recognition_cluster(mtface_recognition_t* const* list, size_t size, char **error);

    /// @brief 人脸识别（FR) 根据最近邻类的赋类别
    MTFACE_API mtface_status_t mtface_recognition_assign_cluster(mtface_recognition_t* const* list,
                                                                size_t size,
                                                                mtface_recognition_t *target,
                                                                bool update_cluster_center,char **error);

    /// @brief 人脸识别（FR）特征搜索
    /// @param [in] list            人脸识别句柄列表
    /// @param [in] size            list 长度
    /// @param [in] target          搜索目标
    /// @param [in] similarity_threshold    相似阈值
    /// @param [in, out] in_out_index_list         相似的人脸在 list 中的索引 （外部分配内存)
    /// @param [in, out] in_out_index_score_list   相似人脸列表的相似性得分 （外部分配内存)
    /// @param [in, out] in_out_index_size         相似人脸个数 (外部分配内存, in时为预分配数组长度，out时为有效数组长度)
    /// @param [out] error 错误信息
    /// @return 
    ///    - mtface_status_ok 执行成功
    ///    - mtface_status_err 发生错误，请检查 error 返回值
    MTFACE_API mtface_status_t mtface_recognition_search(const mtface_recognition_t* const* list,
                                                         size_t size,
                                                         mtface_recognition_t *target,
                                                         float similarity_threshold,
                                                         size_t *in_out_index_list,
                                                         float *in_out_index_score_list,
                                                         size_t *in_out_index_size, char **error);

    /// @}

    /// @addtogroup miscellaneous 其他
    /// @{
    MTFACE_API const char *mtface_version();
    MTFACE_API void mtface_free(void *);
    /// @}

#ifdef __cplusplus
}
#endif

///
/// @example TestInterface.cpp
/// 
/// C 接口图像/视频检测代码示例 
///
/// @example TestInterfaceFR.cpp
/// 
/// C 接口人脸识别（FR）代码示例
/// 
/// @example TestInterfaceFacialFeature.cpp
/// 
/// C 接口局部五官代码示例
/// 
/// @example TestInterfaceVideo3D.cpp
/// 
/// C 接口3D人脸mesh代码示例
///
