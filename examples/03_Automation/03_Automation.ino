/*
 * ПРИМЕР 3: Промышленная автоматика (TON, TOF, Reset, Remaining)
 * 
 * Описание:
 * Эмуляция управления Насосом и Вентилятором охлаждения.
 * Демонстрация цепочки таймеров и функции аварийного сброса Reset().
 * 
 * Оборудование (ВХОДЫ):
 * PIN 12: Кнопка "СТАРТ" (Рабочий режим).
 * PIN 13: Кнопка "RESET" (Ручной сброс таймеров).
 * PIN 15: Кнопка "СБОЙ" (Имитация блокировки выполнения кода).
 * 
 * Логика выхода (Смотрим в Serial Monitor):
 * 1. Насос: Включается с задержкой 2 сек (TON).
 * 2. Вентилятор: Работает пока работает Насос + 3 сек после (TOF).
 */

#include "SavaTime.h"

// --- Настройки ---
const int PIN_BTN_START = 12;
const int PIN_BTN_RESET = 13;
const int PIN_BTN_BLOCK = 15;

const uint32_t TIME_DELAY_ON  = 2000; // 2 сек задержка включения
const uint32_t TIME_DELAY_OFF = 3000; // 3 сек охлаждение

// --- Объекты ---
SavaTime timerPumpTON; // Таймер на включение насоса
SavaTime timerFanTOF;  // Таймер на выключение вентилятора

// Состояния для вывода в порт (чтобы не спамить)
bool prevPumpState = false;
bool prevFanState = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  pinMode(PIN_BTN_BLOCK, INPUT_PULLUP);

  Serial.println("--- Старт Примера 03_Automation ---");
  Serial.println("12: Старт, 13: Сброс, 15: Имитация сбоя");
}

void loop() {
  // Считываем кнопки (инверсия из-за INPUT_PULLUP)
  bool signalStart = !digitalRead(PIN_BTN_START);
  bool signalReset = !digitalRead(PIN_BTN_RESET);
  bool signalBlock = !digitalRead(PIN_BTN_BLOCK);

  // =================================================================
  // 1. ИМИТАЦИЯ СБОЯ (Плохой стиль / Блокировка)
  // Если нажата кнопка 15, мы просто "зависаем" и не идем дальше.
  // Таймеры перестают обновляться.
  // =================================================================
  if (signalBlock) {
    Serial.println("[СБОЙ] Программа остановлена (имитация)...");
    delay(500); // Тормозим процессор
    return;     // Выходим из loop, не доходя до логики таймеров!
  }

  // =================================================================
  // 2. АВАРИЙНЫЙ СБРОС (Reset)
  // Если нажата кнопка 13, принудительно обнуляем таймеры.
  // Это лечит последствия "Сбоя", если таймеры запомнили старое время.
  // =================================================================
  if (signalReset) {
    timerPumpTON.Reset();
    timerFanTOF.Reset();
    Serial.println(">>> АВАРИЙНЫЙ СБРОС (RESET) ВЫПОЛНЕН <<<");
    // Важно: после Reset таймеры чисты, как при запуске Arduino
  }

  // =================================================================
  // 3. ЛОГИКА АВТОМАТИКИ
  // =================================================================

  // Шаг А: Управление Насосом (Задержка включения 2 сек)
  // На вход подаем сигнал с кнопки Старт.
  bool pumpState = timerPumpTON.TON(TIME_DELAY_ON, signalStart);

  // Шаг Б: Управление Вентилятором (Задержка выключения 3 сек)
  // На вход подаем состояние Насоса.
  // Если Насос ВКЛ -> Вентилятор заряжается (ВКЛ).
  // Если Насос ВЫКЛ -> Вентилятор работает еще 3 сек.
  bool fanState = timerFanTOF.TOF(TIME_DELAY_OFF, pumpState);


  // =================================================================
  // 4. ВИЗУАЛИЗАЦИЯ (Serial)
  // =================================================================
  
  // Вывод состояния Насоса
  if (pumpState != prevPumpState) {
    if (pumpState) Serial.println("НАСОС: [ВКЛЮЧЕН]");
    else           Serial.println("НАСОС: [ОСТАНОВЛЕН]");
    prevPumpState = pumpState;
  }

  // Вывод состояния Вентилятора
  if (fanState != prevFanState) {
    if (fanState) Serial.println("ВЕНТИЛЯТОР: [ВКЛЮЧЕН]");
    else          Serial.println("ВЕНТИЛЯТОР: [ОСТАНОВЛЕН]");
    prevFanState = fanState;
  }

  // Индикация обратного отсчета (Только если идет охлаждение)
  // Охлаждение = Насос выключился (input=0), а Вентилятор еще крутит.
  if (!pumpState && fanState) {
    // Используем функцию TOF_Remaining
    uint32_t left = timerFanTOF.TOF_Remaining(TIME_DELAY_OFF);
    
    // Выводим не чаще раза в 200мс, чтобы читать успевали
    static uint32_t printTimer = 0;
    if (millis() - printTimer > 500) {
       printTimer = millis();
       Serial.print("   Охлаждение... Осталось: ");
       Serial.print(left);
       Serial.println(" мс");
    }
  }
}