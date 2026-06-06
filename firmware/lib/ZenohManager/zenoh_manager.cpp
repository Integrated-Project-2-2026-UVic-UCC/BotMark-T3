#include "zenoh_manager.h"

ZenohManager::ZenohManager() {}

bool ZenohManager::begin(const char* router_ip, String ssid, String password) {
    WiFi.mode(WIFI_STA);
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Conectando a WiFi");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) { 
            delay(500); 
            Serial.print("."); 
        }
        Serial.println("\nWiFi Conectado!");
        delay(2000); 
    }

    z_owned_config_t config;
    z_config_default(&config);
    
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, "client");
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, router_ip);

    if (z_open(&session, z_config_move(&config), NULL) < 0) {
        return false;
    }

    if (zp_start_read_task(z_session_loan_mut(&session), NULL) < 0 || 
        zp_start_lease_task(z_session_loan_mut(&session), NULL) < 0) {
        z_session_drop(z_session_move(&session));
        return false;
    }

    // --- CONFIGURACIÓN DE SUSCRIPTORES ---
    z_view_keyexpr_t ke;
    
    // 1. Suscriptor Comandos (Twist)
    z_owned_closure_sample_t cb_cmd;
    z_closure_sample(&cb_cmd, ZenohManager::on_twist, NULL, (void*)this);
    z_view_keyexpr_from_str_unchecked(&ke, _expr_sub_cmd);
    if (z_declare_subscriber(z_session_loan(&session), &sub_cmd, z_view_keyexpr_loan(&ke), z_closure_sample_move(&cb_cmd), NULL) < 0) {
        return false;
    }

    // 2. Suscriptor Obstáculos
    z_owned_closure_sample_t cb_obs;
    z_closure_sample(&cb_obs, ZenohManager::on_obstacle, NULL, (void*)this);
    z_view_keyexpr_from_str_unchecked(&ke, _expr_sub_obstacle);
    if (z_declare_subscriber(z_session_loan(&session), &sub_obstacle, z_view_keyexpr_loan(&ke), z_closure_sample_move(&cb_obs), NULL) < 0) {
        return false;
    }

    // --- CONFIGURACIÓN DE PUBLICADORES ---
    
    // 1. Publicador Sensores
    z_view_keyexpr_from_str_unchecked(&ke, _expr_pub_sensors);
    if (z_declare_publisher(z_session_loan(&session), &pub_sensors, z_view_keyexpr_loan(&ke), NULL) < 0) return false;

    // 2. Publicador Pausa
    z_view_keyexpr_from_str_unchecked(&ke, _expr_pub_paused);
    if (z_declare_publisher(z_session_loan(&session), &pub_paused, z_view_keyexpr_loan(&ke), NULL) < 0) return false;

    // 3. Publicador Stop
    z_view_keyexpr_from_str_unchecked(&ke, _expr_pub_stoped);
    if (z_declare_publisher(z_session_loan(&session), &pub_stoped, z_view_keyexpr_loan(&ke), NULL) < 0) return false;

    _connected = true;
    return true;
}

// --- MÉTODOS DE PUBLICACIÓN ---

void ZenohManager::publishSensors(const SensorData& data) {
    if (!_connected) return;
    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&data, sizeof(SensorData));
    if (z_publisher_put(z_publisher_loan(&pub_sensors), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error while publishing sensor data");
    }
}

void ZenohManager::publishPaused(bool paused) {
    if (!_connected) return;
    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&paused, sizeof(bool));
    if (z_publisher_put(z_publisher_loan(&pub_paused), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error publishing paused state");
    }
}

void ZenohManager::publishStopped(bool stopped) {
    if (!_connected) return;
    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&stopped, sizeof(bool));
    if (z_publisher_put(z_publisher_loan(&pub_stoped), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error publishing stopped state");
    }
}


// --- CALLBACKS DE RECEPCIÓN ---

void ZenohManager::on_twist(z_loaned_sample_t* sample, void* arg) {
    ZenohManager* self = (ZenohManager*)arg;

    const _z_bytes_t* payload = z_sample_payload(sample);
    size_t len = z_bytes_len(payload);

    if (len == sizeof(Twist)) {
        uint8_t buffer[sizeof(Twist)];
        size_t copiados = _z_bytes_to_buf(payload, buffer, len);

        if (copiados == len) {
            memcpy(&self->_last_command, buffer, sizeof(Twist));
            Serial.printf("¡Comando OK! Lin: %.2f | Ang: %.2f\n", 
                          self->_last_command.linear_x, 
                          self->_last_command.angular_z);
        } else {
            Serial.printf("Error Zenoh CMD: Solo se copiaron %d de %d bytes\n", (int)copiados, (int)len);
        }
    } else {
        Serial.printf("Error Zenoh CMD: Tamaño incorrecto. Esperado %u, recibido %u\n",
                      (unsigned int)sizeof(Twist), (unsigned int)len);
    }
}

void ZenohManager::on_obstacle(z_loaned_sample_t* sample, void* arg) {
    ZenohManager* self = (ZenohManager*)arg;

    const _z_bytes_t* payload = z_sample_payload(sample);
    size_t len = z_bytes_len(payload);

    if (len == sizeof(bool)) {
        bool val;
        size_t copiados = _z_bytes_to_buf(payload, (uint8_t*)&val, len);
        if (copiados == len) {
            self->_obstacle_detected = val;
            Serial.printf("¡Obstáculo Actualizado! Estado: %d\n", val);
        }
    } else {
        Serial.printf("Error Zenoh OBS: Tamaño incorrecto. Recibido %u\n", (unsigned int)len);
    }
}

// --- MÉTODOS DE CONSULTA ---
Twist ZenohManager::getLastCommand() { return _last_command; }
bool ZenohManager::isObstacleDetected() { return _obstacle_detected; }
bool ZenohManager::isConnected() { return _connected; }