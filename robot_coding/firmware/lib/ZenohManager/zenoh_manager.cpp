// Implementation of the ZenohManager library for handling Zenoh-Pico network 
// communication, including publisher and subscriber node declarations.
#include "zenoh_manager.h"

ZenohManager::ZenohManager() {}

bool ZenohManager::begin(const char* router_ip, String ssid, String password) {
    WiFi.mode(WIFI_STA);
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting to WiFi");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) { 
            delay(500); 
            Serial.print("."); 
        }
        Serial.println("\nWiFi Connected");
        delay(2000); 
    }

    z_owned_config_t config;
    z_config_default(&config);
    
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, "client");
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, router_ip);

    if (z_open(&_session, z_config_move(&config), NULL) < 0) {
        return false;
    }

    if (zp_start_read_task(z_session_loan_mut(&_session), NULL) < 0 || 
        zp_start_lease_task(z_session_loan_mut(&_session), NULL) < 0) {
        z_session_drop(z_session_move(&_session));
        return false;
    }

    // SUBSCRIBER CONFIGURATION
    z_view_keyexpr_t ke;
    
    // 1. Command Subscriber (Twist)
    z_owned_closure_sample_t cb_cmd;
    z_closure_sample(&cb_cmd, ZenohManager::onTwist, NULL, (void*)this);
    z_view_keyexpr_from_str_unchecked(&ke, EXPR_SUB_CMD);
    if (z_declare_subscriber(z_session_loan(&_session), &_sub_cmd, z_view_keyexpr_loan(&ke), z_closure_sample_move(&cb_cmd), NULL) < 0) {
        return false;
    }

    // 2. Obstacle Subscriber
    z_owned_closure_sample_t cb_obs;
    z_closure_sample(&cb_obs, ZenohManager::onObstacle, NULL, (void*)this);
    z_view_keyexpr_from_str_unchecked(&ke, EXPR_SUB_OBSTACLE);
    if (z_declare_subscriber(z_session_loan(&_session), &_sub_obstacle, z_view_keyexpr_loan(&ke), z_closure_sample_move(&cb_obs), NULL) < 0) {
        return false;
    }

    // PUBLISHER CONFIGURATION
    
    // 1. Sensors Publisher
    z_view_keyexpr_from_str_unchecked(&ke, EXPR_PUB_SENSORS);
    if (z_declare_publisher(z_session_loan(&_session), &_pub_sensors, z_view_keyexpr_loan(&ke), NULL) < 0) return false;

    // 2. Pause Publisher
    z_view_keyexpr_from_str_unchecked(&ke, EXPR_PUB_PAUSED);
    if (z_declare_publisher(z_session_loan(&_session), &_pub_paused, z_view_keyexpr_loan(&ke), NULL) < 0) return false;

    // 3. Stop Publisher
    z_view_keyexpr_from_str_unchecked(&ke, EXPR_PUB_STOPPED);
    if (z_declare_publisher(z_session_loan(&_session), &_pub_stopped, z_view_keyexpr_loan(&ke), NULL) < 0) return false;

    _connected = true;
    return true;
}

// PUBLISHING METHODS

void ZenohManager::publishSensors(const SensorData& data) {
    if (!_connected) return;
    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&data, sizeof(SensorData));
    if (z_publisher_put(z_publisher_loan(&_pub_sensors), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error while publishing sensor data");
    }
}

void ZenohManager::publishPaused(bool paused) {
    if (!_connected) return;
    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&paused, sizeof(bool));
    if (z_publisher_put(z_publisher_loan(&_pub_paused), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error publishing paused state");
    }
}

void ZenohManager::publishStopped(bool stopped) {
    if (!_connected) return;
    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, (const uint8_t*)&stopped, sizeof(bool));
    if (z_publisher_put(z_publisher_loan(&_pub_stopped), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error publishing stopped state");
    }
}

// RECEPTION CALLBACKS

void ZenohManager::onTwist(z_loaned_sample_t* sample, void* arg) {
    ZenohManager* self = (ZenohManager*)arg;

    const _z_bytes_t* payload = z_sample_payload(sample);
    size_t len = z_bytes_len(payload);

    if (len == sizeof(Twist)) {
        uint8_t buffer[sizeof(Twist)];
        size_t copied_bytes = _z_bytes_to_buf(payload, buffer, len);

        if (copied_bytes == len) {
            memcpy(&self->_last_command, buffer, sizeof(Twist));
        } else {
            Serial.printf("Zenoh CMD Error: Only copied %d of %d bytes\n", (int)copied_bytes, (int)len);
        }
    } else {
        Serial.printf("Zenoh CMD Error: Incorrect size. Expected %u, received %u\n",
                      (unsigned int)sizeof(Twist), (unsigned int)len);
    }
}

void ZenohManager::onObstacle(z_loaned_sample_t* sample, void* arg) {
    ZenohManager* self = (ZenohManager*)arg;

    const _z_bytes_t* payload = z_sample_payload(sample);
    size_t len = z_bytes_len(payload);

    if (len == sizeof(bool)) {
        bool val;
        size_t copied_bytes = _z_bytes_to_buf(payload, (uint8_t*)&val, len);
        if (copied_bytes == len) {
            self->_obstacle_detected = val;
        }
    } else {
        Serial.printf("Zenoh OBS Error: Incorrect size. Received %u\n", (unsigned int)len);
    }
}

// QUERY METHODS
Twist ZenohManager::getLastCommand() { return _last_command; }
bool ZenohManager::isObstacleDetected() { return _obstacle_detected; }
bool ZenohManager::isConnected() { return _connected; }