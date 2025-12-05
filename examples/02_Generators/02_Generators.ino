/*
 * ПРИМЕР 2: Генераторы и Мультивибраторы (02_Generators)
 * 
 * Цель:
 * Показать работу циклических таймеров с управляющим входом (enable).
 * Демонстрация того, как избежать конструкций if/else при управлении автоматикой.
 * 
 * Оборудование:
 * 1. Кнопка "Рубильник": PIN 12 (Вкл/Выкл всей системы).
 * 2. Светодиод "Авария" (Симметричный): PIN 13 (Встроенный).
 * 3. Светодиод "Маяк" (Асимметричный): PIN 15.
 * 
 * Логика:
 * Пока кнопка НАЖАТА -> Все таймеры работают синхронно.
 * Кнопка ОТПУЩЕНА -> Все таймеры сбрасываются и выходы гаснут.
 */

#include "SavaTime.h"

// --- Настройки Пинов ---
const int PIN_BTN_ENABLE = 12; // Главный рубильник
const int PIN_LED_SYM    = 13; // Симметричный (Авария)
const int PIN_LED_ASYM   = 15; // Асимметричный (Маяк)

// --- Настройки Времени ---
const uint32_t TIME_GEN_EVENT = 2000; // Событие раз в 2 сек
const uint32_t TIME_MULTI     = 300;  // 300 мс вкл / 300 мс выкл
const uint32_t TIME_FLASH     = 50;   // 50 мс вспышка
const uint32_t TIME_PAUSE     = 1000; // 1000 мс пауза

// --- Объекты Таймеров ---
SavaTime timerEvent;  // Для текстовых событий
SavaTime timerSym;    // Для симметричного мигания
SavaTime timerAsym;   // Для асимметричного стробоскопа

// Вспомогательные переменные для отслеживания изменений в Serial
bool prevSymState = false;
bool prevAsymState = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_BTN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_LED_SYM, OUTPUT);
  pinMode(PIN_LED_ASYM, OUTPUT);

  Serial.println("--- Старт Примера 02_Generators ---");
  Serial.println("Нажмите и держите кнопку 12 для запуска генераторов.");
}

void loop() {
  // 1. Считываем общий сигнал управления (Enable)
  // Нажата = true, Отпущена = false
  bool systemOn = !digitalRead(PIN_BTN_ENABLE);

  // =================================================================
  // 1. ГЕНЕРАТОР СОБЫТИЙ (Gen)
  // Выдает true один раз за период, если systemOn == true.
  // =================================================================
  if (timerEvent.Gen(TIME_GEN_EVENT, systemOn)) {
    Serial.print("[GEN] Прошло 2 секунды. Системное время: ");
    Serial.println(millis());
  }

  // =================================================================
  // 2. СИММЕТРИЧНЫЙ МУЛЬТИВИБРАТОР (Multi)
  // Возвращает true/false (меандр), пока systemOn == true.
  // При systemOn == false мгновенно сбрасывается в false.
  // =================================================================
  bool symState = timerSym.Multi(TIME_MULTI, systemOn);
  
  digitalWrite(PIN_LED_SYM, symState); // Управление светодиодом

  // Вывод в порт только при изменении состояния (чтобы не спамить)
  if (symState != prevSymState) {
    Serial.print("   -> Multi (Pin 13): ");
    Serial.println(symState ? "ВКЛ" : "выкл");
    prevSymState = symState;
  }

  // =================================================================
  // 3. АСИММЕТРИЧНЫЙ МУЛЬТИВИБРАТОР (AsMulti)
  // Короткая вспышка, длинная пауза.
  // =================================================================
  bool asymState = timerAsym.AsMulti(TIME_FLASH, TIME_PAUSE, systemOn);
  
  digitalWrite(PIN_LED_ASYM, asymState); // Управление светодиодом

  // Вывод в порт при изменении
  if (asymState != prevAsymState) {
    Serial.print("      -> AsMulti (Pin 15): ");
    Serial.println(asymState ? "ВСПЫШКА!" : ".......");
    prevAsymState = asymState;
  }
}