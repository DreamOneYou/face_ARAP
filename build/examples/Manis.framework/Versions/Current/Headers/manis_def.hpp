 #pragma once

#include <cstdint>
#include <cstddef>
#include "manis_export.hpp"

namespace manis {

    /// @brief 功能扩展项配置
    typedef enum {
        /// @brief NET_OPTION_CFG: 用于创建(通用)网络对象 CreateNet
        NET_OPTION_CFG_START = 0,
        NET_OPTION_CFG_ENABLE_MULTIPLE_THREAD,  // 弃用：配置在OMP环境下是否开启多线程
        NET_OPTION_CFG_DEVICE_TYPE,             // 配置外设类型 CPU/GL/HEXAGON
        NET_OPTION_CFG_LAYOUT_TYPE,             // 配置算子数据排列方式, 该配置当前仅支持浮点算子的切换 NCHW/NCHWC4
        NET_OPTION_CFG_DATA_TYPE,               // TODO 配置算子数据类型, 该配置主要用于定点模型在某些场景下走浮点算子
        NET_OPTION_CFG_MEMORY_MODE,             // TODO 内存模式 0:默认模式:性能优先 1:内存优先
        NET_OPTION_CFG_FLOAT_PRECISION,         // 配置浮点计算的精度：0:默认精度：fp32 1:fp16优先(若硬件不支持则fp32) 2:bf16
        NET_OPTION_CFG_OPTIMIZE_TYPE,           // 配置优化类型：0: 不做任何优化 1:cpu 在线选择算子的最优实现 
                                                // 2:保存在线生成的外设模型（减少后续再次使用的模型转换时间）
        NET_OPTION_CFG_OPTIMIZATION_RESULT_PATH,  // 配置优化结果保存路径，配合优化类型使用

        /// @brief NET_CONFIG_CFG : 用于配置(通用)网络对象 Config
        NET_CONFIG_CFG_START = 200,

        /// @brief EXE_OPTION_CFG : 用于创建(通用)执行器 CreateExecutor
        EXE_OPTION_CFG_START = 400,
        EXE_OPTION_CFG_NUM_THREADS,             // 配置多线程: 0:自动配置线程数，n(n≠0):设置线程数为n

        /// @brief EXE_EXPAND_CFG : 用于扩展(通用)执行器 Expand
        EXE_EXPAND_CFG_START = 600,
        EXE_EXPAND_CFG_GET_MEMORY_POOL,         // 获取执行器内部的内存池, 用于跨模型内存共享 必须在调用 Prepare 接口之后获取
        EXE_EXPAND_CFG_SET_MEMORY_POOL,         // 设置执行器的外部共享内存池, 用于跨模型内存共享 必须在调用 Prepare 接口之前设置
        EXE_EXPAND_CFG_MEMORY_POOL_INFO,        // 获取内存池当前使用情况
        EXE_EXPAND_CFG_CLEAN_MEMORY_POOL,       // 清除内存池中空闲内存
        EXE_EXPAND_CFG_CLOSE_MEMORY_POOL,       // 关闭内存池


        // OPENGL START
        NET_OPTION_CFG_GL_START = 800,
        NET_OPTION_CFG_GL_PRECISION,            // 构造 GL 脚本工厂类 0:低精度 1:高精度
        NET_OPTION_CFG_GL_OPT_FLAG,             // 构造 GL 脚本工厂类 已废弃,可填值 -1
        NET_OPTION_CFG_GL_TEXTURE_BIT,          // GL作外设时, 设置纹理位数: 32/16 
        NET_OPTION_CFG_GL_SCALE,                // GL作外设时, 设置scale (-1: 输入uchar数据，且网络输入范围是0~255; n: 输入uchar数据，且网络输入范围是0~n (n是任意整数))
        NET_OPTION_CFG_GL_INPUT_MODE,           // GL作外设时, 设置mode (0: RGBA2BGRA; 1: BGRA2RGBA; 2: SAME) [default: SAME]
        NET_OPTION_CFG_GL_ENABLE_FLAG,          // GL作外设时, 设置flag 不同平台可能需要相应设置 (Android单线程/多线程需设置为 true; iOS单线程需设置为 false, 多线程需设置为 true)
        NET_OPTION_CFG_GL_SET_SHADEFACTORY,     // GL作外设时, 设置外部共享的shadeFactory指针
        NET_OPTION_CFG_GL_TEXTURE_MODE,         // TODO GL作外设时, 非纹理输入模式是否取消创建纹理转换算子 0:非纹理输入输出 1:纹理输入输出
        NET_OPTION_CFG_GL_USE_GROBAL_SHADEFACTORY,     // GL作外设时, 使用内部全局的shadeFactory对象

        NET_CONFIG_CFG_GL_GET_SHADEFACTORY,     // GL作外设时, 配置获取shadeFactory指针
        NET_CONFIG_CFG_GL_LOAD_SHADER,          // GL作外设时, 配置加载已初始化好了的脚本序列文件 用于减少脚本初始化时间 必须在 LoadModel 接口调用前加载
        NET_CONFIG_CFG_GL_SAVE_SHADER,          // GL作外设时, 配置保存已初始化好了的脚本序列文件 可用于减少下一次脚本初始化时间 必须在 LoadModel 接口调用后保存
        NET_CONFIG_CFG_GL_ENABLE_GL,            // TODO: GL作外设时, 判断设备环境是否支持GL运行

        EXE_EXPAND_CFG_GL_BIND_FBO,             // GL作外设时, 配置重新绑定纹理 主要用于多线程: 如果run线程没有创建纹理过程,则需要重新绑定纹理过程(之前绑定的是在init线程) 
        EXE_EXPAND_CFG_GL_CREATE_ALL_TEXTURE,   // GL作外设时, 配置创建所有纹理 主要用于降低常驻内存: Run前重新创建所有纹理
        EXE_EXPAND_CFG_GL_DELETE_ALL_TEXTURE,   // GL作外设时, 配置删除所有纹理 主要用于降低常驻内存: Run后删除所有纹理
        
        NET_OPTION_CFG_GL_FSCALE,               // GL作外设时, 设置scale(浮点)

        EXE_EXPAND_CFG_GL_CUSTOMER_SHADER,      // 自定义输出脚本（仅适用于纹理输出）
        EXE_EXPAND_CFG_GL_CUSTOMER_PARAM,       // 自定义输出参数
        // OPENGL END



        // HEXAGON START
        NET_OPTION_CFG_HEXAGON_START = 1000,

        // HEXAGON END
        
        // METAL START
        NET_OPTION_CFG_METAL_START = 1200,
        
        NET_OPTION_CFG_METAL_SET_CONTEXT,          // METAL作外设时, 设置context指针
        NET_OPTION_CFG_METAL_MEM_TYPE,               //内存形式： 0:mtlbuffer  1: mtltexture
        NET_OPTION_CFG_METAL_LIB_PATH,             //配置METAL LIB路径
        // METAL END

        // CUDA START
        NET_OPTION_CFG_CUDA_START = 1400,
        NET_OPTION_CFG_CUDA_SET_NETWORK,
        NET_OPTION_CFG_CUDA_SET_TENSORMAP,
        NET_OPTION_CFG_CUDA_ENGINE_MODE,
        NET_OPTION_CFG_CUDA_OFFLINE_MODEL_PATH,
		NET_OPTION_CFG_CUDA_SET_ADAPTER,
        NET_OPTION_CFG_CUDA_ADAPTER_LIB_HANDLE,
        //CUDA END

        //NPU START
        NET_OPTION_CFG_NPU_START = 1600,
        NET_OPTION_CFG_NPU_SET_MODEL,
        NET_OPTION_CFG_NPU_IRMODEL_PATH,
        NET_OPTION_CFG_NPU_IRMODEL,
        NET_OPTION_CFG_NPU_ADAPTER_LIB_HANDLE,
        //NPU END

        // COREML
        NET_OPTION_CFG_COREML_START = 1800,
        NET_OPTION_CFG_COREML_NETWORK_MODEL,    // 内部应用，coreml中间模型抽象成的实例
        NET_OPTION_CFG_COREML_INNER_RENAME,     // 内部应用，适配coreml中间模型过程中产生的边名重命名问题
        NET_OPTION_CFG_COREML_MODEL_QUANTIZATION,     // 压缩位数：1-8，16, 目前仅支持16位
        EXE_EXPAND_CFG_COREML_RUN_MODE,         // coreml运行模式 0:cpu 1:coreml 默认值 1
        NET_OPTION_CFG_COREML_MODEL_TYPE,       // 使用coreml方式 0(默认值):导入mlmodelc模型 1:导入manis模型（manis库需开启相关编译选项）
        NET_OPTION_CFG_COREML_MODEL_CONVERT_INFO,       // 传入 CoreMLModelConverter 指针，用于导入manis模型后指导生成mlmodel模型
        EXE_EXPAND_CFG_COREML_GL_CONTEXT,       // 设置gl上下文
        NET_OPTION_CFG_COREML_LIB_PATH,             //配置COREML METAL LIB路径
        // COREML END

        EXTEND_OPTION_CFG_RESERVE_START = 10000,
        EXTEND_OPTION_CFG_RESERVE_END = 10240
    } ExtendOptionID;

