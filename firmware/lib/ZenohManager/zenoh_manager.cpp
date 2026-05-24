#include "zenoh_manager.h"

ZenohManager::ZenohManager() {}

bool ZenohManager::begin(const char* ssid, const char* password, const char* router_ip) {
    WiFi.mode(WIFI_STA);
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
        Serial.println("\nWiFi Conectado!");
    }

    delay(2000); 

    z_owned_config_t config;
    z_config_default(&config);
    
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, "client");
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, router_ip);

    Serial.println("Abriendo sesión Zenoh...");
    
    if (z_open(&session, z_config_move(&config), NULL) < 0) {
        Serial.println("Error: No se pudo abrir la sesión.");
        return false;
    }

    if (zp_start_read_task(z_session_loan_mut(&session), NULL) < 0 || 
        zp_start_lease_task(z_session_loan_mut(&session), NULL) < 0) {
        Serial.println("Error: No se pudieron iniciar tareas de fondo.");
        z_session_drop(z_session_move(&session));
        return false;
    }

    // Configuración del callback de suscripción
    z_owned_closure_sample_t callback;
    z_closure_sample(&callback, ZenohManager::on_data, NULL, (void*)this);
    
    z_view_keyexpr_t ke_sub, ke_pub;
    
    z_view_keyexpr_from_str_unchecked(&ke_sub, _expr_sub);
    if (z_declare_subscriber(z_session_loan(&session), &sub, z_view_keyexpr_loan(&ke_sub), z_closure_sample_move(&callback), NULL) < 0) {
        Serial.println("Error: No se pudo crear el suscriptor.");
        return false;
    }

    Serial.printf("DEBUG: Declarando pub en: %s\n", _expr_pub);
    z_view_keyexpr_from_str_unchecked(&ke_pub, _expr_pub);
    if (z_declare_publisher(z_session_loan(&session), &pub, z_view_keyexpr_loan(&ke_pub), NULL) < 0) {
        Serial.println("Unable to declare publisher for key expression!");
        while (1) {
            ;
        }
    }

    Serial.println("¡Zenoh 1.0 Conectado!");
    _connected = true;
    return true;
}

void ZenohManager::publishSensors(const SensorData& data) {
    if (!_connected) return;

    // En lugar de copy_from_mem, usamos la forma compatible de inicializar bytes
    z_owned_bytes_t payload;
    // Esta es la función estándar para buffers de memoria en v1.0
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&data, sizeof(SensorData));
    
    if (z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error while publishing data");
        
    }
}

void ZenohManager::on_data(z_loaned_sample_t* sample, void* arg) {
    ZenohManager* self = (ZenohManager*)arg;

    const _z_bytes_t* payload = z_sample_payload(sample);
    size_t len = z_bytes_len(payload);

    if (len == sizeof(CommandData)) {

        uint8_t buffer[sizeof(CommandData)];

        // Guardamos el número real de bytes copiados
        size_t copiados = _z_bytes_to_buf(payload, buffer, len);

        // Comprobamos que ha logrado copiar todos los bytes esperados
        if (copiados == len) {

            memcpy(&self->_last_command, buffer, sizeof(CommandData));
            
            // Confirmación visual con los datos reales
            Serial.printf("¡Comando OK! Lin: %.2f | Ang: %.2f | Stop: %d\n", 
                          self->_last_command.linear_x, 
                          self->_last_command.angular_z, 
                          self->_last_command.emergency_stop);

        } else {
            Serial.printf("Error Zenoh: Solo se copiaron %d de %d bytes\n", 
                          (int)copiados, (int)len);
        }

    } else {
        Serial.printf("Error Zenoh: Tamaño incorrecto. Esperado %u, recibido %u\n",
                      (unsigned int)sizeof(CommandData),
                      (unsigned int)len);
    }
}

CommandData ZenohManager::getLastCommand() { return _last_command; }
bool ZenohManager::isConnected() { return _connected; }