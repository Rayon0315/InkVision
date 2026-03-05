#include "YOLO_Detector.h"
#include <QWidget>

YOLO_Detector::YOLO_Detector(const std::string &model_path,
                             int input_size,
                             float conf_threshold,
                             float nms_threshold) {
    net = cv::dnn::readNetFromONNX(model_path);

    this->input_size = input_size;
    this->conf_threshold = conf_threshold;
    this->nms_threshold = nms_threshold;

    net.setPreferableBackend((cv::dnn::DNN_BACKEND_OPENCV));
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    num_ops = {
        {0, "0"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"},
        {5, "5"}, {6, "6"}, {7, "7"}, {8, "8"}, {9, "9"},
        {10, "/"}, {11, "="}, {12, "-"}, {13, "*"}, {14, "+"}
    };
}

void YOLO_Detector::basicTest() {
    if (!net.empty()) {
        std::cout << "Success!" << std::endl;
    }

    cv::Mat dummy = cv::Mat::zeros(512, 512, CV_8UC3);

    cv::Mat blob = cv::dnn::blobFromImage(
        dummy,
        1.0 / 255.0,
        cv::Size(512, 512),
        cv::Scalar(),
        true,
        false
        );

    net.setInput(blob);

    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    for (auto &out : outputs) {
        std::cout << "Output dims: ";
        for (int i = 0; i < out.dims; i++)
            std::cout << out.size[i] << " ";
        std::cout << std::endl;
    }
}

std::vector<Detection> YOLO_Detector::detect(const cv::Mat& image) {
    std::vector<Detection> result;

    float scale;
    int top, left;

    cv::Mat blob = preprocess(image, scale, top, left);

    auto outputs = infer(blob);

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    decode(outputs[0], class_ids, confidences, boxes);

    auto indices = applyNMS(boxes, confidences);

    for (int idx : indices) {
        Detection det;
        det.class_id = class_ids[idx];
        det.confidence = confidences[idx];
        det.box = boxes[idx];

        result.push_back(det);
    }

    return result;
}

cv::Mat YOLO_Detector::preprocess(
    const cv::Mat& image,
    float& scale,
    int& top,
    int& left
) {
    cv::Mat padded = letterbox(image, scale, top, left);
    // cv::imshow("padded", padded);

    return cv::dnn::blobFromImage(
        padded,
        1.0/255.0,
        cv::Size(input_size,input_size),
        cv::Scalar(),
        true,
        false
    );
}

std::vector<cv::Mat> YOLO_Detector::infer(const cv::Mat& blob) {
    net.setInput(blob);

    std::vector<cv::Mat> outputs;

    net.forward(outputs, net.getUnconnectedOutLayersNames());

    return outputs;
}

void YOLO_Detector::decode(const cv::Mat& output,
                           std::vector<int>& class_ids,
                           std::vector<float>& confidences,
                           std::vector<cv::Rect>& boxes) {
    int rows = output.size[1];
    int dimensions = output.size[2];
    int num_classes = dimensions - 5;

    float* data = (float*)output.data;

    // float maxObj = 0., maxConf = 0;
    // double mmaxScore = 0.;

    for (int i = 0; i < rows; i++) {
        float obj = data[4];

        if (obj > conf_threshold) {
            cv::Mat scores(1, num_classes, CV_32FC1, data + 5);

            cv::Point classIdPoint;
            double maxScore;

            minMaxLoc(scores, 0, &maxScore, 0, &classIdPoint);

            float conf = obj * maxScore;


            // maxObj = std::max(obj, maxObj);
            // mmaxScore = std::max(maxScore, mmaxScore);
            // maxConf = std::max(conf, maxConf);
            // std::cout << "obj: " << obj
            //           << " class: " << maxScore
            //           << " conf: " << conf
            //           << std::endl;

            if (conf > conf_threshold && maxScore > 0.7) {
                float cx = data[0];
                float cy = data[1];
                float w = data[2];
                float h = data[3];

                int left = int(cx - w/2);
                int top = int(cy - h/2);

                boxes.emplace_back(left, top, (int)w, (int)h);
                confidences.push_back(conf);
                class_ids.push_back(classIdPoint.x);
            }
        }

        data += dimensions;
    }

    // std::cout << "maxObj: " << maxObj
    //           << " mmaxScore: " << mmaxScore
    //           << " maxConf: " << maxConf
    //           << std::endl;
}

std::vector<int> YOLO_Detector::applyNMS(
    const std::vector<cv::Rect>& boxes,
    const std::vector<float>& confidences
) {
    std::vector<int> indices;

    cv::dnn::NMSBoxes(
        boxes,
        confidences,
        conf_threshold,
        nms_threshold,
        indices
    );

    return indices;
}

void YOLO_Detector::functionTest(const cv::Mat& image) {
    auto detections = detect(image);

    std::cout << "Detection count: " << detections.size() << std::endl;

    for (const auto& d : detections) {
        std::cout
            << "class: " << d.class_id
            << "  confidence: " << d.confidence
            << "  box: "
            << d.box.x << ", "
            << d.box.y << ", "
            << d.box.width << ", "
            << d.box.height
            << std::endl;
    }
}

std::string YOLO_Detector::generateExpressionOri(const cv::Mat& image) {
    auto detection = detect(image);

    std::sort(detection.begin(), detection.end(),
              [](const Detection& a, const Detection& b) {
        int ax = a.box.x + a.box.width / 2;
        int bx = b.box.x + b.box.width / 2;
        return ax < bx;
    });

    std::string result = "";
    for (auto det : detection) {
        result += num_ops[det.class_id];
    }

    return result;
}

std::string YOLO_Detector::generateExpression(const cv::Mat& image)
{
    const int target_h = input_size;     // 512
    const int window_w = input_size;     // 512
    const int stride   = input_size / 2; // 256

    // 1 resize height
    float scale = (float)target_h / image.rows;
    int new_w = int(image.cols * scale);

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(new_w, target_h));
    cv::imshow("resized", resized);

    std::vector<Detection> global_detections;

    // 2 sliding window
    for (int x = 0; x < new_w; x += stride)
    {
        int w = std::min(window_w, new_w - x);

        cv::Rect roi(x, 0, w, target_h);
        cv::Mat crop = resized(roi);

        // 如果窗口不足512，padding
        cv::Mat padded = cv::Mat::zeros(target_h, window_w, crop.type());
        crop.copyTo(padded(cv::Rect(0,0,crop.cols,crop.rows)));

        auto dets = detect(padded);

        for (auto& d : dets)
        {
            Detection g = d;

            // bbox 映射回全局坐标
            g.box.x += x;

            global_detections.push_back(g);
        }
    }

    // 3 全局 NMS
    std::vector<cv::Rect> boxes;
    std::vector<float> confs;

    for (auto& d : global_detections)
    {
        boxes.push_back(d.box);
        confs.push_back(d.confidence);
    }

    std::vector<int> indices;

    cv::dnn::NMSBoxes(
        boxes,
        confs,
        conf_threshold,
        nms_threshold,
        indices
        );

    std::vector<Detection> final_det;

    for (int i : indices)
        final_det.push_back(global_detections[i]);

    // 4 排序
    std::sort(final_det.begin(), final_det.end(),
              [](const Detection& a, const Detection& b)
              {
                  int ax = a.box.x + a.box.width/2;
                  int bx = b.box.x + b.box.width/2;
                  return ax < bx;
              });

    // 5 生成表达式
    std::string result;

    for (auto& d : final_det)
        result += num_ops[d.class_id];

    return result;
}

cv::Mat YOLO_Detector::letterbox(
    const cv::Mat& image,
    float& scale,
    int& top,
    int& left
) {
    int w = image.cols;
    int h = image.rows;

    scale = std::min(
        (float)input_size / w,
        (float)input_size / h
        );

    int new_w = int(w * scale);
    int new_h = int(h * scale);

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(new_w, new_h));

    int pad_w = input_size - new_w;
    int pad_h = input_size - new_h;

    left = pad_w / 2;
    top  = pad_h / 2;

    cv::Mat padded;

    cv::copyMakeBorder(
        resized,
        padded,
        top,
        pad_h - top,
        left,
        pad_w - left,
        cv::BORDER_CONSTANT,
        cv::Scalar(114,114,114)
        );

    return padded;
}
