# ESP32-C6 DHT11 Sensor con Mosquitto MQTT en AWS EC2

**Autor:** Edward Fabian Goyeneche Velandia  
**Universidad:** Universidad Nacional de Colombia - Sede Manizales  
**Grupo:** Percepción de Control Inteligente  
**Asignatura:** Desarrollo de Sistemas IoT  
**Período:** 2025-II  

---

Este proyecto implementa un sensor DHT11 en ESP32-C6 DevKitC-1 usando ESP-IDF que se conecta a un broker Mosquitto en Amazon EC2.

##  Características

- **Sensor DHT11**: Lectura de temperatura y humedad en GPIO4
- **Servidor web HTTP**: Interfaz web para monitoreo en tiempo real  
- **Cliente MQTT**: Comunicación con Mosquitto en EC2 con autenticación
- **Integración AWS EC2**: Envío de datos via HTTP y MQTT
- **API REST**: Endpoints para acceso programático a datos

##  Hardware Requerido

- ESP32-C6 DevKitC-1
- Sensor DHT11 (Temperatura y Humedad)
- Cables jumper
- Breadboard (opcional)

## 📊 Conexiones

Ver archivo `DHT11_WIRING.md` para diagrama detallado.

**Resumen de conexiones:**
- DHT11 VCC → ESP32 3.3V
- DHT11 DATA → ESP32 GPIO4  
- DHT11 GND → ESP32 GND

## 🔧 Configuración

### 1. Configurar WiFi y Credenciales
Edita `main/config.h`:
```c
// conexion a WiFi
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Credenciales MQTT (configurar en EC2 primero)
#define MQTT_USERNAME "esp32_user"
#define MQTT_PASSWORD "esp32_pass"
```

### 2. Configurar Mosquitto en EC2
Ver archivo `MQTT_SETUP.md` para instrucciones completas de configuración del broker.

**Tu EC2 ya configurado:**
- Host: `ec2-13-220-220-130.compute-1.amazonaws.com`
- Puerto MQTT: 1883
- Puerto WebSocket: 9001
- Autenticación requerida

##  Construcción y Flash

### Requisitos
- ESP-IDF v5.0 o superior
- Toolchain para ESP32-C6

### Comandos
```bash
# Configurar el proyecto
idf.py set-target esp32c6

# Construir
idf.py build

# Flash y monitor
idf.py flash monitor
```

## 📡 Uso

### Interfaz Web
Accede a `http://[IP_DEL_ESP32]` para ver:
- Datos de sensores en tiempo real
- Estado del sistema
- Controles manuales

### API Endpoints

#### GET /api/sensor
Obtiene los datos actuales del sensor:
```json
{
  "timestamp": 1634567890123,
  "temperature": 25.6,
  "humidity": 60.5,
  "analog_raw": 2048,
  "digital_state": 1,
  "device_id": "ESP32_SENSOR_001"
}
```

#### GET /api/status
Estado del sistema:
```json
{
  "status": "online",
  "uptime_seconds": 3600,
  "free_heap_bytes": 180000,
  "device_id": "ESP32_SENSOR_001"
}
```

#### POST /api/aws-send
Envía datos manualmente a AWS ECS:
```json
{
  "status": "success",
  "message": "Datos enviados a AWS ECS exitosamente"
}
```

### Topics MQTT

#### Publicación (ESP32 → Broker)
- `sensor/data`: Datos del sensor en JSON
- `sensor/status`: Estado del dispositivo

#### Suscripción (Broker → ESP32)
- `sensor/commands`: Comandos remotos
  - `restart`: Reinicia el dispositivo
  - `status`: Solicita envío de datos
- `sensor/config`: Cambios de configuración

##  Arquitectura del Sistema

```
ESP32-C6
├── Sensores (ADC/GPIO)
├── WiFi Connection
├── HTTP Server (Puerto 80)
│   ├── Interfaz Web
│   └── API REST
├── MQTT Client
│   ├── → AWS EC2 Broker
│   └── ← Comandos remotos
└── AWS ECS Integration
    └── HTTP POST a servicios
```

##  Estructura del Proyecto

```
Practice_1_ESP32-C6-with-sensor-and-Web-Server-with-MQTT-and-HTTP-AWS-EC2/
├── main/                       # Código fuente principal
│   ├── main.c                 # Aplicación principal
│   ├── config.h               # Configuraciones del sistema
│   ├── wifi_config.c/h        # Gestión WiFi
│   ├── sensor_manager.c/h     # Gestión del sensor DHT11
│   ├── mqtt_client.c/h        # Cliente MQTT con autenticación
│   ├── http_server.c/h        # Servidor HTTP con SPIFFS
│   └── CMakeLists.txt         # Configuración del componente
├── web/                       # Frontend web (archivos estáticos)
│   ├── index.html             # Página principal responsive
│   ├── css/
│   │   └── styles.css         # Estilos CSS modernos
│   └── js/
│       └── app.js             # Lógica JavaScript
├── partitions.csv             # Tabla de particiones (incluye SPIFFS)
├── CMakeLists.txt             # Configuración principal
├── sdkconfig.defaults         # Configuración por defecto
├── MQTT_SETUP.md             # Guía de configuración MQTT
├── DHT11_WIRING.md           # Diagrama de conexiones
└── README.md                 # Este archivo
```

## 🎨 **Interfaz Web **

La interfaz web ahora está separada en archivos independientes:

### **Frontend :**
- ✅ **HTML5 Semántico**: Estructura clara y accesible
- ✅ **CSS3 Avanzado**: Variables CSS, Grid, Flexbox, animaciones
- ✅ **JavaScript ES6+**: Fetch API, async/await, manejo de errores
- ✅ **Responsive Design**: Funciona en móviles, tablets y desktop
- ✅ **PWA-Ready**: Preparado para Progressive Web App

### **Tecnologías Web Utilizadas:**
- **SPIFFS**: Sistema de archivos embebido para servir archivos estáticos
- **HTTP Server**: Servidor web integrado en ESP32
- **REST API**: Endpoints JSON para comunicación con frontend
- **CSS Grid/Flexbox**: Layout moderno y responsive
- **JavaScript Vanilla**: Sin dependencias externas

## 🔌 Conexiones de Hardware

### Sensor Analógico (ej: LM35)
- VCC → 3.3V
- GND → GND  
- OUT → GPIO0 (ADC1_CH0)


## 🔒 Seguridad

Para producción, considera:
- Usar MQTT con TLS (puerto 8883)
- Autenticación con certificados
- Encriptación de datos sensibles
- Validación de entrada en APIs

## 📝 Notas para AWS ECS

Este proyecto está diseñado para integrarse con servicios en Amazon ECS. El ESP32 puede enviar datos a:

1. **MQTT Broker en EC2**: Para comunicación en tiempo real
2. **HTTP API en ECS**: Para almacenamiento y procesamiento de datos
3. **AWS IoT Core**: Para mayor escalabilidad (requiere certificados)

### Configuración del Lado del Servidor (ECS)

En el  servicio en ECS debería exponer endpoints como:
- `POST /api/sensor-data`: Para recibir datos del ESP32
- `GET /api/devices`: Para listar dispositivos conectados
- `POST /api/commands`: Para enviar comandos a dispositivos específicos

