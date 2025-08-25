/*
 * JavaScript para ESP32-C6 DHT11 Sensor Monitor
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 */

// Configuración global
const CONFIG = {
    REFRESH_INTERVAL: 5000, // 5 segundos
    API_TIMEOUT: 10000,     // 10 segundos
    MAX_RETRIES: 3,
    RETRY_DELAY: 1000       // 1 segundo
};

// Estado global de la aplicación
let appState = {
    isConnected: true,
    lastUpdate: null,
    retryCount: 0,
    autoRefresh: true,
    intervalId: null
};

// Elementos DOM
const elements = {
    temperature: document.getElementById('temperature'),
    humidity: document.getElementById('humidity'),
    sensorValid: document.getElementById('sensor_valid'),
    errorCount: document.getElementById('error_count'),
    timestamp: document.getElementById('timestamp'),
    connectionStatus: document.getElementById('connection-status'),
    loadingOverlay: document.getElementById('loading-overlay'),
    notification: document.getElementById('notification')
};

/**
 * Inicializa la aplicación cuando el DOM está listo
 */
document.addEventListener('DOMContentLoaded', function() {
    console.log('🚀 Iniciando ESP32-C6 DHT11 Monitor Web App');
    console.log('👨‍💻 Autor: Edward Fabian Goyeneche Velandia');
    console.log('🏛️ Universidad Nacional de Colombia - Sede Manizales');
    
    initializeApp();
});

/**
 * Inicializa la aplicación
 */
function initializeApp() {
    // Cargar datos iniciales
    refreshData();
    
    // Configurar actualización automática
    startAutoRefresh();
    
    // Configurar event listeners
    setupEventListeners();
    
    // Mostrar información de inicialización
    showNotification('Sistema iniciado correctamente', 'success');
}

/**
 * Configura los event listeners
 */
function setupEventListeners() {
    // Detectar cuando la ventana pierde/gana foco para pausar/reanudar actualizaciones
    document.addEventListener('visibilitychange', function() {
        if (document.hidden) {
            stopAutoRefresh();
        } else {
            startAutoRefresh();
            refreshData(); // Actualizar inmediatamente al volver al foco
        }
    });
    
    // Manejar errores de red
    window.addEventListener('online', function() {
        showNotification('Conexión restaurada', 'success');
        appState.isConnected = true;
        updateConnectionStatus(true);
        refreshData();
    });
    
    window.addEventListener('offline', function() {
        showNotification('Sin conexión a internet', 'error');
        appState.isConnected = false;
        updateConnectionStatus(false);
    });
}

/**
 * Actualiza los datos del sensor
 */
async function refreshData() {
    if (!appState.isConnected) {
        console.warn('Sin conexión, saltando actualización');
        return;
    }

    try {
        showLoading(true);
        
        const data = await fetchWithRetry('/api/sensor');
        
        if (data) {
            updateSensorDisplay(data);
            appState.retryCount = 0;
            updateConnectionStatus(true);
            appState.lastUpdate = new Date();
        }
        
    } catch (error) {
        console.error('Error actualizando datos:', error);
        handleFetchError(error);
    } finally {
        showLoading(false);
    }
}

/**
 * Realiza una petición HTTP con reintentos
 */
async function fetchWithRetry(url, options = {}) {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), CONFIG.API_TIMEOUT);
    
    try {
        const response = await fetch(url, {
            ...options,
            signal: controller.signal,
            headers: {
                'Content-Type': 'application/json',
                ...options.headers
            }
        });
        
        clearTimeout(timeoutId);
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        
        return await response.json();
        
    } catch (error) {
        clearTimeout(timeoutId);
        
        if (appState.retryCount < CONFIG.MAX_RETRIES) {
            appState.retryCount++;
            console.log(`Reintento ${appState.retryCount}/${CONFIG.MAX_RETRIES}`);
            
            await new Promise(resolve => setTimeout(resolve, CONFIG.RETRY_DELAY));
            return fetchWithRetry(url, options);
        }
        
        throw error;
    }
}

