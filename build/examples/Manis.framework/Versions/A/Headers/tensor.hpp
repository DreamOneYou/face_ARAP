#ifndef _TENSOR_HPP
#define _TENSOR_HPP

#include "manis_def.hpp"

namespace manis {

/// @brief 对外张量类. 2.3.x 版本之后的新接口
class MANIS_EXPORT Tensor final {
public:
    /// @brief 构造一个 Tensor
    explicit Tensor(const DeviceType& dev = DEVICE_CPU, const LayoutType& layout = NCHW, const DataType& dat = DATA_TYPE_FLOAT);
    ~Tensor();

    Tensor(const Tensor& other);
    Tensor &operator = (const Tensor& other);

    /// @brief 深拷贝
    Tensor& CopyFrom(const Tensor& other);

    /// @brief 获取类型信息
    DeviceType GetDeviceType() const;
    LayoutType GetLayoutType() const;
    DataType GetDataType() const;

    bool SetType(TensorMemType type);
    TensorMemType GetType() const;

    /// @brief 设置维度的大小
    /// @param in: axis 指定要改变大小的维度
    /// @param in: value 指定维度的新大小
    Tensor& SetDim(uint32_t axis, uint32_t value);
    
    /// @brief 批量设置维度的大小
    /// @param in: ndim 指定一共几维
    /// @param in: array 数组,指定维度的新大小
    Tensor& SetDim(uint32_t ndim, uint32_t* array);

    /// @brief 追加维度, 同时指定其大小
    /// @param in: value 追加维度的大小
    Tensor& AddDim(uint32_t value);
    
    /// @brief 获取指定维度的大小
    /// @param in: axis 指定要获取的维度
    uint32_t GetDim(uint32_t axis) const;

    /// @brief 获取维度个数
    uint32_t GetDimNum() const;

    /// @brief 获取维度大小的数组
    /// @param out: array 数组大小应大于等于 GetDimNum()
    void GetDimArray(uint32_t* array) const;

    /// @brief 获取维度大小的乘积 [start_axis, end_axis) default: dim0 * dim1 * dim2 * dim3...
    uint32_t GetDimCount(int start_axis = 0, int end_axis = -1) const;

    /// @brief 获取内存块大小
    uint32_t GetBytes() const;
    /// @brief 获取数据类型大小
    uint32_t GetElemsize() const;

    /// @brief 设置内存数据, 直接引用用户管理的一块内存, 析构时不会一起释放
    Tensor& ReferenceFromData(const void *data);
    /// @brief 设置内存数据, 拷贝一份用户的一块内存, 拷贝大小为 GetBytes()
    Tensor& CopyFromData(const void *data);

    /// @brief 获取内存地址 - 当且仅当未分配内存且有维度信息时, 会创建一块内存
    void* MutableData();
    /// @brief 获取内存地址 - 获取tensor指向的内存块
    const void* Data() const;

    /// @brief 调试使用, 读取/保存成二进制文件
    bool FromFile(const char* path);
    bool ToFile(const char* path);

private:
    class Impl;
    Impl* impl_;
};
    
} // namespace manis


#endif