#ifndef _NET_HPP
#define _NET_HPP

#include <stdio.h>
#include <stdlib.h>
#include "manis_def.hpp"

namespace manis {

/// @brief 功能扩展项类
class MANIS_EXPORT ExtendOptions {
public:
    ExtendOptions();
    ~ExtendOptions();

    ExtendOptions& Add(ExtendOptionID id, int32_t value);
    ExtendOptions& Add(ExtendOptionID id, const void* value);

    void* Get();

    ExtendOptions& Add(ExtendOptionID id, float value);

private:
    class Impl;
    Impl* impl_;
};

/// @brief 模型网络类. 2.3.x 版本之后的新接口
class MANIS_EXPORT Net {
public:
    /// @brief 模型存储的网络信息
    typedef struct __ModelNetInfo {
        uint32_t input_num = 0;                     // 模型网络的输入个数
        uint32_t output_num = 0;                    // 模型网络的输出个数
        DeviceType device_type = DEVICE_CPU;        // 当前无使用
        LayoutType layout_type = NCHW;              // 当前无使用
        DataType data_type = DATA_TYPE_UNDEFINED;   // 当前主要是记录该模型是定点/浮点
        uint32_t model_version[3];                  // 转换模型时用的工具版本号 major,minor,patch
        char net_name[256];                         // 模型网络名, 实际用不到256，后面的空间作为预留
    } ModelNetInfo;

    /// @brief 模型存储的输入/输出Tensor信息
    typedef struct __ModelTensorInfo {
        DataType data_type;         // Tensor的数据类型
        uint32_t dim_num;           // Tensor的维度
        uint32_t dim[20];           // Tensor的维度信息, 实际用不到20，后面空间作为预留
    } ModelTensorInfo;

public:
    /// @brief 加载模型并创建Net, 默认适配CPU设备
    static Net* CreateNet(ExtendOptions* options = nullptr);
    /// @brief 释放Net
    static void ReleaseNet(Net* net);
    
    virtual ~Net();

    virtual bool LoadModel(const char *path, ExtendOptions* options = nullptr) = 0;
    virtual bool LoadModel(const uint8_t* model_data, uint32_t dataLen, ExtendOptions* options = nullptr) = 0;

    /// @brief 获取模型存储的信息
    virtual bool GetNetInfo(ModelNetInfo& info) = 0;
    virtual bool GetInputTensorInfo(uint32_t index, ModelTensorInfo& info) = 0;
    virtual bool GetOutputTensorInfo(uint32_t index, ModelTensorInfo& info) = 0;

    /// @brief TODO: 保存已优化过的模型
    virtual bool SaveModel(const char* path) = 0;
    virtual bool SaveModel(uint8_t* data, uint32_t& dataLen) = 0;

    /// @brief 配置扩展功能
    virtual bool Config(const ExtendOptionID cfg_id, int32_t& value) =0;
    virtual bool Config(const ExtendOptionID cfg_id, void* value = nullptr) =0;

    virtual bool GetInputTensorInfoByName(const char* name, ModelTensorInfo& info) = 0;
    virtual bool GetOutputTensorInfoByName(const char* name, ModelTensorInfo& info) = 0;

protected:
    Net();
};

    /// @brief 检测是否支持该外设
    MANIS_EXPORT bool IsSupport(DeviceType device);
    /// @brief 检测是该外设是否支持对应数据类型
    MANIS_EXPORT bool IsSupport(DeviceType device, DataType type);

    /// @brief 检测模型是否有效
    MANIS_EXPORT bool CheckModelValid(const uint8_t* model_data, uint32_t data_len);

    /// @brief 返回底层硬件信息
    MANIS_EXPORT char* GetDeviceInfo(int &len);

    /// @brief 获取Manis的编译版本
    MANIS_EXPORT const int8_t* ManisVersion();
    
    // bind all threads on little clusters if powersave enabled
    // affacts HMP arch cpu like ARM big.LITTLE
    // only implemented on android at the moment
    // switching powersave is expensive and not thread-safe
    // 0 = all cores enabled(default)
    // 1 = only little clusters enabled
    // 2 = only big clusters enabled
    MANIS_EXPORT void SetCPUPowerMode(int model);

    MANIS_EXPORT void* CreateMemPool(ExtendOptions* options = nullptr);

    // SetMemoryMode实验性功能，用于测试内存相关功能
    MANIS_EXPORT void SetMemoryMode(int mode);

    MANIS_EXPORT bool SetGlobalOptions(ExtendOptions* options = nullptr);
}

#endif /*  */
