#include "accelerometer/gui/fmain.h"

#include <QApplication>

#include <rclcpp/rclcpp.hpp>

int main(int argc, char *argv[])
{
    // Initialize ROS 2
    rclcpp::init(argc, argv);

    QApplication a(argc, argv);
    fmain w;
    w.show();
    int result = a.exec();

    rclcpp::shutdown();
    return result;
}
