#include "YOLO_Detector.h"

#include <QWidget>

#include <algorithm>
#include <cmath>
#include <iostream>

YOLO_Detector::YOLO_Detector(const std::string& model_path,
                             int input_height,
                             int input_width,
                             float conf_threshold,
                             float nms_threshold) {
    net = cv::dnn::readNetFromONNX(model_path);

    this->input_height = input_height;
    this->input_width = input_width;
    this->conf_threshold = conf_threshold;
    this->nms_threshold = nms_threshold;

    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    num_ops = {
        {0, "0"},  {1, "1"},  {2, "2"},  {3, "3"},  {4, "4"},
        {5, "5"},  {6, "6"},  {7, "7"},  {8, "8"},  {9, "9"},
        {10, "/"}, {11, "="}, {12, "-"}, {13, "*"}, {14, "+"}
    };
}

void YOLO_Detector::basicTest() {
    if (!net.empty()) {
        std::cout << "Success!" << std::endl;
    } else {
        std::cout << "Failed to load model." << std::endl;
        return;
    }

    cv::Mat dummy = cv::Mat::zeros(input_height, input_width, CV_8UC3);

    cv::Mat blob = cv::dnn::blobFromImage(dummy,
                                          1.0 / 255.0,
                                          cv::Size(input_width, input_height),
                                          cv::Scalar(),
                                          true,
                                          false);

    net.setInput(blob);

    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    for (auto& out : outputs) {
        std::cout << "Output dims: ";
        for (int i = 0; i < out.dims; i++) {
            std::cout << out.size[i] << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<Detection> YOLO_Detector::detect(const cv::Mat& image) {
    std::vector<Detection> result;

    if (image.empty()) {
        std::cout << "detect: image is empty" << std::endl;
        return result;
    }

    if (net.empty()) {
        std::cout << "detect: net is empty" << std::endl;
        return result;
    }

    float scale = 1.0f;
    int top = 0;
    int left = 0;

    cv::Mat blob = preprocess(image, scale, top, left);
    if (blob.empty()) {
        std::cout << "detect: preprocess failed" << std::endl;
        return result;
    }

    std::vector<cv::Mat> outputs = infer(blob);
    if (outputs.empty()) {
        std::cout << "detect: outputs empty" << std::endl;
        return result;
    }

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    decode(outputs[0], class_ids, confidences, boxes);

    // std::cout << "after decode:" << std::endl;
    // std::cout << "boxes.size() = " << boxes.size() << std::endl;
    // std::cout << "confidences.size() = " << confidences.size() << std::endl;
    // std::cout << "class_ids.size() = " << class_ids.size() << std::endl;
    // std::cout << "scale = " << scale
    //           << ", top = " << top
    //           << ", left = " << left << std::endl;
    // std::cout << "image size = " << image.cols << " x " << image.rows << std::endl;
    // std::cout << "input size = " << input_width << " x " << input_height << std::endl;

    if (boxes.empty()) {
        return result;
    }

    std::vector<int> indices = applyNMS(boxes, confidences);

    // std::cout << "after NMS: indices.size() = " << indices.size() << std::endl;

    for (int idx : indices) {
        if (idx < 0 || idx >= static_cast<int>(boxes.size())) {
            continue;
        }

        const cv::Rect& box = boxes[idx];

        // std::cout << "raw box: "
        //           << box.x << ", " << box.y << ", "
        //           << box.width << ", " << box.height
        //           << " conf = " << confidences[idx]
        //           << " cls = " << class_ids[idx] << std::endl;

        float x1 = (static_cast<float>(box.x) - static_cast<float>(left)) / scale;
        float y1 = (static_cast<float>(box.y) - static_cast<float>(top)) / scale;
        float x2 = (static_cast<float>(box.x + box.width) - static_cast<float>(left)) / scale;
        float y2 = (static_cast<float>(box.y + box.height) - static_cast<float>(top)) / scale;

        cv::Rect mapped_box(static_cast<int>(std::round(x1)),
                            static_cast<int>(std::round(y1)),
                            static_cast<int>(std::round(x2 - x1)),
                            static_cast<int>(std::round(y2 - y1)));

        // std::cout << "mapped box before clamp: "
        //           << mapped_box.x << ", " << mapped_box.y << ", "
        //           << mapped_box.width << ", " << mapped_box.height << std::endl;

        int xx1 = std::max(0, mapped_box.x);
        int yy1 = std::max(0, mapped_box.y);
        int xx2 = std::min(image.cols, mapped_box.x + mapped_box.width);
        int yy2 = std::min(image.rows, mapped_box.y + mapped_box.height);

        int ww = xx2 - xx1;
        int hh = yy2 - yy1;

        // std::cout << "clamped box candidate: "
        //           << xx1 << ", " << yy1 << ", "
        //           << ww << ", " << hh << std::endl;

        if (ww <= 0 || hh <= 0) {
            std::cout << "box dropped after clamp" << std::endl;
            continue;
        }

        Detection det;
        det.class_id = class_ids[idx];
        det.confidence = confidences[idx];
        det.box = cv::Rect(xx1, yy1, ww, hh);

        result.push_back(det);
    }

    // std::cout << "final result.size() = " << result.size() << std::endl;
    return result;
}

cv::Mat YOLO_Detector::preprocess(const cv::Mat& image,
                                  float& scale,
                                  int& top,
                                  int& left) {
    cv::Mat padded = letterbox(image, scale, top, left);
    if (padded.empty()) {
        return cv::Mat();
    }

    return cv::dnn::blobFromImage(padded,
                                  1.0 / 255.0,
                                  cv::Size(input_width, input_height),
                                  cv::Scalar(),
                                  true,
                                  false);
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
    class_ids.clear();
    confidences.clear();
    boxes.clear();

    if (output.empty()) {
        std::cout << "decode: output is empty" << std::endl;
        return;
    }

    if (output.dims != 3) {
        std::cout << "decode: unsupported dims = " << output.dims << std::endl;
        return;
    }

    int rows = output.size[1];
    int dimensions = output.size[2];

    // std::cout << "decode rows = " << rows
    //           << ", dimensions = " << dimensions << std::endl;

    float* data = reinterpret_cast<float*>(output.data);

    for (int i = 0; i < std::min(rows, 5); i++) {
        // std::cout << "candidate[" << i << "]: ";
        // for (int j = 0; j < dimensions; j++) {
        //     std::cout << data[j] << " ";
        // }
        // std::cout << std::endl;
        data += dimensions;
    }

    data = reinterpret_cast<float*>(output.data);

    int kept_with_obj = 0;
    int kept_no_obj = 0;

    for (int i = 0; i < rows; i++) {
        float cx = data[0];
        float cy = data[1];
        float w = data[2];
        float h = data[3];

        if (w <= 1.0f || h <= 1.0f) {
            data += dimensions;
            continue;
        }

        if (dimensions >= 6) {
            int num_classes_obj = dimensions - 5;
            float obj = data[4];

            if (num_classes_obj > 0) {
                cv::Mat scores_obj(1, num_classes_obj, CV_32FC1, data + 5);

                cv::Point class_id_point_obj;
                double max_score_obj = 0.0;
                cv::minMaxLoc(scores_obj, nullptr, &max_score_obj, nullptr, &class_id_point_obj);

                float conf_obj = obj * static_cast<float>(max_score_obj);

                if (obj > 0.01f && conf_obj > conf_threshold) {
                    int box_left = static_cast<int>(std::round(cx - w / 2.0f));
                    int box_top = static_cast<int>(std::round(cy - h / 2.0f));
                    int box_width = static_cast<int>(std::round(w));
                    int box_height = static_cast<int>(std::round(h));

                    boxes.emplace_back(box_left, box_top, box_width, box_height);
                    confidences.push_back(conf_obj);
                    class_ids.push_back(class_id_point_obj.x);
                    kept_with_obj++;
                }
            }
        }

        data += dimensions;
    }

    // std::cout << "kept_with_obj = " << kept_with_obj << std::endl;

    if (!boxes.empty()) {
        return;
    }

    data = reinterpret_cast<float*>(output.data);

    for (int i = 0; i < rows; i++) {
        float cx = data[0];
        float cy = data[1];
        float w = data[2];
        float h = data[3];

        if (w <= 1.0f || h <= 1.0f) {
            data += dimensions;
            continue;
        }

        int num_classes_no_obj = dimensions - 4;
        if (num_classes_no_obj <= 0) {
            data += dimensions;
            continue;
        }

        cv::Mat scores_no_obj(1, num_classes_no_obj, CV_32FC1, data + 4);

        cv::Point class_id_point_no_obj;
        double max_score_no_obj = 0.0;
        cv::minMaxLoc(scores_no_obj, nullptr, &max_score_no_obj, nullptr, &class_id_point_no_obj);

        float conf_no_obj = static_cast<float>(max_score_no_obj);

        if (conf_no_obj > conf_threshold) {
            int box_left = static_cast<int>(std::round(cx - w / 2.0f));
            int box_top = static_cast<int>(std::round(cy - h / 2.0f));
            int box_width = static_cast<int>(std::round(w));
            int box_height = static_cast<int>(std::round(h));

            boxes.emplace_back(box_left, box_top, box_width, box_height);
            confidences.push_back(conf_no_obj);
            class_ids.push_back(class_id_point_no_obj.x);
            kept_no_obj++;
        }

        data += dimensions;
    }

    // std::cout << "kept_no_obj = " << kept_no_obj << std::endl;
}

std::vector<int> YOLO_Detector::applyNMS(const std::vector<cv::Rect>& boxes,
                                         const std::vector<float>& confidences) {
    std::vector<int> indices;

    if (boxes.empty() || confidences.empty()) {
        return indices;
    }

    cv::dnn::NMSBoxes(boxes,
                      confidences,
                      conf_threshold,
                      nms_threshold,
                      indices);

    return indices;
}

void YOLO_Detector::functionTest(const cv::Mat& image) {
    std::vector<Detection> detections = detect(image);

    std::cout << "Detection count: " << detections.size() << std::endl;

    for (const auto& d : detections) {
        std::cout << "class: " << d.class_id
                  << "  symbol: "
                  << (num_ops.count(d.class_id) ? num_ops[d.class_id] : "?")
                  << "  confidence: " << d.confidence
                  << "  box: "
                  << d.box.x << ", " << d.box.y << ", "
                  << d.box.width << ", " << d.box.height
                  << std::endl;
    }
}

// std::string YOLO_Detector::generateExpression(const cv::Mat& image) {
//     std::vector<Detection> detections = detect(image);
//     if (detections.empty()) {
//         return "";
//     }

//     const float expr_conf_threshold = 0.50f;
//     const int min_row_gap = 18;

//     std::vector<Detection> valid;
//     valid.reserve(detections.size());

//     for (const auto& d : detections) {
//         if (d.confidence < expr_conf_threshold) {
//             continue;
//         }
//         if (d.box.width < 5 || d.box.height < 5) {
//             continue;
//         }
//         if (d.box.x >= image.cols || d.box.y >= image.rows) {
//             continue;
//         }
//         if (num_ops.find(d.class_id) == num_ops.end()) {
//             continue;
//         }
//         valid.push_back(d);
//     }

//     if (valid.empty()) {
//         return "";
//     }

//     std::sort(valid.begin(), valid.end(), [min_row_gap](const Detection& a, const Detection& b) {
//         int ay = a.box.y + a.box.height / 2;
//         int by = b.box.y + b.box.height / 2;

//         if (std::abs(ay - by) > min_row_gap) {
//             return ay < by;
//         }

//         int ax = a.box.x + a.box.width / 2;
//         int bx = b.box.x + b.box.width / 2;
//         return ax < bx;
//     });

//     float mean_width = 0.0f;
//     for (const auto& d : valid) {
//         mean_width += static_cast<float>(d.box.width);
//     }
//     mean_width /= static_cast<float>(valid.size());

//     int x_threshold = std::max(10, static_cast<int>(std::round(mean_width * 0.45f)));
//     int y_threshold = std::max(12, static_cast<int>(std::round(mean_width * 0.35f)));

//     std::vector<Detection> filtered;
//     filtered.reserve(valid.size());

//     for (const auto& det : valid) {
//         if (filtered.empty()) {
//             filtered.push_back(det);
//             continue;
//         }

//         Detection& last = filtered.back();

//         int x = det.box.x + det.box.width / 2;
//         int y = det.box.y + det.box.height / 2;
//         int last_x = last.box.x + last.box.width / 2;
//         int last_y = last.box.y + last.box.height / 2;

//         bool same_row = std::abs(y - last_y) < y_threshold;
//         bool same_char = std::abs(x - last_x) < x_threshold && same_row;

//         if (same_char) {
//             if (det.confidence > last.confidence) {
//                 last = det;
//             }
//         } else {
//             filtered.push_back(det);
//         }
//     }

//     if (filtered.empty()) {
//         return "";
//     }

//     std::sort(filtered.begin(), filtered.end(), [y_threshold](const Detection& a, const Detection& b) {
//         int ay = a.box.y + a.box.height / 2;
//         int by = b.box.y + b.box.height / 2;

//         if (std::abs(ay - by) > y_threshold) {
//             return ay < by;
//         }

//         int ax = a.box.x + a.box.width / 2;
//         int bx = b.box.x + b.box.width / 2;
//         return ax < bx;
//     });

//     std::string result;
//     result.reserve(filtered.size());

//     for (const auto& det : filtered) {
//         result += num_ops[det.class_id];
//     }

//     return result;
// }

std::string YOLO_Detector::generateExpression(const cv::Mat& image) {
    std::vector<Detection> detections = detect(image);
    if (detections.empty()) {
        return "";
    }

    const float expr_conf_threshold = 0.50f;

    std::vector<Detection> valid;
    valid.reserve(detections.size());

    for (const auto& d : detections) {
        if (d.confidence < expr_conf_threshold) {
            continue;
        }
        if (d.box.width < 5 || d.box.height < 5) {
            continue;
        }
        if (d.box.x >= image.cols || d.box.y >= image.rows) {
            continue;
        }
        if (num_ops.find(d.class_id) == num_ops.end()) {
            continue;
        }
        valid.push_back(d);
    }

    if (valid.empty()) {
        return "";
    }

    // 只按 x 中心排序
    std::sort(valid.begin(), valid.end(), [](const Detection& a, const Detection& b) {
        int ax = a.box.x + a.box.width / 2;
        int bx = b.box.x + b.box.width / 2;
        return ax < bx;
    });

    // 用平均宽度估计“同一个字符重复框”的 x 阈值
    float mean_width = 0.0f;
    for (const auto& d : valid) {
        mean_width += static_cast<float>(d.box.width);
    }
    mean_width /= static_cast<float>(valid.size());

    int x_threshold = std::max(10, static_cast<int>(std::round(mean_width * 0.45f)));

    std::vector<Detection> filtered;
    filtered.reserve(valid.size());

    for (const auto& det : valid) {
        if (filtered.empty()) {
            filtered.push_back(det);
            continue;
        }

        Detection& last = filtered.back();

        int x = det.box.x + det.box.width / 2;
        int last_x = last.box.x + last.box.width / 2;

        bool same_char = std::abs(x - last_x) < x_threshold;

        if (same_char) {
            if (det.confidence > last.confidence) {
                last = det;
            }
        } else {
            filtered.push_back(det);
        }
    }

    // 再按 x 中心稳定排序一次
    std::sort(filtered.begin(), filtered.end(), [](const Detection& a, const Detection& b) {
        int ax = a.box.x + a.box.width / 2;
        int bx = b.box.x + b.box.width / 2;
        return ax < bx;
    });

    std::string result;
    result.reserve(filtered.size());

    for (const auto& det : filtered) {
        result += num_ops[det.class_id];
    }

    return result;
}

cv::Mat YOLO_Detector::letterbox(const cv::Mat& image,
                                 float& scale,
                                 int& top,
                                 int& left) {
    if (image.empty()) {
        scale = 1.0f;
        top = 0;
        left = 0;
        return cv::Mat();
    }

    int src_width = image.cols;
    int src_height = image.rows;

    scale = std::min(static_cast<float>(input_width) / static_cast<float>(src_width),
                     static_cast<float>(input_height) / static_cast<float>(src_height));

    int resized_width = static_cast<int>(std::round(src_width * scale));
    int resized_height = static_cast<int>(std::round(src_height * scale));

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(resized_width, resized_height));

    int pad_width = input_width - resized_width;
    int pad_height = input_height - resized_height;

    left = pad_width / 2;
    top = pad_height / 2;

    int right = pad_width - left;
    int bottom = pad_height - top;

    cv::Mat padded;
    cv::copyMakeBorder(resized,
                       padded,
                       top,
                       bottom,
                       left,
                       right,
                       cv::BORDER_CONSTANT,
                       cv::Scalar(114, 114, 114));

    return padded;
}
