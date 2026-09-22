# Future training work

Reserved for a separate Python/PyTorch environment, dataset tools, training, and
ONNX export. There is no training code or Python dependency in this scaffold.

Before C++ inference, specify input shape and layout (NCHW/NHWC), BGR/RGB order,
resize/letterbox mapping, normalization, precision, output tensors, box coordinates,
labels, confidence thresholds, and NMS ownership. Verify Python and C++ preprocessing
and outputs against the same fixed image. Select OpenCV DNN or ONNX Runtime when
there is a real model to evaluate.