    /// @brief 内存统计信息
    struct CPUBufferInfo{
        uint32_t size = 0; 
    };

    struct GLBufferInfo{
        uint32_t id = 0;
        uint32_t fbo = 0;
        uint16_t width = 0;
        uint16_t height = 0;
        uint32_t type = 0;

        uint32_t reverse[10];
    };

    struct BufferMonitorInfo {
        void *ptr = nullptr;
        uint32_t count = 0;

        int buffer_size = 0;
        int buffer_free_size = 0;
        int buffer_free_count = 0;
        int buffer_total_count = 0;

        int reverse[10];
    };

    enum EventType
    {
        ///////////
        //COMMON
        Event_NULL = 0,
        Event_DataCore = 1,
        Event_Statistic_Resource = 2,

        ///////////
        //OPENGL
        Event_GL_ShadersFactory_Register = 50,
        Event_GL_ShadersFactory_Load = 51,
        Event_GL_ShadersFactory_Save = 52,
        Event_GL_IsSupportGl = 53,
        Event_GL_Texture = 54,

        ///////////
        //
        ///////////
        //Event_BASE_MemPool = 20
    };

    enum LayoutType
    {
        NCHW = 0,
        NHWC = 1,
        NCHWC4 = 2,
        NCHWC8 = 3,
    };

