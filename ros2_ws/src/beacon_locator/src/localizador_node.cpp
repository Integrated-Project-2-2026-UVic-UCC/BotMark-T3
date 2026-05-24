#include "lidar_localization/localizador_node.hpp"

LocalizadorNode::LocalizadorNode() : Node("localizador_node") {
    // 1. Suscribirse al LiDAR
    scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
                "/scan", 
                10, 
                [this](sensor_msgs::msg::LaserScan::SharedPtr msg)
                    {scan_callback(msg);}
                );

    // 2. Publicar nuestra posición (X, Y)
    pose_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("/robot_pose", 10);

    // 3. Publicar puntos que son obstáculos (para verlos en RViz)
    obstaculos_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>("/obstaculos", 10);

    beacons_debug_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>("/scan_beacons_debug", 10);
}

void LocalizadorNode::scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr scan_msg) {
    // Como cuando miras un vídeo de un extranjero en youtube:
    // El tio habla otro idioma (coord polares [LaserScan]), pero tu solo hablas español (coord cartesianas [PointCloud2]). 
    // Entonces usas la traduccion automatica (projectLaser de laser_geometry::LaserProjection) para que te lo traduzca
    // De por si no viene activado la intensidad. El -1.0 es para que te de todo. Si pones 2 te da los valores a partir de 2m.
    sensor_msgs::msg::PointCloud2 cloud_msg;
    projector_.projectLaser(*scan_msg, cloud_msg, -1.0, laser_geometry::channel_option::Intensity); // Puntero para evitar copia de todos los puntos y su lentitud
    
    // cloud es el nombre de la variable
    // pcl::PointCloud -> contendedor
    // pcl::PointXYZ -> contenido
    // new pcl::PointCloud<pcl::PointXYZ> reserva de memoria
    // new reserva la memoria que pidas y te da un puntero (llave a este)[se guarda en cloud]. si lo pierdes (la variable muere) se queda la memoria ocupando sitio no se borra. 
    // ::Ptr crea un "puntero compartido". Mira cuantos usan la llave que le ha dado new, si es 0 borra el contenido. (Evita que se quede memoria vieja por ahi ocupando)
    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>);

    // Como cuando miras un vídeo de un latino explicando algo.
    // Hablais los dos español (PointCloud2) pero cada uno a su manera, haciendo que no os entendais en la mitad de lo que decis.
    // Por eso en el traductor hay la opcion de traducir español España (pcl::PointCloud) y español Latino (sensor_msgs::msg::PointCloud2). 
    // Pues lo mismo, pasas (pcl::fromROSMsg) de español Latino (cloud_msg) a español España (cloud) para entenderlo 
    pcl::fromROSMsg(cloud_msg, *cloud);

    // if (!cloud->empty()) {
    //     RCLCPP_INFO(this->get_logger(), "Intensidad del primer punto: %f", cloud->points[0].intensity);
    // }
    // else{
    //     RCLCPP_INFO(this->get_logger(), "vacio");
    // }

    // Guarda el frame exacto que viene del sensor (ej: "base_laser") para visualizar en rviz2
    std::string frame_del_sensor = scan_msg->header.frame_id;

    procesar_beacons(cloud, frame_del_sensor);
}



void LocalizadorNode::procesar_beacons(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, std::string frame_id) {
    CAMBIAR POR FILTRO DE POCISION YA que la intnsidad varia segun la distancia a la que estes. si sabes la posicion en la que estas  y aprox por donde esta el beacon. puedes filtrar lo que haya ahi como beacon
    Luego se puede añadir un filtro de intensidad (intensidad proporcional a la distancia) que filtre si hay algo al lado.
    Hacer control de si no se detecta algun beacon que se use la de odometria
    // 1. Filtro de Intensidad (Solo lo que brilla)
    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_brillante(new pcl::PointCloud<pcl::PointXYZI>);
    for (const auto& punto : cloud->points) {
        if (punto.intensity > 240.0) {
            cloud_brillante->push_back(punto);
        }
    }

    if (!cloud_brillante->empty()) {

        sensor_msgs::msg::PointCloud2 debug_msg;
        pcl::toROSMsg(*cloud_brillante, debug_msg);
        
        debug_msg.header.frame_id = frame_id;
        debug_msg.header.stamp = this->get_clock()->now();
        
        beacons_debug_pub_->publish(debug_msg);
    }

    // 2. Crear el buscador KdTree para los puntos brillantes (ordenar)
    pcl::search::KdTree<pcl::PointXYZI>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZI>);
    tree->setInputCloud(cloud_brillante);

    // 3. Configurar el Euclidean Clustering (agrupar)
    std::vector<pcl::PointIndices> cluster_indices; // Lista de los indices de los resultados. Se guarda indices para ligereza
    pcl::EuclideanClusterExtraction<pcl::PointXYZI> ec; // El agrupador que contiene toda la lógica para decidir qué puntos van juntos y cuáles no.
    ec.setClusterTolerance(0.05); // Puntos a menos con una distancia menor de 5cm entre ellos forman el mismo objeto
    ec.setMinClusterSize(5);      // Al menos 5 puntos para ser una baliza
    ec.setMaxClusterSize(50);     // Si tiene muchos puntos, es una pared, no una baliza
    ec.setSearchMethod(tree);     // Le pasas el mapa para que sepa el orden y no lo haga a lo loco desordenado
    ec.setInputCloud(cloud_brillante); // Le pasas la nube con los datos
    ec.extract(cluster_indices); // El botón de EJECUTAR. El algoritmo empieza a saltar de punto en punto usando el mapa y llena la lista cluster_indices con los grupos que haya encontrado.

    // 4. Analizar cada grupo con Eigen (calcular centros balizas)
    for (const auto& indices_grupo : cluster_indices) {
        // Extraemos los puntos de este cluster a un objeto Eigen para calcular rápido

        // min_pt pones x,y en un valor absurdamente alto para que en la primera el primer valor pase a ser el nuevo minimo. Igual con el max pero al reves.
        // Como si estuvieras definiendo dos variables en una misma linea -> float num1, num2;
        Eigen::Vector2f min_pt(999, 999), max_pt(-999, -999); 

        for (auto idx : indices_grupo.indices) { // Encontrar punto mas lejano/cercano y punto mas derecha/izquierda. para poder sacar el ancho.
            const auto& p = cloud_brillante->points[idx];
            if (p.x < min_pt.x()) min_pt.x() = p.x;
            if (p.y < min_pt.y()) min_pt.y() = p.y;
            if (p.x > max_pt.x()) max_pt.x() = p.x;
            if (p.y > max_pt.y()) max_pt.y() = p.y;
        }

        // Calculamos el ancho del objeto usando la distancia euclidiana de Eigen
        float ancho = (max_pt - min_pt).norm();

        // Comprobar si es baliza con el ancho dando un margen
        if (ancho > 0.07 && ancho < 0.13) {
            // Encontrar centros.
            float centro_x = (min_pt.x() + max_pt.x()) / 2.0;
            float centro_y = (min_pt.y() + max_pt.y()) / 2.0;
        }
    }
}

int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LocalizadorNode>(); 

    try {
        rclcpp::spin(node);
    }
    catch (const std::exception & e) {
        RCLCPP_ERROR(node->get_logger(), "Error en el ejecutor: %s", e.what());
    }
    
    rclcpp::shutdown();
    return 0;
}