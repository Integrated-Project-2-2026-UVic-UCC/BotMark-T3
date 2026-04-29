#include "zenoh_manager.h"

ZenohManager::ZenohManager() {}

bool ZenohManager::begin(const char* ssid, const char* password, const char* router_ip) {
    // 1. Conectar WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Conectado!");

    // 2. Configurar Zenoh
    z_config_t config = z_config_default();
    z_config_insert(config, Z_CONFIG_CONNECT_KEY, z_string_make(router_ip));

    Serial.println("Abriendo sesión Zenoh...");
    session = z_open(config);

    if (z_session_check(&session)) {
        Serial.println("Sesión Zenoh abierta con éxito.");
        
        // 3. Suscribirse a cmd_vel
        z_owned_closure_sample_t callback = z_closure_sample_make(on_data, nullptr, this);
        z_declare_subscriber(session, z_path_make(_expr_sub), z_move(callback), nullptr);
        
        _connected = true;
        return true;
    }

    Serial.println("Error al abrir sesión Zenoh.");
    return false;
}

void ZenohManager::update() {
    if (_connected) {
        // Zenoh-pico procesa los mensajes entrantes aquí
        // z_session_check o tareas de mantenimiento si fueran necesarias
    }
}

void ZenohManager::publishStatus(float v, float w, float yaw) {
    if (!_connected) return;

    // Creamos un buffer con los datos (v, w, yaw)
    float data[3] = {v, w, yaw};
    
    z_publisher_put_options_t options = z_publisher_put_options_default();
    z_path_t path = z_path_make(_expr_pub);
    
    z_publisher_put(session, path, (const uint8_t*)data, sizeof(data), &options);
}

// Esta función se ejecuta cuando ROS 2 manda un mensaje a "rt/robot/cmd_vel"
void ZenohManager::on_data(const z_sample_t* sample, void* arg) {
    ZenohManager* self = (ZenohManager*)arg;
    
    // Suponemos que recibimos 2 floats (linear.x y angular.z)
    if (sample->payload.len >= sizeof(float) * 2) {
        float* vals = (float*)sample->payload.start;
        self->_last_command.linear_x = vals[0];
        self->_last_command.angular_z = vals[1];
    }
}

TwistCommand ZenohManager::getLastCommand() { return _last_command; }
    
bool ZenohManager::isConnected() { return _connected; }