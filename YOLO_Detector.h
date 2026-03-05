#ifndef YOLO_DETECTOR_H
#define YOLO_DETECTOR_H

#include <opencv2/opencv.hpp>

struct Detection {
    int class_id;
    float confidence;
    cv::Rect box;
};

class YOLO_Detector {
private:
    cv::dnn::Net net;
    std::map<int, std::string> num_ops;

    int input_size;
    float conf_threshold;
    float nms_threshold;

    cv::Mat letterbox(
        const cv::Mat& image,
        float& scale,
        int& top,
        int& left
    );
    cv::Mat preprocess(
        const cv::Mat& image,
        float& scale,
        int& top,
        int& left
    );
    std::vector<cv::Mat> infer(const cv::Mat& blob);
    void decode(
        const cv::Mat& output,
        std::vector<int>& class_ids,
        std::vector<float>& confidences,
        std::vector<cv::Rect>& boxes
    );
    std::vector<int> applyNMS(
        const std::vector<cv::Rect>& boxes,
        const std::vector<float>& confidence
    );

public:
    YOLO_Detector(const std::string& model_path = "models/yolo.onnx",
                  int input_size = 512,
                  float conf_threshold = 0.5,
                  float nms_threshold = 0.3);

    std::vector<Detection> detect(const cv::Mat& image);

    void basicTest();
    void functionTest(const cv::Mat& image);
    std::string generateExpressionOri(const cv::Mat& image);
    std::string generateExpression(const cv::Mat& image);
};

#endif // YOLO_DETECTOR_H
