#include "encoder_handler.h"

EncoderHandler::EncoderHandler(EncoderConfig configDer, EncoderConfig configIzq, RobotPhysics phys) : _phys(phys) {
    _pinDerA = configDer.pinA; _pinDerB = configDer.pinB;
    _pinIzqA = configIzq.pinA; _pinIzqB = configIzq.pinB;

    // Distancia que avanza por cada pulso (metros)
    _metersPerTick = (PI * phys.wheelDiameter) / (phys.ppr * phys.gearRatio * 4.0);
}

void EncoderHandler::begin(){
    // Activar el pull up, para que mientras no detecte nada no se quede flotando. low al detectar.
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    // Configura los pines (Fase A, Fase B). Al poner fullquad tiene en cuenta subida i bajada de los 2 pines, lo que da una resolucion x4.
    _encoderIzq.attachFullQuad(_pinIzqA, _pinIzqB);
    _encoderDer.attachFullQuad(_pinDerA, _pinDerB);
    
    // Ignora pulsos de menos de 100 ciclos de reloj para evitar "high-low-high-low" por ruido (ajustar entre 100 y 500)
    _encoderIzq.setFilter(100); 
    _encoderDer.setFilter(100);

    // Inicialización de tiempos para evitar saltos en el primer update (tiempo de la ultima actualizacion, ticks pasados contados por el encoder)
    _lastUpdateTime = micros();
    _lastTicksIzq = _encoderIzq.getCount();
    _lastTicksDer = _encoderDer.getCount();
}

void EncoderHandler::update() {
    unsigned long currentTime = micros();
    float time_increased = (currentTime - _lastUpdateTime) / 1000000.0; // (segundos)

    if (time_increased >= 0.01) { 

        //ticks totales
        long currentTicksIzq = _encoderIzq.getCount();
        long currentTicksDer = _encoderDer.getCount();

        // numero de ticks en la ronda
        long left_ticks_increased = currentTicksIzq - _lastTicksIzq;
        long right_ticks_increased = currentTicksDer - _lastTicksDer;

        // dist recorrida (numero de ticks * distancia recorreguda en cada tick)
        float left_dist_increased =  left_ticks_increased * _metersPerTick;
        float right_dist_increased = right_ticks_increased * _metersPerTick;

        // Velocidad instantánea (dist recorrida/ tiempo)
        _velIzq = left_dist_increased / time_increased;
        _velDer = right_dist_increased / time_increased;

        // Distancia total (debug)
        // _totaldistIzq += left_dist_increased;
        // _totaldistDer += right_dist_increased;

        _lastTicksIzq = currentTicksIzq;
        _lastTicksDer = currentTicksDer;
        _lastUpdateTime = currentTime;
    }
}

float EncoderHandler::getVelocityIzq() { return _velIzq; }
float EncoderHandler::getVelocityDer() { return _velDer; }
// float EncoderHandler::getDistIzq() { return _totaldistIzq; }
// float EncoderHandler::getDistDer() { return _totaldistDer; }