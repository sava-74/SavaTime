/*
 * ПРИМЕР 1: Функция Time (Одиночный импульс) с Антидребезгом
 * 
 * Логика работы:
 * 1. Фильтрация дребезга контактов (50 мс) через функцию TON.
 * 2. Если "чистый" сигнал удерживается 2000 мс -> Выдает сообщение "БАХ!".
 * 3. Если кнопку бросили раньше -> Выдает статистику (сколько держали).
 * 
 * Подключение:
 * Кнопка: PIN 12 (второй конец на GND)
 */

#include "SavaTime.h"

// --- Настройки ---
const int PIN_BTN = 12;         // Пин кнопки
const uint32_t DEBOUNCE_MS = 50;// Время антидребезга
const uint32_t DELAY_MS = 2000; // Время до "выстрела"

// --- Объекты ---
SavaTime debounceTimer; // Таймер для фильтрации дребезга
SavaTime mainTimer;     // Основной таймер логики

// --- Переменные для статистики ---
uint32_t pressStartTimestamp = 0; // Время начала нажатия
bool prevCleanState = false;      // Предыдущее состояние (для отлова фронтов)
bool isFired = false;             // Флаг: произошел ли выстрел в этом цикле нажатия?

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BTN, INPUT_PULLUP); // Кнопка замыкает на землю

  Serial.println("--- Старт Примера 1 ---");
  Serial.print("Нажмите и держите кнопку (Пин ");
  Serial.print(PIN_BTN);
  Serial.println(")");
  Serial.println("Ждем 2 секунды...");
}

void loop() {
  // 1. Считываем "сырой" сигнал (инвертируем, т.к. INPUT_PULLUP)
  bool rawBtnState = !digitalRead(PIN_BTN);

  // 2. АНТИДРЕБЕЗГ
  // Функция TON вернет true, только если rawBtnState равен 1 уже 50 мс подряд.
  // Если будет дребезг (0-1-0), таймер сбросится и вернет false.
  bool cleanBtnState = debounceTimer.TON(DEBOUNCE_MS, rawBtnState);

  // 3. ОБРАБОТКА НАЖАТИЯ (Передний фронт очищенного сигнала)
  if (cleanBtnState && !prevCleanState) {
    pressStartTimestamp = millis(); // Запоминаем время старта
    isFired = false;                // Сбрасываем флаг выстрела
    Serial.println("\n[Кнопка нажата] Отсчет пошел...");
  }

  // 4. ОСНОВНАЯ ЛОГИКА (Функция Time)
  // Передаем "чистый" сигнал. 
  if (mainTimer.Time(DELAY_MS, cleanBtnState)) {
    // --- ЭТОТ БЛОК СРАБОТАЕТ 1 РАЗ ЧЕРЕЗ 2 СЕКУНДЫ ---
    Serial.print("--> БАХ! (Импульс выдан ровно через ");
    Serial.print(millis() - pressStartTimestamp);
    Serial.println(" мс)");
    
    isFired = true; // Запоминаем, что выстрел был
  }

  // 5. ОБРАБОТКА ОТПУСКАНИЯ (Задний фронт очищенного сигнала)
  if (!cleanBtnState && prevCleanState) {
    // Кнопку отпустили. Смотрим, успел ли таймер сработать?
    
    if (isFired) {
      Serial.println("[Кнопка отпущена] Цикл завершен успешно. Таймер сброшен.");
    } else {
      // Если выстрела не было, значит бросили рано. Считаем время.
      uint32_t holdTime = millis() - pressStartTimestamp;
      uint32_t remaining = DELAY_MS - holdTime;
      
      Serial.println("[СРЫВ!] Кнопка отпущена слишком рано.");
      Serial.print("   Вы держали: "); Serial.print(holdTime); Serial.println(" мс");
      Serial.print("   Не хватило: "); Serial.print(remaining); Serial.println(" мс");
    }
  }

  // Запоминаем состояние для следующего цикла
  prevCleanState = cleanBtnState;
}