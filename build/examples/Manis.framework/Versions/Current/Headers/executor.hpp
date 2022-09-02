#ifndef _EXECUTOR_HPP
#define _EXECUTOR_HPP

#include "net.hpp"
#include "tensor.hpp"

namespace manis {

/// @brief 执行器类. 2.3.x 版本之后的新接口
class MANIS_EXPORT Executor {
public:
    /// @brief 算子性能监测
    struct MANIS_EXPORT OpBenchmarkInfo {
        int op_idx;
        int op_name;
        int op_type;
        char op_type_ext[32];
        float op_run_time;
        int op_in_num;
        int op_in_shape[4][5];
        int op_out_num;
        int op_out_shape[4][5];
        int diff_memory_value;
        int max_alloc_size;
        int reverse[10];
        float flops;
    };
    /// @brief 回调函数指针类型
    typedef void (*OpRunCallback)(Tensor* tensor, int tensor_num, void* user_data);
    typedef void (*OpBenchmarkCallback)(const OpBenchmarkInfo& benchmark, void* user_data);
    typedef bool (*OpCustomRunCallback)(int in_num, Tensor* in, int out_num, Tensor* out, void* user_data);
    typedef void (*BufferMonitorCallback)(const BufferMonitorInfo& buffer_monitor, void* user_data);

public:
    /// @brief 构建执行器
    static Executor* CreateExecutor(Net* net, ExtendOptions* options = nullptr);
    /// @brief 释放执行器
    static void ReleaseExecutor(Executor* exe);

    /// @brief 析构执行器
    virtual ~Executor();
    
    /// @brief 设置输入
    virtual bool SetInput(const int tensor_index, Tensor& tensor) = 0;
    virtual bool SetInput(const char* tensor_name, Tensor& tensor) = 0;

    /// @brief 设置输出
    virtual bool SetOutput(const int tensor_index, Tensor& tensor) = 0;
    virtual bool SetOutput(const char* tensor_name, Tensor& tensor) = 0;

    /// @brief 推理前的准备工作, 用于将所有配置生效，可能耗时，可以在初始化阶段先调用准备好，如果没调用在Run内部也会调用
    virtual bool Prepare() = 0;
    /// @brief 推理测试 - 针对单输入单输出从头运行到尾的情况
    virtual bool Run(Tensor &input, Tensor &output) = 0;
    /// @brief 推理测试 - 针对输入/输出已经预先配好的情况
    virtual bool Run() = 0;

    /// @brief 通过注册回调扩展功能 - 获取/修改算子的输入(输出)
    virtual bool SetInputCallback(const char* op_name, OpRunCallback func, void* user_data) = 0;
    virtual bool SetOutputCallback(const char* op_name, OpRunCallback func, void* user_data) = 0;

    /// @brief 通过注册回调扩展功能 - 重写算子(等价于自定义算子)
    virtual bool SetOpCustomRunCallback(const char* op_name, OpCustomRunCallback func, void* user_data) = 0;

    /// @brief 通过注册回调扩展功能 - 监测算子性能
    virtual bool SetBenchmarkCallback(OpBenchmarkCallback func, void* user_data) = 0;

    /// @brief 通过注册回调扩展功能 - 监测内存池
    virtual bool SetBufferMonitorCallback(BufferMonitorCallback func, void* user_data) = 0;

    /// @brief 通过自定义配置ID扩展功能
    virtual bool Expand(ExtendOptionID key, int32_t& value) = 0;
    virtual bool Expand(ExtendOptionID key, void* value = NULL) = 0;

protected:
    Executor();
};

}

#endif // _EXECUTOR_HPP