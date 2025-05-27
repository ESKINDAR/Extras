#include <Arduino.h>
#include <FastLED.h>

// Configuración de la tira de LED
#define LED_PIN       5           // Pin de datos conectado a la tira
#define NUM_LEDS      30          // Número de LEDs en la tira
#define LED_TYPE      WS2812B     // Aunque la tira es WS2818B, el estándar de tiempo es similar al WS2812B
#define COLOR_ORDER   GRB

CRGB leds[NUM_LEDS];

// Tarea de FreeRTOS para la animación
void animationTask(void* pvParameters) {
    uint8_t hue = 0;
    // Bucle infinito de animación
    for (;;) {
        // Se llena la tira con un degradado arcoíris que comienza en el valor de hue
        fill_rainbow(leds, NUM_LEDS, hue, 5);
        FastLED.show(); // Actualiza la tira
        hue++;        // Incrementa el tono para la siguiente iteración

        // Espera 50 ms (la función vTaskDelay es propia de FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    // Inicializa la tira de LED
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
           .setCorrection(TypicalLEDStrip);
  
    // Crea y lanza la tarea de animación en el core 1 (para ESP32; en otros MCUs, el pinado puede variar)
    xTaskCreatePinnedToCore(
        animationTask,    // Función de la tarea
        "AnimationTask",  // Nombre de la tarea (para depuración)
        4096,             // Tamaño de la pila (en palabras)
        NULL,             // Parámetro de entrada a la tarea
        1,                // Prioridad de la tarea
        NULL,             // Handle de la tarea (no se usa en este ejemplo)
        1                 // Core en el que se ejecutará la tarea
    );
}

void loop() {
    // El loop principal queda "libre" ya que la animación se ejecuta en una tarea independiente.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
