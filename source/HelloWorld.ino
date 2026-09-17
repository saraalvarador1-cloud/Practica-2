#include <Wire.h>
#include <MPU6050.h>
#include <Arduino.h>

MPU6050 mpu;

// Offsets de calibración (los calcularemos en setup)
float ax_offset = 0, ay_offset = 0, az_offset = 0;

// Constantes de conversión
// ±2g por defecto → 16384 LSB por cada 1g
const float LSB_PER_G   = 16384.0f;
const float G_TO_MS2    = 9.80665f;

// Número de muestras para calibración en reposo
const int CALIB_SAMPLES = 200;

void calibrar() {
  Serial.println("Calibrando... manten el sensor QUIETO y PLANO");
  delay(2000);

  long sum_ax = 0, sum_ay = 0, sum_az = 0;

  for (int i = 0; i < CALIB_SAMPLES; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    sum_ax += ax;
    sum_ay += ay;
    sum_az += az;
    delay(5);
  }

  // Promedio de las lecturas en reposo
  ax_offset = sum_ax / (float)CALIB_SAMPLES;
  ay_offset = sum_ay / (float)CALIB_SAMPLES;
  // En az, en reposo el sensor mide 1g (la gravedad).
  // Restamos 16384 para que "quieto" = 0 en z también
  az_offset = (sum_az / (float)CALIB_SAMPLES) - LSB_PER_G;

  Serial.println("Calibración completa!");
  Serial.print("Offsets raw → ax: "); Serial.print(ax_offset);
  Serial.print("  ay: ");             Serial.print(ay_offset);
  Serial.print("  az: ");             Serial.println(az_offset);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5); // SDA=D2 (GPIO4), SCL=D1 (GPIO5)

  mpu.initialize();

  if (!mpu.testConnection()) {
    delay(10000);
    Serial.println("ERROR: No se detecta el MPU-6500. Revisa el cableado.");
    while (true) delay(500);
  }
  Serial.println("MPU-6500 detectado!");

  // Rango del acelerómetro en ±2g (máxima resolución)
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  // Rango del giroscopio en ±250°/s
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

  calibrar();
  Serial.println("\n--- Iniciando lecturas ---");
  Serial.println("ax(m/s²) | ay(m/s²) | az(m/s²) | pitch(°) | roll(°)");
}

void loop() {
  int16_t ax_raw, ay_raw, az_raw;
  int16_t gx_raw, gy_raw, gz_raw;

  // Leer los 6 ejes de una sola vez (más eficiente que leerlos separado)
  mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);

  // Restar offset de calibración y convertir a m/s²
  float ax = ((ax_raw - ax_offset) / LSB_PER_G) * G_TO_MS2;
  float ay = ((ay_raw - ay_offset) / LSB_PER_G) * G_TO_MS2;
  float az = ((az_raw - az_offset) / LSB_PER_G) * G_TO_MS2;

  // ── Inclinación ──────────────────────────────────────────────
  // pitch: rotación adelante/atrás (eje X del sensor)
  // roll:  rotación izquierda/derecha (eje Y del sensor)
  // Usamos atan2 porque maneja los 4 cuadrantes correctamente.
  // La fórmula viene de proyectar el vector de gravedad sobre los ejes.
  float pitch = atan2(ay, sqrt(ax * ax + az * az)) * 180.0f / PI;
  float roll  = atan2(-ax, az) * 180.0f / PI;

  // Imprimir en formato legible
  Serial.print("ax: "); Serial.print(ax, 2);
  Serial.print(" | ay: "); Serial.print(ay, 2);
  Serial.print(" | az: "); Serial.print(az, 2);
  Serial.print(" | pitch: "); Serial.print(pitch, 1);
  Serial.print("° | roll: "); Serial.print(roll, 1);
  Serial.println("°");

  delay(100); // 10 Hz — suficiente para ver los valores cómodamente
}