# Conexiones DHT11 con ESP32-C6 DevKitC-1

**Autor:** Edward Fabian Goyeneche Velandia  
**Universidad:** Universidad Nacional de Colombia - Sede Manizales  
**Grupo:** Percepción de Control Inteligente  
**Asignatura:** Desarrollo de Sistemas IoT  
**Período:** 2025-II  

---

##   Diagrama de Conexión

```
DHT11 Sensor          ESP32-C6 DevKitC-1
┌─────────────┐       ┌──────────────────┐
│     VCC     │ ───── │ 3.3V (Pin 1)     │
│     DATA    │ ───── │ GPIO4 (Pin 5)    │
│     NC      │       │                  │
│     GND     │ ───── │ GND (Pin 2)      │
└─────────────┘       └──────────────────┘
```

## 📍 Detalles de Conexión

### DHT11 (4 pines)
- **Pin 1 (VCC)**: Conectar a **3.3V** del ESP32-C6
- **Pin 2 (DATA)**: Conectar a **GPIO4** del ESP32-C6  
- **Pin 3 (GND)**: Conectar a **GND** del ESP32-C6

### ESP32-C6 DevKitC-1 (Pines utilizados)
- **3.3V**: Alimentación para el DHT11
- **GND**: Tierra común
- **GPIO4**: Pin de datos del DHT11 (configurado con pull-up interno)

##   Características del DHT11

- **Voltaje de operación**: 3.3V - 5.5V
- **Corriente**: 0.3mA (midiendo) - 60μA (standby)
- **Rango de temperatura**: 0°C a 50°C (±2°C precisión)
- **Rango de humedad**: 20% a 90% RH (±5% RH precisión)
- **Tiempo de muestreo**: 1 segundo (mínimo)
- **Protocolo**: One-Wire digital

##  Configuración en el Código

El pin se configura en `config.h`:
```c
#define DHT11_GPIO_PIN GPIO_NUM_4
```

##   Tener en cuenta

1. **Pull-up interno**: El ESP32-C6 tiene pull-up interno activado en GPIO4
2. **Timing crítico**: El DHT11 requiere timing preciso, se usan interrupciones deshabilitadas durante la lectura
3. **Intervalo mínimo**: DHT11 necesita al menos 1 segundo entre lecturas
4. **Checksum**: El código verifica la integridad de los datos recibidos

5.  Se puede validar el funcionamiento correcto si se ve esto:
```
DHT11 - Temp: 25.0°C, Hum: 60.0%
```

Y en la interfaz web los valores se actualizarán cada 2 segundos.
