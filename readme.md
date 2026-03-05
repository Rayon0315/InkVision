# InkVision

![](/logo.jpg)

InkVision 是一个基于 **Qt + C++ + OpenCV** 的手写数字识别桌面应用。
 用户可以在画板上手写数字，系统会使用 **ONNX 模型**进行推理，并实时显示预测结果和各类别概率。

该项目主要用于探索 **C++ 桌面 GUI 与深度学习推理的结合**。

------

## Features

- 手写数字绘制画板
- MNIST 风格图像预处理
- ONNX 模型推理（OpenCV DNN）
- 实时预测结果显示
- 概率柱状图可视化

------

## Tech Stack

- C++
- Qt
- OpenCV
- ONNX

------

## Model

当前使用 **MNIST 分类模型（10 类数字）**。
 只需保持输入为 `1×1×28×28` 即可替换为其他 ONNX 模型。