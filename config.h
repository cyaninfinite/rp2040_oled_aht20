const uint8_t DEBUG_PIN{ 20 };

const uint8_t LED_PIN{ 12 };
const int LED_COUNT{ 1 };
const int LED_MAX_BRIGHTNESS{ 15 };
// LED start temp threshold (celcius)
const int start_hot{ 35 };
const int start_cold{ 30 };

// I2C def: SDA, SCL
const uint8_t I2C1_pins[2] = { 22, 23 };  //I2C1 for OLED Display
const uint8_t I2C0_pins[2] = { 28, 29 };  //I2C0 for AHT20

// Refresh
const long interval{ 1000 };  //Update interval in ms