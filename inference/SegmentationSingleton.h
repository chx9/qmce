//
// Created by u on 23-10-11.
//

#ifndef MCE_SEGMENTATIONSINGLETON_H
#define MCE_SEGMENTATIONSINGLETON_H
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "onnxruntime/cpu_provider_factory.h"
#include "common.h"
#include <QString>
#include <QCoreApplication>
#include <opencv2/opencv.hpp>
#include <iostream>

typedef std::vector<std::vector<cv::Point>> Contour;
class SegmentationSingleton {
private:
    std::string model_path;
    const int input_width_ = 256;
    const int input_height_ = 256;
    Ort::Session *session_;
    OrtAllocator *allocator_;
    OrtStatus *status_;
    Ort::MemoryInfo memory_info_ = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Env env_ = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "ONNXRuntime");;
    const std::vector<const char *> input_node_names = {"modelInput"};
    const std::vector<const char *> output_node_names = {"modelOutput"};
    ~SegmentationSingleton() {
    }

    SegmentationSingleton();

public:

    static SegmentationSingleton &getInstance();

    std::vector<Contour> performSegmentation(const std::vector<cv::Mat> &imgs_);
};



#endif //MCE_SEGMENTATIONSINGLETON_H
