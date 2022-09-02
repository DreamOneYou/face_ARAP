//
// Created by root on 9/17/20.
//

#ifndef EOS_FACEDECTOR_HPP
#define EOS_FACEDECTOR_HPP

#include "Eigen/Core"
#include "boost/filesystem.hpp"

#include "mtface/mtface.h"
#include "mtface/types.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


namespace fs = boost::filesystem;

void onError(char *error)
{
    std::cout << error << std::endl;
    mtface_free(error);
    abort();
}

class ThrowOnError
{
public:
    ~ThrowOnError()
    {
        expect();
    }

    void expect()
    {
        if (err_)
        {
            std::cout << err_ << std::endl;
            auto e = std::runtime_error(err_);
            mtface_free(err_);
            err_ = nullptr;
            throw e;
        }
    }

    operator char **()
    {
        expect();
        return &err_;
    }

private:
    char *err_ = nullptr;
};

class FaceDetector {
public:
    FaceDetector(fs::path model_path)
    {
        // 0. 获取版本信息
        std::cout << mtface_version() << std::endl;

        // 1. 创建静态图检测器， 此时它啥也做不了，因为还没有加载模型
        detector = mtface_detector_create(ThrowOnError());

        // 2. 配置一些参数
        mtface_options_t *opts = mtface_detector_get_options(detector, nullptr, ThrowOnError());
        mtface_options_set(opts, mtface_option_minimal_face, 0.073);
        mtface_options_set(opts, mtface_option_score_threshold, 0.5);
        mtface_options_set(opts, mtface_option_enable_visibility, false);
        mtface_options_set(opts, mtface_option_enable_pose_estimation, false);
        mtface_options_set(opts, mtface_option_enable_mesh_generation, true);
        mtface_detector_set_options(detector, opts, ThrowOnError());
        mtface_options_destroy(opts);
        opts = nullptr; // opts 不用试将其销毁

        // 3. 加载需要的功能
        // 3.1 加载模型文件
        mtface_detector_append_model_file(detector, mtface_type_fd, (model_path / "mtface_fd.bin").string().c_str(), true, ThrowOnError());
        mtface_detector_append_model_file(detector, mtface_type_fa_medium, (model_path / "mtface_fa_medium.bin").string().c_str(), true, ThrowOnError());
        mtface_detector_append_model_file(detector, mtface_type_3d, (model_path / "mtface_3d.bin").string().c_str(), true, ThrowOnError());
        // 静态图人脸点，不考虑实时的话， 需要 mtface_type_fa_heavy、mtface_type_refine_eyes、mtface_type_refine_mouth
//        mtface_detector_append_model_file(detector, mtface_type_fa_heavy, (model_path / "mtface_fa_heavy.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_refine_eyes, (model_path / "mtface_refine_eyes.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_refine_mouth, (model_path / "mtface_refine_mouth.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_age, (model_path / "mtface_age_fast.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_gender, (model_path / "mtface_gender.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_glasses, (model_path / "mtface_glasses.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_mustache, (model_path / "mtface_mustache.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_eyelid, (model_path / "mtface_eyelid.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_emotion, (model_path / "mtface_emotion.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_fr, (model_path / "mtface_fr.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_face_quality, (model_path / "mtface_face_quality.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_race, (model_path / "mtface_race.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_ear, (model_path / "mtface_ear.bin").string().c_str(), true, ThrowOnError());
//        mtface_detector_append_model_file(detector, mtface_type_beauty, (model_path / "mtface_beauty.bin").string().c_str(), true, ThrowOnError());

        // 3.2 加载内存模型
        // 读入内存
//        auto buffer = mtface::readModel(model_path + "mtface_age_fast.bin");
        // false 表示加载但是不激活，即不做检测
//        mtface_detector_append_model_buffer(detector, mtface_type_age, buffer.data(), buffer.size(), false, ThrowOnError());

        // 3.3 开关 各项检测器
        mtface_detector_enable_model(detector, mtface_type_fd, true, ThrowOnError());
        mtface_detector_enable_model(detector, mtface_type_fa_medium, true, ThrowOnError());        // （是/否）开启非实时版人脸点定位
        mtface_detector_enable_model(detector, mtface_type_3d, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_gender, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_glasses, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_mustache, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_eyelid, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_emotion, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_fr, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_face_quality, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_race, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_ear, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_beauty, false, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_fd, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_fa_heavy, true, ThrowOnError());        // （是/否）开启非实时版人脸点定位
//        mtface_detector_enable_model(detector, mtface_type_refine_eyes, true, ThrowOnError());     // （是/否）进一步优化眼睛点精度
//        mtface_detector_enable_model(detector, mtface_type_refine_mouth, true, ThrowOnError());    // （是/否）进一步优化嘴唇点精度
//        mtface_detector_enable_model(detector, mtface_type_age, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_gender, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_glasses, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_mustache, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_eyelid, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_emotion, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_fr, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_face_quality, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_race, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_ear, true, ThrowOnError());
//        mtface_detector_enable_model(detector, mtface_type_beauty, true, ThrowOnError());
    }
    ~FaceDetector()
    {
        mtface_detector_destroy(detector);

    }

    std::vector<std::vector<float>> getFacePoints(cv::Mat cvbgr, int detect_times = 1)
    {
        mtface_image_t *image = mtface_image_adapt(nullptr,
                                                   cvbgr.cols, cvbgr.rows, mtface_pixel_format_bgr, 1, cvbgr.data, cvbgr.step,
                                                   nullptr, 0, nullptr, 0, ThrowOnError());
        // 5. 进行检测
        //
        mtface_feature_t *feature = mtface_detector_detect(detector, image, nullptr, ThrowOnError()); // 创建新的句柄
        
        //mtface_feature_set_face_box(feature, 0, mtface_rect_t{160.0f, 200.0f, 180.0f, 180.0f});
        for (size_t i = 0; i < detect_times; i++)
        {
            feature = mtface_detector_detect(detector, image, feature, ThrowOnError());                   // 复用结果句柄
        }
        
        std::cout<<mtface_feature_get_face_size(feature)<<std::endl;

        std::vector<std::vector<float>> face_points;
        // 6. 获取检测结果
        for (int i = 0; i < mtface_feature_get_face_size(feature); ++i)
        {
            std::vector<float> points;
            // 6.2 get landmark
            size_t num_points = 0;
            const float *landmark = mtface_feature_get_face_landmark(feature, i, &num_points);

            for (size_t i = 0; i < num_points; ++i)
            {
                auto x = landmark[2 * i];
                auto y = landmark[2 * i + 1];
                points.push_back(x);
                points.push_back(y);
            }
            face_points.push_back(points);
        }

        mtface_image_destroy(image);
        mtface_feature_destroy(feature);

        return face_points;
    }
   



private:
    mtface_detector_t *detector=NULL;

};


#endif //EOS_FACEDECTOR_HPP
