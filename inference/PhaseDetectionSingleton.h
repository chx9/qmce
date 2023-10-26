//
// Created by u on 23-10-19.
//

#ifndef MCE_PHASEDETECTIONSINGLETON_H
#define MCE_PHASEDETECTIONSINGLETON_H
#include <pybind11/embed.h>
#include <QApplication>
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "onnxruntime/cpu_provider_factory.h"
#include "common.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <chrono>
#include <thread>
#include <mutex>
namespace py = pybind11;
class PhaseDetectionSingleton {
private:
    std::string model_path;
    const int input_width_ = 128;
    const int input_height_ = 128;
    const int seq_len_ = 100;
    Ort::Session *session_;
    OrtAllocator *allocator_;
    OrtStatus *status_;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Env env_ = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "ONNXRuntime");
    const std::vector<const char *> input_node_names_ = {"modelInput"};
    const std::vector<const char *> output_node_names_ = {"modelOutput"};
    PhaseDetectionSingleton();
    ~PhaseDetectionSingleton(){
//        py::finalize_interpreter();  do not finalize_interpreter
    }
public:
    static PhaseDetectionSingleton &getInstance();
    static std::vector<int> findPeaks(const std::vector<double>& data);
    std::vector<int> performPhaseDetection(const std::vector<cv::Mat> &imgs_);
};
#endif //MCE_PHASEDETECTIONSINGLETON_H
