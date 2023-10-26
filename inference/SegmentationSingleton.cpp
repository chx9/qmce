//
// Created by u on 23-10-11.
//

#include "SegmentationSingleton.h"

SegmentationSingleton::SegmentationSingleton() {
    Ort::SessionOptions session_options;
    status_ = OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0); // Use GPU (device_id = 0)
    QString onnxFilePath = QCoreApplication::applicationDirPath() + "/models/seg.onnx";
    model_path = onnxFilePath.toStdString();
    session_ = new Ort::Session(env_, model_path.c_str(), session_options);
}
SegmentationSingleton& SegmentationSingleton::getInstance() {
    static SegmentationSingleton instance;
    return instance;
}


std::vector<Contour> SegmentationSingleton::performSegmentation(const std::vector<cv::Mat> &imgs_) {
    std::vector<cv::Mat> imgs(imgs_);
    int batch_size = imgs.size();
    for (auto &img: imgs) {
        imgPreprocessing(img, input_width_);
    }
    std::vector<int64_t> inputNodeDims = {batch_size, 3, input_width_, input_height_};

    std::vector<Ort::Value> input_tensors;
    auto blob = cv::dnn::blobFromImages(imgs, 1., cv::Size(256, 256), cv::Scalar(0, 0, 0), false, true);
    input_tensors.emplace_back(
            Ort::Value::CreateTensor<float>(memory_info_, blob.ptr<float>(), blob.total(), inputNodeDims.data(),
                                            inputNodeDims.size()));

    auto output_tensors = session_->Run(Ort::RunOptions{nullptr}, input_node_names.data(), input_tensors.data(),
                                        input_node_names.size(), output_node_names.data(), output_node_names.size());
    float *float_arr = output_tensors.front().GetTensorMutableData<float>();
    int output_width = input_width_;
    int output_height = input_height_;
    std::vector<int64_t> output_node_dims = {batch_size, 2, output_width, output_height};
    std::vector<cv::Mat> masks;
    for (int i = 0; i < batch_size; i++) {
        masks.emplace_back(output_height, output_width, CV_8UC1);
    }
    for (int i = 0; i < batch_size; i++) {
        for (int row = 0; row < output_height; ++row) {
            for (int col = 0; col < output_width; ++col) {
                int maxClass = -1;
                float maxProbability = -1.0;
                for (int c = 0; c < 2; ++c) { //  2 classes
                    int index =
                            (i * 2 * output_width * output_height) + (c * output_width * output_height) +
                            (row * output_width) + col;
                    float outputValue = float_arr[index];
                    if (outputValue > maxProbability) {
                        maxProbability = outputValue;
                        maxClass = c;
                    }
                }
                masks[i].at<uchar>(row, col) = (maxClass == 1) ? 255 : 0;
            }
        }
    }
    for (auto &mask: masks) {
        cv::resize(mask, mask, cv::Size(512, 512));
    }
    int i = 0;
    std::vector<Contour> contours_vec;
    contours_vec.resize(batch_size);
    for (int i = 0; i < masks.size(); i++) {
        auto &mask = masks[i];
        cv::resize(mask, mask, cv::Size(512, 512));
        cv::findContours(mask, contours_vec[i], cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    }
    Ort::SessionOptions session_options;
    delete session_;
    session_ = new Ort::Session(env_, model_path.c_str(), session_options);
    return contours_vec;
}