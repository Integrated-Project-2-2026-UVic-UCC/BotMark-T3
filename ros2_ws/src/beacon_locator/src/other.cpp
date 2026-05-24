// void LocalizadorNode::detectar_obstaculos(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud) {
// bool emergencia_freno = false;
//     float ancho_robot_medio = 0.20; 
//     float distancia_seguridad = 0.60; 

//     // En cloud no estan por orden, entonces KdTree hace un mapa de los indice donde los relaciona i ordena. Para luego poder buscar al rededor
//     pcl::search::KdTree<pcl::PointXYZI>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZI>); 
//     tree->setInputCloud(cloud); // Crea el mapa / arbol

//     for (const auto& punto : cloud->points) { // como hacer en python for i in lista. punto es i. points es la lista de dentro de cloud
//         // Descarte rápido por geometría
//         if (punto.x < 0.0 || punto.x > distancia_seguridad || std::abs(punto.y) > ancho_robot_medio) {
//             continue;
//         }

//         // FILTRO DE SOLIDEZ DIRECTO
//         std::vector<int> indices; // Los números de índice de los puntos que están cerca
//         std::vector<float> distancias; // A cuántos metros está cada uno de esos puntos.

//         // Buscamos en un radio un poco mayor (10cm) 
//         // Si hay más de 5 puntos no es ruido
//         if (tree->radiusSearch(punto, 0.10, indices, distancias) > 5) {
//             emergencia_freno = true;
//             break; // ¡Encontrado! No hace falta mirar nada más.
//         }
//     }

//     if (emergencia_freno) this->parar_robot();
// }

// void LocalizadorNode::ver_obstaculos_rviz2(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud) {
//     pcl::PointCloud<pcl::PointXYZI>::Ptr obstaculos(new pcl::PointCloud<pcl::PointXYZI>);

//     pcl::search::KdTree<pcl::PointXYZI>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZI>);
//     tree->setInputCloud(cloud);

//     for (const auto& punto : cloud->points) {
//         if (punto.x < 0.0 || punto.x > distancia_seguridad) {
//             continue;
//         }

//         B. FILTRO DE RUIDO (Radius Outlier):
//         Buscamos cuántos vecinos tiene este punto en un radio de 10cm
//         std::vector<int> indices_vecinos;
//         std::vector<float> distancias_vecinos;

//         Si tiene más de 3 vecinos cerca, no es ruido, es un objeto sólido
//         if (tree->radiusSearch(punto, 0.10, indices_vecinos, distancias_vecinos) > 5) {
//             obstaculos->push_back(punto); // Guarda los puntos en la nueva nube de puntos
//         }
//     }

//     3. Publicar la nube filtrada
//     if (!obstaculos->empty()) {
//         sensor_msgs::msg::PointCloud2 output_msg;
//         pcl::toROSMsg(*obstaculos, output_msg);
//         output_msg.header.stamp = this->get_clock()->now(); // Sello de tiempo actual
//         output_msg.header.frame_id = "laser_frame";
//         obstaculos_pub_->publish(output_msg);
//     }
// }