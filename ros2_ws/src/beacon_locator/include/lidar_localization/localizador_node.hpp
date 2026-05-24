#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
// Nuevo Include para visualización
#include "visualization_msgs/msg/marker_array.hpp"

#include "laser_geometry/laser_geometry.hpp"
#include <pcl_conversions/pcl_conversions.h>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/search/kdtree.h>
#include <pcl/segmentation/extract_clusters.h>
#include <Eigen/Dense>
#include <vector>

class LocalizadorNode : public rclcpp::Node { // Cambiado a LocalizadorNode para coincidir con tu .cpp
    public:
        LocalizadorNode();

    private:
        void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr scan_msg);
        
        // Cambiado a PointXYZI para poder usar .intensity como en tu código
        void procesar_beacons(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, std::string frame_id);
        void detectar_obstaculos(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud);

        // --- MÉTODO DE APOYO PARA VERIFICACIÓN ---
        // Recibe los centros calculados y los manda a RViz
        void visualizar_detecciones(const std::vector<Eigen::Vector2f>& centros, std::string frame_id);

        rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
        rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
        rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr obstaculos_pub_;
        rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr beacons_debug_pub_;
        
        // Publisher para los marcadores de RViz
        rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;

        laser_geometry::LaserProjection projector_;
};