#include "magnetometer/data/data_interface.h"



#include <rclcpp/serialization.hpp>

using namespace magnetometer;

// CONSTRUCTORS
data_interface::data_interface(std::shared_ptr<rclcpp::Node> node)
{
    m_node = node;
    // Initialize flags.
    data_interface::f_subscriber_enabled = false;

    // Load parameters.
    m_node->declare_parameter("max_data_rate", 1000.0);
    data_interface::p_max_data_rate = m_node->get_parameter("max_data_rate").as_double();
}
data_interface::~data_interface()
{
    // Stop the subscriber if it's running.
    data_interface::stop_subscriber();
}

// DATA SUBSCRIBER
void data_interface::start_subscriber()
{
    if(!data_interface::f_subscriber_enabled)
    {
        data_interface::m_subscriber = m_node->create_subscription<sensor_msgs::msg::MagneticField>(
            "~/magnetometer", 100, std::bind(&data_interface::subscriber, this, std::placeholders::_1));
        data_interface::m_data_timer.start();
        data_interface::f_subscriber_enabled = true;
    }
}
void data_interface::stop_subscriber()
{
    if(data_interface::f_subscriber_enabled)
    {
        data_interface::m_subscriber.reset();
        data_interface::m_data_timer.invalidate();
        data_interface::f_subscriber_enabled = false;
    }
}

// DATA FILE IO
bool data_interface::save_data(std::string& bag_file) const
{
    // Write file.
    try
    {
        // Open the bag file for writing.
        rosbag2_cpp::Writer writer;
        writer.open(bag_file);

        // Iterate over points.
        sensor_msgs::msg::MagneticField message;
        for(uint32_t i = 0; i < data_interface::m_x.size(); ++i)
        {
            // Populate message.
            message.magnetic_field.x = data_interface::m_x.at(i);
            message.magnetic_field.y = data_interface::m_y.at(i);
            message.magnetic_field.z = data_interface::m_z.at(i);

            // Write message to bag.
            writer.write(message, "/imu/magnetometer", m_node->now());
        }

        return true;
    }
    catch(std::exception& e)
    {
        RCLCPP_ERROR_STREAM(m_node->get_logger(), "error writing data to bag file (" << e.what() << ")");
        return false;
    }
}
bool data_interface::load_data(std::string& bag_file)
{
    // Clear existing data.
    data_interface::clear_data();

    // Read file.
    try
    {
        // Open the bag file for reading.
        rosbag2_cpp::Reader reader;
        reader.open(bag_file);

        // Serialization helper
        rclcpp::Serialization<sensor_msgs::msg::MagneticField> serialization;

        // Iterate through view.
        while(reader.has_next())
        {
            auto bag_message = reader.read_next();

            // Check topic
            if(bag_message->topic_name == "/imu/magnetometer")
            {
                 sensor_msgs::msg::MagneticField message;
                 rclcpp::SerializedMessage extracted_serialized_msg(*bag_message->serialized_data);
                 serialization.deserialize_message(&extracted_serialized_msg, &message);

                 data_interface::m_x.push_back(message.magnetic_field.x);
                 data_interface::m_y.push_back(message.magnetic_field.y);
                 data_interface::m_z.push_back(message.magnetic_field.z);
            }
        }

        // Emit updated signal.
        emit data_interface::data_updated();

        return true;
    }
    catch(std::exception& e)
    {
        RCLCPP_ERROR_STREAM(m_node->get_logger(), "error reading from bag file (" << e.what() << ")");
        return false;
    }
}

// DATA MANAGER
void data_interface::clear_data()
{
    // Clear data vectors.
    data_interface::m_x.clear();
    data_interface::m_y.clear();
    data_interface::m_z.clear();

    // Emit updated signal.
    emit data_interface::data_updated();
}

// DATA ACCESS
uint32_t data_interface::n_points()
{
    return data_interface::m_x.size();
}
bool data_interface::get_point(uint32_t index, Eigen::Vector3d& point)
{
    if(index < data_interface::m_x.size())
    {
        point(0) = data_interface::m_x.at(index);
        point(1) = data_interface::m_y.at(index);
        point(2) = data_interface::m_z.at(index);

        return true;
    }
    else
    {
        return false;
    }
}
bool data_interface::get_point(uint32_t index, QVector3D& point)
{
    if(index < data_interface::m_x.size())
    {
        point.setX(data_interface::m_x.at(index));
        point.setY(data_interface::m_z.at(index));
        point.setZ(data_interface::m_y.at(index));

        return true;
    }
    else
    {
        return false;
    }
}

// DATA SUBSCRIBER
void data_interface::subscriber(const sensor_msgs::msg::MagneticField::SharedPtr message)
{
    // Enforce max data rate.
    if(data_interface::m_data_timer.elapsed() >= 1000.0/data_interface::p_max_data_rate)
    {
        // Reset data timer.
        data_interface::m_data_timer.restart();

        // Capture point.
        data_interface::m_x.push_back(message->magnetic_field.x);
        data_interface::m_y.push_back(message->magnetic_field.y);
        data_interface::m_z.push_back(message->magnetic_field.z);

        // Raise signal.
        emit data_interface::data_updated();
    }
}
