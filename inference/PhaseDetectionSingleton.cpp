//
// Created by u on 23-10-19.
//

#include "PhaseDetectionSingleton.h"

PhaseDetectionSingleton::PhaseDetectionSingleton() {
    QString onnxFilePath = QCoreApplication::applicationDirPath() + "/models/phase.onnx";
    model_path = onnxFilePath.toStdString();
    Ort::SessionOptions session_options;
    status_ = OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0); // Use GPU (device_id = 0)
    session_ = new Ort::Session(env_, model_path.c_str(), session_options);
    py::initialize_interpreter();
}

PhaseDetectionSingleton &PhaseDetectionSingleton::getInstance() {
    static PhaseDetectionSingleton instance;
    return instance;
}

std::vector<int> PhaseDetectionSingleton::findPeaks(const std::vector<double> &data) {
    try {

        py::module sys = py::module::import("sys");
//        sys.attr("path").attr("append")("/home/u/Projects/trypybind/");
        std::string script_path = (QCoreApplication::applicationDirPath()+"/script/").toStdString();
        qDebug()<<script_path.c_str();
        sys.attr("path").attr("append")(script_path.c_str());
        py::module findPeakModule = py::module::import("findpeak");
        py::function find_peaks = findPeakModule.attr("find_peaks");          // get the add function
        py::list py_list;
        for (double i: data) {
            py_list.append(i);
        }
        py::object result = find_peaks(py_list);
        py::list result_list = result.cast<py::list>();
        std::vector<int> vec_result;
        for (const py::handle &item: result_list) {
            // Use py::cast to convert each element to an integer
            int value = py::cast<int>(item);
            vec_result.push_back(value);
        }
        return vec_result;
    }catch(py::error_already_set const &pythonErr) {
        qDebug()<<pythonErr.what();
    }
    return std::vector<int>{};
}

std::vector<int> PhaseDetectionSingleton::performPhaseDetection(const std::vector<cv::Mat> &imgs_) {
    std::vector<cv::Mat> imgs(imgs_);
    int frame_cnt = imgs.size();
    Accumulator accumulator(frame_cnt);
    for (auto &img: imgs) {
        imgPreprocessing(img, input_width_);
    }
    std::vector<int64_t> input_node_dims = {1, seq_len_, 3, input_width_, input_height_};
    int i = 0;
    std::vector<cv::Mat> input_imgs;
    while (i < frame_cnt) {
        int end = i + seq_len_;
        if (end < frame_cnt) {
            input_imgs = std::vector<cv::Mat>(imgs.begin() + i, imgs.begin() + end);
        } else {
            input_imgs = std::vector<cv::Mat>(imgs.end() - seq_len_, imgs.end());
            i = frame_cnt - seq_len_;
        }
        std::vector<Ort::Value> input_tensors;
        auto blob = cv::dnn::blobFromImages(input_imgs, 1, cv::Size(128, 128), cv::Scalar(0, 0, 0), false, true);
        input_tensors.emplace_back(
                Ort::Value::CreateTensor<float>(memory_info, blob.ptr<float>(), blob.total(), input_node_dims.data(),
                                                input_node_dims.size()));
        auto output_tensors = session_->Run(Ort::RunOptions{nullptr}, input_node_names_.data(), input_tensors.data(),
                                            input_node_names_.size(), output_node_names_.data(),
                                            output_node_names_.size());
        float *float_arr = output_tensors.front().GetTensorMutableData<float>();
        accumulator.add(float_arr, i);
        if (end > frame_cnt) {
            break;
        }
        i = end;
    }
    // reset session
    Ort::SessionOptions session_options;
    delete session_;
    session_ = new Ort::Session(env_, model_path.c_str(), session_options);
    auto result = accumulator.getResult();
    auto peaks = findPeaks(result);
    return peaks;
}