/**
 * Actualiza la visualización de los datos del sensor
 */
function updateSensorDisplay(data) {
    // Agregar clase de actualización para efecto visual
    Object.values(elements).forEach(el => {
        if (el && el.classList) {
            el.classList.add('updating');
        }
    });
    
    setTimeout(() => {
        // Actualizar temperatura
        if (elements.temperature) {
            elements.temperature.textContent = `${data.temperature.toFixed(1)}°C`;
            elements.temperature.parentElement.classList.add('updated');
        }
        
        // Actualizar humedad
        if (elements.humidity) {
            elements.humidity.textContent = `${data.humidity.toFixed(1)}%`;
            elements.humidity.parentElement.classList.add('updated');
        }
        
        // Actualizar estado del sensor
        if (elements.sensorValid) {
            const isValid = data.sensor_valid;
            elements.sensorValid.textContent = isValid ? 'SÍ' : 'NO';
            elements.sensorValid.className = `value ${isValid ? 'valid' : 'invalid'}`;
            elements.sensorValid.parentElement.classList.add('updated');
        }
        
        // Actualizar contador de errores
        if (elements.errorCount) {
            elements.errorCount.textContent = data.error_count || 0;
            elements.errorCount.parentElement.classList.add('updated');
            
            // Cambiar color según errores
            if (data.error_count > 5) {
                elements.errorCount.style.color = 'var(--danger-color)';
            } else if (data.error_count > 0) {
                elements.errorCount.style.color = 'var(--warning-color)';
            } else {
                elements.errorCount.style.color = 'var(--secondary-color)';
            }
        }
        
        // Actualizar timestamp
        if (elements.timestamp) {
            const date = new Date(data.timestamp);
            elements.timestamp.textContent = date.toLocaleString('es-CO', {
                year: 'numeric',
                month: '2-digit',
                day: '2-digit',
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
            elements.timestamp.parentElement.classList.add('updated');
        }
        
        // Remover clases de actualización
        Object.values(elements).forEach(el => {
            if (el && el.classList) {
                el.classList.remove('updating');
                setTimeout(() => el.classList.remove('updated'), 500);
            }
        });
        
        console.log('📊 Datos actualizados:', {
            temperatura: data.temperature,
            humedad: data.humidity,
            valido: data.sensor_valid,
            errores: data.error_count
        });
        
    }, 200); // Pequeño delay para el efecto visual
}

/**
 * Envía datos manualmente a AWS EC2
 */
async function sendToAWS() {
    try {
        showLoading(true);
        showNotification('Enviando datos a EC2...', 'warning');
        
        const response = await fetchWithRetry('/api/aws-send', {
            method: 'POST'
        });
        
        if (response.status === 'success') {
            showNotification('✅ Datos enviados a EC2 exitosamente', 'success');
        } else {
            throw new Error(response.message || 'Error desconocido');
        }
        
    } catch (error) {
        console.error('Error enviando a AWS:', error);
        showNotification('❌ Error enviando datos a EC2', 'error');
    } finally {
        showLoading(false);
    }
}

/**
 * Inicia la actualización automática
 */
function startAutoRefresh() {
    if (appState.intervalId) {
        clearInterval(appState.intervalId);
    }
    
    appState.autoRefresh = true;
    appState.intervalId = setInterval(refreshData, CONFIG.REFRESH_INTERVAL);
    
    console.log(`🔄 Actualización automática iniciada (cada ${CONFIG.REFRESH_INTERVAL/1000}s)`);
}

/**
 * Detiene la actualización automática
 */
function stopAutoRefresh() {
    if (appState.intervalId) {
        clearInterval(appState.intervalId);
        appState.intervalId = null;
    }
    
    appState.autoRefresh = false;
    console.log('⏸️ Actualización automática pausada');
}

/**
 * Actualiza el indicador de estado de conexión
 */
function updateConnectionStatus(isConnected) {
    if (!elements.connectionStatus) return;
    
    const statusDot = elements.connectionStatus.querySelector('.status-dot');
    const statusText = elements.connectionStatus.querySelector('span:last-child');
    
    if (isConnected) {
        statusDot.className = 'status-dot online';
        statusText.textContent = 'Sistema Online - DHT11 en GPIO4 - Enviando a Mosquitto EC2';
        elements.connectionStatus.style.background = '#e8f5e8';
        elements.connectionStatus.style.borderColor = '#c3e6cb';
        elements.connectionStatus.style.color = '#155724';
    } else {
        statusDot.className = 'status-dot offline';
        statusText.textContent = 'Sistema Offline - Verificando conexión...';
        elements.connectionStatus.style.background = '#f8d7da';
        elements.connectionStatus.style.borderColor = '#f5c6cb';
        elements.connectionStatus.style.color = '#721c24';
    }
    
    appState.isConnected = isConnected;
}

/**
 * Maneja errores de peticiones HTTP
 */
function handleFetchError(error) {
    console.error('Error de conexión:', error);
    
    updateConnectionStatus(false);
    
    if (error.name === 'AbortError') {
        showNotification('⏱️ Timeout de conexión', 'warning');
    } else if (error.message.includes('fetch')) {
        showNotification('📡 Error de red', 'error');
    } else {
        showNotification(`❌ Error: ${error.message}`, 'error');
    }
}

/**
 * Muestra/oculta el overlay de carga
 */
function showLoading(show) {
    if (elements.loadingOverlay) {
        if (show) {
            elements.loadingOverlay.classList.add('show');
        } else {
            elements.loadingOverlay.classList.remove('show');
        }
    }
}

/**
 * Muestra una notificación
 */
function showNotification(message, type = 'info', duration = 3000) {
    if (!elements.notification) return;
    
    elements.notification.textContent = message;
    elements.notification.className = `notification ${type}`;
    elements.notification.classList.add('show');
    
    setTimeout(() => {
        elements.notification.classList.remove('show');
    }, duration);
    
    console.log(`📢 ${type.toUpperCase()}: ${message}`);
}

/**
 * Formatea un timestamp para mostrar tiempo relativo
 */
function formatRelativeTime(timestamp) {
    const now = new Date();
    const time = new Date(timestamp);
    const diffMs = now - time;
    const diffSecs = Math.floor(diffMs / 1000);
    const diffMins = Math.floor(diffSecs / 60);
    const diffHours = Math.floor(diffMins / 60);
    
    if (diffSecs < 60) {
        return `hace ${diffSecs} segundos`;
    } else if (diffMins < 60) {
        return `hace ${diffMins} minutos`;
    } else if (diffHours < 24) {
        return `hace ${diffHours} horas`;
    } else {
        return time.toLocaleDateString();
    }
}

/**
 * Detecta el tipo de dispositivo
 */
function getDeviceType() {
    const userAgent = navigator.userAgent;
    if (/tablet|ipad|playbook|silk/i.test(userAgent)) {
        return 'tablet';
    } else if (/mobile|iphone|ipod|android|blackberry|opera|mini|windows\sce|palm|smartphone|iemobile/i.test(userAgent)) {
        return 'mobile';
    } else {
        return 'desktop';
    }
}

/**
 * Funciones exportadas para uso global
 */
window.refreshData = refreshData;
window.sendToAWS = sendToAWS;
window.startAutoRefresh = startAutoRefresh;
window.stopAutoRefresh = stopAutoRefresh;

// Log de información del dispositivo
console.log('📱 Tipo de dispositivo:', getDeviceType());
console.log('🌐 User Agent:', navigator.userAgent);
console.log('📊 Configuración:', CONFIG);
