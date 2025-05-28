#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN       5           // Pin de datos para la tira LED
#define NUM_LEDS      30          // Número total de LEDs en la tira
#define LED_TYPE      WS2812B     // Tipo de LED (compatible con WS2818B en tiempos)
#define COLOR_ORDER   GRB

CRGB leds[NUM_LEDS];

// Definición de modos
enum LEDMode { 
  DEFAULT_ANIM,  // Animación arcoíris por defecto
  MODE_B,      // B: Reversa (parpadea primeros y últimos 15 en blanco)
  MODE_I,      // I: Intermitentes (parpadea primeros y últimos 15 en ámbar)
  MODE_L,      // L: Izquierda (dos LEDs en movimiento hacia la izquierda)
  MODE_R,      // R: Derecha (dos LEDs en movimiento hacia la derecha)
  MODE_S       // S: Alto (enciende primeros y últimos 15 en rojo)
};
volatile LEDMode currentMode = DEFAULT_ANIM;

// Función para procesar comandos leídos desde el Serial
void processSerialCommand() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    // Ignorar saltos de línea y retornos de carro
    if (cmd == '\n' || cmd == '\r') return;
    // Convertir a mayúscula para evitar problemas
    cmd = toupper(cmd);
    switch (cmd) {
      case 'B':
        currentMode = (currentMode == MODE_B) ? DEFAULT_ANIM : MODE_B;
        Serial.println("Modo: Reversa (B)");
        break;
      case 'I':
        currentMode = (currentMode == MODE_I) ? DEFAULT_ANIM : MODE_I;
        Serial.println("Modo: Intermitentes (I)");
        break;
      case 'L':
        currentMode = (currentMode == MODE_L) ? DEFAULT_ANIM : MODE_L;
        Serial.println("Modo: Animaci\u00F3n Izquierda (L)");
        break;
      case 'R':
        currentMode = (currentMode == MODE_R) ? DEFAULT_ANIM : MODE_R;
        Serial.println("Modo: Animaci\u00F3n Derecha (R)");
        break;
      case 'S':
        currentMode = (currentMode == MODE_S) ? DEFAULT_ANIM : MODE_S;
        Serial.println("Modo: Alto/Stop (S)");
        break;
      default:
        Serial.println("Comando no reconocido");
        break;
    }
  }
}

// Tarea de FreeRTOS que actualiza la animaci\u00F3n de la tira LED según el modo activo
void animationTask(void* pvParameters) {
  // Variables utilizadas en distintos modos:
  uint8_t hue = 0;                    // Para la animaci\u00F3n arco\u00EDris
  static bool blinkToggle_B = false;  // Para parpadeo en modo B
  static bool blinkToggle_I = false;  // Para parpadeo en modo I
  static int leftPos = NUM_LEDS - 1;    // Para animaci\u00F3n izquierda
  static int rightPos = 0;              // Para animaci\u00F3n derecha

  for (;;) {
    switch(currentMode) {
      // Modo por defecto: animaci\u00F3n arco\u00EDris
      case DEFAULT_ANIM:
        fill_rainbow(leds, NUM_LEDS, hue, 5);
        FastLED.show();
        hue++; 
        vTaskDelay(pdMS_TO_TICKS(50));
        break;

      // Modo B: parpadeo primeros y \u00FAltimos 15 leds en blanco
      case MODE_B:
        blinkToggle_B = !blinkToggle_B;
        for (int i = 0; i < 15 && i < NUM_LEDS; i++) {
          leds[i] = blinkToggle_B ? CRGB::White : CRGB::Black;
          int index = NUM_LEDS - 1 - i;
          if (index >= 0 && index < NUM_LEDS)
            leds[index] = blinkToggle_B ? CRGB::White : CRGB::Black;
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(500));
        break;

      // Modo I: parpadeo primeros y \u00FAltimos 15 leds en \u00E1mbar
      case MODE_I:
        blinkToggle_I = !blinkToggle_I;
        {
          CRGB amber(255, 191, 0);  // Color \u00E1mbar
          for (int i = 0; i < 15 && i < NUM_LEDS; i++) {
            leds[i] = blinkToggle_I ? amber : CRGB::Black;
            int index = NUM_LEDS - 1 - i;
            if (index >= 0 && index < NUM_LEDS)
              leds[index] = blinkToggle_I ? amber : CRGB::Black;
          }
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(500));
        break;

      // Modo L: animaci\u00F3n direccional izquierda con dos leds (color púrpura)
      case MODE_L:
        FastLED.clear();
        leds[leftPos] = CRGB::Purple;
        int prev = leftPos - 1;
        if (prev < 0) prev = NUM_LEDS - 1;
        leds[prev] = CRGB::Purple;
        FastLED.show();
        leftPos--;             // Se mueve hacia la izquierda
        if (leftPos < 0) {
          leftPos = NUM_LEDS - 1;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      // Modo R: animaci\u00F3n direccional derecha con dos leds (color cian)
      case MODE_R:
        FastLED.clear();
        leds[rightPos] = CRGB::Cyan;
        int next = rightPos + 1;
        if (next >= NUM_LEDS) next = 0;
        leds[next] = CRGB::Cyan;
        FastLED.show();
        rightPos++;            // Se mueve hacia la derecha
        if (rightPos >= NUM_LEDS) {
          rightPos = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      // Modo S: detener animaci\u00F3n y encender los primeros y \u00FAltimos 15 leds en rojo
      case MODE_S:
        for (int i = 0; i < 15 && i < NUM_LEDS; i++) {
          leds[i] = CRGB::Red;
          int index = NUM_LEDS - 1 - i;
          if (index >= 0 && index < NUM_LEDS)
            leds[index] = CRGB::Red;
        }
        // Se apaga la parte media de la tira (opcional)
        for (int i = 15; i < NUM_LEDS - 15; i++) {
          leds[i] = CRGB::Black;
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(500));
        break;
    }
  }
}

void setup() {
  // Inicializa el puerto Serial para depuraci\u00F3n y para recibir comandos
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
         .setCorrection(TypicalLEDStrip);

  // Crea la tarea de animaci\u00F3n en un core independiente (para ESP32)
  xTaskCreatePinnedToCore(
    animationTask,    // Función de la tarea
    "AnimationTask",  // Nombre de la tarea
    4096,             // Tamaño de la pila
    NULL,             // Par\u00E1metro (no se usa)
    1,                // Prioridad de la tarea
    NULL,             // Handle de la tarea
    1                 // Core en el que se ejecut\u00E1 la tarea
  );
}

void loop() {
  // Se procesa la lectura de comandos via Serial
  processSerialCommand();
  // Para permitir que las tareas de FreeRTOS se ejecuten, se puede dar un peque\u00F1o retardo:
  vTaskDelay(pdMS_TO_TICKS(10));
}
