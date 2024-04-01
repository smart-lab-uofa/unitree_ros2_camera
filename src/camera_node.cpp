#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>

class CameraNode : public rclcpp::Node {
public:
    CameraNode() : Node("camera_node") {
        publisher_ = this->create_publisher<sensor_msgs::msg::Image>("camera_image", 10);
    }

    void publishImage(const cv::Mat &frame) {
        auto message = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
        publisher_->publish(*message);
    }

private:
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraNode>();

    std::string IpLastSegment = "100";
    int cam = 1;
    if (argc >= 2) {
        cam = std::atoi(argv[1]);
    }
    std::string udpstrPrevData = "udpsrc address=192.168.123." + IpLastSegment + " port=";
    std::array<int, 5> udpPORT = {9201, 9202, 9203, 9204, 9205};
    std::string udpstrBehindData = " ! application/x-rtp,media=video,encoding-name=H264 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink";
    std::string udpSendIntegratedPipe = udpstrPrevData + std::to_string(udpPORT[cam - 1]) + udpstrBehindData;

    cv::VideoCapture cap(udpSendIntegratedPipe);
    if (!cap.isOpened()) {
        return 0;
    }

    cv::Mat frame;
    while (rclcpp::ok()) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        node->publishImage(frame);
        rclcpp::spin_some(node);
    }

    cap.release();
    rclcpp::shutdown();
    return 0;
}
