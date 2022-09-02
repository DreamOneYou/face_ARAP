#pragma once

#include <stdint.h>

/// @file types.h

#ifdef __cplusplus
extern "C"
{
#endif
/* enums */

#if __cplusplus >= 201100
#define MTFACE_ENUM(_type) enum _type : int32_t
#else
#define MTFACE_ENUM(_type)      \
  typedef int32_t _type;        \
  enum
#endif

MTFACE_ENUM(mtface_status_t)
{
    mtface_status_err = -1,                     ///< 出错了
    mtface_status_ok = 0,                       ///< 成功
};

MTFACE_ENUM(mtface_pixel_format_t)
{
    mtface_pixel_format_gray,
    mtface_pixel_format_rgba,
    mtface_pixel_format_nv12,
    mtface_pixel_format_nv21,
    mtface_pixel_format_i420,
    mtface_pixel_format_bgra,
    mtface_pixel_format_bgr,
    mtface_pixel_format_rgb
};

MTFACE_ENUM(mtface_filter_t){
    mtface_filter_balance = 0,     ///< 均衡模式，与原接口关闭画质效果一致
    mtface_filter_performance = 1, ///< 性能模式，保证图像质量，与原接口开启画质效果一致
    mtface_filter_smooth = 2,      ///< 流畅模式，保证出脸速度
    mtface_filter_double_check = 3,           ///< 重检测模式，画质ok时修正属性结果
    mtface_filter_triple_check = 4,           ///< 三帧检测模式，画质ok时修正属性结果
    mtface_filter_return_quality = 5,         ///< 返回画质值模式，不进行过滤
    
    mtface_filter_none = -1,       ///< 关闭模式，FR的过滤条件也关闭
};

MTFACE_ENUM(mtface_attr_key_t)
{
    mtface_attr_age,                            ///< 年龄  0-79 分，国内使用FAST模型，海外使用NORMAL模型
    mtface_attr_gender_male,                    ///< 性别  男 
    mtface_attr_gender_female,                  ///< 性别  女
    mtface_attr_race_white,                     ///< 种族  白
    mtface_attr_race_yellow,                    ///< 种族  黄（东亚）
    mtface_attr_race_black,                     ///< 种族  黑
    mtface_attr_race_india_north,               ///< 种族  印度北（偏白）
    mtface_attr_race_india_south,               ///< 种族  印度南（偏黑）
    mtface_attr_race_southest_asia,             ///< 种族  东南亚
    mtface_attr_beauty,                         ///< 颜值  40-100 分

    mtface_attr_emotion_sad,                    ///< 情绪  伤心 0-100分       
    mtface_attr_emotion_neutral,                ///< 情绪  平静 0-100分
    mtface_attr_emotion_smile,                  ///< 情绪  微笑 0-100分
    mtface_attr_emotion_laugh,                  ///< 情绪  大笑 0-100分
    mtface_attr_emotion_surprise,               ///< 情绪  惊讶 0-100分
    mtface_attr_emotion_fear,                   ///< 情绪  恐惧 0-100分
    mtface_attr_emotion_angry,                  ///< 情绪  愤怒 0-100分
    mtface_attr_emotion_disgust,                ///< 情绪  厌恶 0-100分

    mtface_attr_glasses_type_none,              ///< 眼镜  类型 无眼镜
    mtface_attr_glasses_type_normal,            ///< 眼镜  类型 普通眼镜
    mtface_attr_glasses_type_sunglasses,        ///< 眼镜  类型 墨镜

    mtface_attr_glasses_shape_square,           ///< 眼镜  形状 方形
    mtface_attr_glasses_shape_circle,           ///< 眼镜  形状 圆形
    mtface_attr_glasses_shape_other,            ///< 眼镜  形状 其他形状

    mtface_attr_glasses_frame_full,             ///< 眼镜  框类型 全框
    mtface_attr_glasses_frame_half,             ///< 眼镜  框类型 半框
    mtface_attr_glasses_frame_none,             ///< 眼镜  框类型 全框

    mtface_attr_glasses_thickness_thick,        ///< 眼镜  粗细 粗
    mtface_attr_glasses_thickness_thin,         ///< 眼镜  粗细 细

    mtface_attr_glasses_size_large,             ///< 眼镜  大小 大
    mtface_attr_glasses_size_small,             ///< 眼镜  大小 小

    mtface_attr_mustache_type_none,             ///< 胡子  类型 无胡子
    mtface_attr_mustache_type_stubble,          ///< 胡子  类型 胡渣
    mtface_attr_mustache_type_mustache,         ///< 胡子  类型 普通胡子

    mtface_attr_mustache_length_short,          ///< 胡子  长度 短
    mtface_attr_mustache_length_medium,         ///< 胡子  长度 中
    mtface_attr_mustache_length_long,           ///< 胡子  长度 长

    mtface_attr_mustache_shape_half_goatee,     ///< 胡子  形状 半山羊胡
    mtface_attr_mustache_shape_full_goatee,     ///< 胡子  形状 全山羊胡
    mtface_attr_mustache_shape_pencil_thin,     ///< 胡子  形状 八字胡
    mtface_attr_mustache_shape_full_beard,      ///< 胡子  形状 全胡须
    mtface_attr_mustache_shape_whisker,         ///< 胡子  形状 络腮胡
    
    mtface_attr_mustache_thickness_thin,        ///< 胡子  密度 稀松
    mtface_attr_mustache_thickness_thick,       ///< 胡子  密度 浓密

    mtface_attr_eyelid_left_single,             ///< 眼皮  左眼 单眼皮 （图像左右）
    mtface_attr_eyelid_left_double,             ///< 眼皮  左眼 双眼皮  
    mtface_attr_eyelid_left_double_inside,      ///< 眼皮  左眼  内双

    mtface_attr_eyelid_right_single,            ///< 眼皮  右眼  单眼皮
    mtface_attr_eyelid_right_double,            ///< 眼皮  右眼  双眼皮
    mtface_attr_eyelid_right_double_inside,     ///< 眼皮  右眼  内双

    mtface_attr_face_quality_bright,            ///< 人脸质量  亮度  0-100分 越大越亮
    mtface_attr_face_quality_blur,              ///< 人脸质量  模糊度  0-100分 越大越清晰
    mtface_attr_face_quality_compression,       ///< 人脸质量  压缩质量 0-100分 越大质量越好
    
    mtface_attr_age_child,                      ///< 年龄    儿童     <= 8岁
    mtface_attr_age_nonchild,                   ///< 年龄   非儿童     >8岁

    mtface_attr_occlusion,                      ///< 人脸遮挡  有遮挡概率 0～1

};

/// @addtogroup ability  人脸库功能列表
/// @{
MTFACE_ENUM(mtface_type_t){
    mtface_type_fd,                         ///< 人脸框检测
    mtface_type_face_box,                   ///< 人脸框跟踪
    mtface_type_fa_light,                   ///< 人脸点 快速版（提供基础人脸点，速度极快）
    mtface_type_fa_medium,                  ///< 人脸点 标准版 (提供人脸点、辅助耳朵点、人脸点可见性、姿态、实时表情等)，功能丰富速度快
    mtface_type_fa_heavy,                   ///< 人脸点 精准版 (供非实时场景下使用，追求极致的人脸点定位精度)
    mtface_type_refine_mouth,               ///< 进一步优化嘴唇点精度和输出嘴唇mask，配合全脸 fa_xxx 使用
    mtface_type_refine_eyes,                ///< 进一步优化眼睛点精度、配合全脸 fa_xxx 使用             
    mtface_type_ear,                        ///< 耳朵点需要配合 mtface_type_fa_medium（跟踪）或者 mtface_type_fa_heavy（拍后）使用
    mtface_type_age,                        ///< 年龄
    mtface_type_gender,                     ///< 性别
    mtface_type_race,                       ///< 种族
    mtface_type_beauty,                     ///< 颜值
    mtface_type_emotion,                    ///< 表情
    mtface_type_glasses,                    ///< 眼镜
    mtface_type_mustache,                   ///< 胡子
    mtface_type_eyelid,                     ///< 眼皮
    mtface_type_fr,                         ///< 人脸识别
    mtface_type_3d,                         ///< 3DMM fitting 回归3d系数
    mtface_type_face_quality,               ///< 人脸质量
    mtface_type_facialfeature_detection,    ///< 局部五官检测
    mtface_type_parsing,                    ///< 全脸分割
    mtface_type_dl3d,                       ///< DL回归3d 系数
    mtface_type_head,                       ///< 头顶额头点
    mtface_type_facialfeature_eye,          ///< 局部眼睛点
    mtface_type_facialfeature_mouth,        ///< 局部嘴巴点
    mtface_type_occlusion,                  ///< 人脸遮挡识别，超30%判定为遮挡
};
/// @}

/// @addtogroup feature
/// @{
/// mtface_contain_t 表示人脸点包含的类型
/// 对于完整的全脸，人脸包含所有类型， 即等于 =  mtface_contain_leye | mtface_contain_reye | mtface_contain_mouth | mtface_contain_other
/// 对于只有五官的人脸，包含 mtface_contain_leye， mtface_contain_reye， mtface_contain_mouth 的组合
/// 通过 mtface_feature_get_face_landmark_contain 可获取人脸点包含的类型位图

MTFACE_ENUM(mtface_contain_t){
    mtface_contain_leye =  1 << 0,      ///< 局部人脸左眼
    mtface_contain_reye =  1 << 1,      ///< 局部人脸右眼
    mtface_contain_mouth = 1 << 2,     ///< 局部人脸嘴巴
    mtface_contain_other = 1 << 3,     ///< 除上述五官之外的人脸点
};
///@}

/// @addtogroup options
/// @{
/// mtface_option_enable_async_fd 用于设置是否进行异步人脸检测，
/// mtface_option_enable_async_fd == true，采用异步FD检测，没有卡顿现象，但可能检测出人脸的时机存在延迟；通过设置mtface_option_fast_interval>0, 可在无人脸情况下增加同步FD，可更快检出图像中较大的人脸，建议相关参数设置为：mtface_option_fast_interval = 1，mtface_option_fast_minimal_face = 0.25；
/// mtface_option_enable_async_fd == false，采用同步FD检测，存在卡顿现象，可及时检测出图像中包含的人脸。
/// @addtogroup option-key 配置可选项
/// @{
MTFACE_ENUM(mtface_option_key_t)
{
    mtface_option_minimal_face,             ///< float: FD最小人脸的尺寸，相对于图像短边的比例         [图像：0.073；视频0.084]
    mtface_option_fd_interval,              ///< size_t: FD检测间隔                             [30]
    mtface_option_smooth_weight,            ///< float: 平滑系数范围 0~1，越大越平滑，迟滞越严重，    [0.8]
    mtface_option_score_threshold,          ///< float: 人脸点跟踪和检测的杀框阈值  [0.5]
    mtface_option_face_limit,               ///< size_t: 最大人脸数 [图像10; 视频5]
    mtface_option_enable_filter,             ///< mtface_filter_t: 启用不同模式的过滤规则 [mtface_filter_balance]
    mtface_option_enable_visibility,        ///< bool: 是否预测人脸点可见性 [false]
    mtface_option_enable_pose_estimation,   ///< bool: 是否进行姿态估计, 当开启依赖姿态的相关功能自动开启 [false]
    mtface_option_enable_mask,              ///< bool: 是否进行mask预测, 当前支持嘴唇mask预测 [false]
    mtface_option_enable_mesh_generation,   ///< bool: 是否进行3D mesh生成，只有开启3D模型才有效 [false]
    mtface_option_enable_emotion,           ///< bool: 是否进行实时表情检测， 只在video模式下开启才有效 [false]
    mtface_option_enable_async_fd,          ///< bool: 是否进行异步人脸检测，异步减少卡顿，出人脸慢一些， [true]
    mtface_option_fr_interval,              ///< size_t: FR检测间隔                             [默认只检测一次]
    mtface_option_swap_width_height,        ///< bool: 针对微信小程序宽高对调的内部fix, 其它平台请忽略，将来会删除 [false]
    mtface_option_enable_async_fr,          ///< bool: 是否进行异步人脸识别FR检测，异步减少卡顿，人脸识别特征输出慢一些 [true]
    mtface_option_fast_minimal_face,        ///< float: 异步检测时的同步FD检测时最小人脸的尺寸，相对于图像短边的比例，仅在异步FD检测开启时生效，用于快速检出人脸[视频：0.25]
    mtface_option_fast_fd_interval,            ///< size_t: 异步检测时的同步FD检测间隔，仅在异步FD检测开启时生效，用于快速检出人脸             [0]
    mtface_option_enable_parsing_smooth,    ///< bool: 是否开启全脸分割平滑
    mtface_option_enable_timer,             ///< bool: 是否开启检测器耗时统计
    mtface_option_enable_face_normal_generation, ///< bool: 是否进行face normal生成，只有开启3D模型才有效[false]
    mtface_option_enable_vertice_normal_generation, ///< bool: 是否进行vertice normal生成，只有开启3D模型才有效[false]
    mtface_option_enable_face_tangent_generation, ///<bool: 是否进行face tangent生成，只有开启3D模型才有效[false]
    mtface_option_enable_vertice_tangent_generation,  ///<bool: 是否进行vertice tangent（包括vertice binormal）生成，只有开启3D模型才有效[false]
    mtface_option_enable_mouth_and_parsing,        ///< bool; 是否同时输出人脸mask和嘴唇mask
    mtface_option_enable_half_parsing,        ///< bool: 是否输出一半尺寸的全脸分割结果 [false]
    mtface_option_enable_coef_generation,    ///<bool: 是否输出3D模型的id、exp参数  [false]
};
/// @}
/// @}

typedef struct mtface_rect_t
{
    float x, y, width, height;
} mtface_rect_t;

#ifdef __cplusplus
};
#endif