    enum DeviceType {
        DEVICE_AUTO = 0,
        DEVICE_CPU = 1,
        DEVICE_OPENGL = 2,
        DEVICE_OPENCL = 3,
        DEVICE_CUDA = 4,
        DEVICE_HEXAGON = 5,
        DEVICE_METAL = 6,
        DEVICE_WEBGL = 7,
        DEVICE_GLCS = 8,
        DEVICE_HIAI_NPU = 9,
        DEVICE_COREML = 10,
        DEVICE_LAST
    };
    
    enum DataType
    {
        DATA_TYPE_UNDEFINED = 0,
        DATA_TYPE_FLOAT = 1,
        DATA_TYPE_UINT8 = 2,
        DATA_TYPE_INT32 = 3,
        DATA_TYPE_STRING = 4,
        DATA_TYPE_BOOL = 5,
        DATA_TYPE_INT8 = 6,
        DATA_TYPE_UINT16 = 7,
        DATA_TYPE_INT16 = 8,
        DATA_TYPE_INT64 = 9,
        DATA_TYPE_FLOAT16 = 10,
        DATA_TYPE_DOUBLE = 11,
        DATA_TYPE_UINT32 = 12,
        DATA_TYPE_UINT64 = 13,
        DATA_TYPE_BFLOAT16 = 14,
        DATA_TYPE_TORCH_UINT8 = 15,
        DATA_TYPE_NUM
    };

    enum ShapeIdx {
        Shape_N_ = 0,
        Shape_C_,
        Shape_H_,
        Shape_W_,
        Shape_C4_,
        ShapeIdx_Over
    };

    enum ManisStatus{
        STATUS_SUCCESS = 0,           //成功
        STATUS_NOT_INITIALIZED,       //没有初始化
        STATUS_INVALID_VALUE,         //无效的值
        STATUS_MEM_ALLOC_FAILED,      //内存分配失败
        STATUS_UNKNOWN_ERROR,         //未知错误
        STATUS_OUT_OF_AUTHORITY,      //
        STATUS_OUT_OF_MEM,            //
        STATUS_UN_IMPL_ERROR,         //
        STATUS_WRONG_DEVICE,          //错误的设备
        STATUS_NEED_RESHAPE,
        STATUS_HEXAGON_NNLIB_VERSION_ERROR,
        STATUS_HEXAGON_FAILED_ACCESS_HEXAGON,
        STATUS_HEXAGON_FAILED_CONFIG_HEXAGON,
        STATUS_HEXAGON_FAILED_INIT_NET,
        STATUS_HEXAGON_FAILED_PREPARE_NET,
        STATUS_HEXAGON_FAILED_DEINIT_NET,
        STATUS_HEXAGON_FAILED_RUN_NET,
    };

    enum TensorMemType {
        TensorMem_CPU = 0,                 // 通用的CPU内存方式
        TensorMem_TEXTURE = 1,             // 通用的texture纹理方式 
        TensorMem_WEBGL_ACCELERATE = 2,    // 未经缩放/转换/处理 的cpu数据
        TensorMem_LAST
    };

    enum MemoryMode {
        MemoryMode_Default = 0,
        MemoryMode_MemoryFirst,
    };
    
    enum EngineMode {
        EngineOnline = 0,
        EngineOffline,
    };

    enum FloatPrecision {
        PRECISION_F32,
        PRECISION_FP16,
        PRECISION_BF16,
    };

    enum OptimizeType {
        Optimize_No = 0,
        Optimize_OperatorTuning,
        Optimize_SaveDeviceModel,
        Optimize_SaveCLProgramBinary,
    };

    enum CoreMLModelType {
        MODEL_SUFFIX_MLMODELC = 0,
        MODEL_SUFFIX_MANIS,
    };

    }  // namespace manis
