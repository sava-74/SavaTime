/*
Программный таймер SavaTime (Logic & Function Blocks)
Версия: 06.12.2025
*/

#ifndef SavaTime_h
#define SavaTime_h
#include <Arduino.h>

class SavaTime
{
public:
    // Конструктор
    SavaTime() : _flagOnTime(false), _startTime(0), _outMulti(false), _trigLock(false) {}

    // =========================================================================
    // УПРАВЛЕНИЕ СОСТОЯНИЕМ
    // =========================================================================

    // Полный сброс таймера.
    // Используйте, если таймер вызывается не в каждом цикле (например, в switch-case),
    // чтобы сбросить "зависшее" состояние.
    void Reset()
    {
        _flagOnTime = false;
        _trigLock = false; // Сброс блокировки одиночного импульса
        _outMulti = false; // Сброс выхода мультивибратора
    }

    // Ручной запуск (обычно не нужен, так как функции управляются аргументами)
    void Start() {    
        _flagOnTime = true;
        _startTime = millis(); 
    }
	
    void StartMicros() {
        _flagOnTime = true;
        _startTime = micros();
    }

    // =========================================================================
    // ОДНОВИБРАТОР (Delayed Pulse)
    // =========================================================================
    /* 
      Выдает ОДИН импульс через заданное время после появления input.
      input = 0: Сброс ("взвод курка").
      input = 1: Старт отсчета -> Импульс -> Ожидание сброса.
    */
    bool Time(uint32_t period, bool input = true)
    {
        if (!input) {
            _trigLock = false;   // Снимаем блокировку
            _flagOnTime = false; // Сбрасываем таймер
            return false;
        }

        // Если это передний фронт (нажали, и еще не блокировано)
        if (!_trigLock) {
            _trigLock = true;    // Блокируем повторный запуск
            _flagOnTime = true;  // Запускаем таймер
            _startTime = millis();
        }

        if (!_flagOnTime) return false; // Таймер уже отработал или не запущен

        if (getElapsedTime() >= period) {
            _flagOnTime = false; // Импульс выдан, выключаемся
            return true;
        }

        return false;
    }

    // То же самое для микросекунд
    bool TimeMicros(uint32_t period, bool input = true)
    {
        if (!input) {
            _trigLock = false;
            _flagOnTime = false;
            return false;
        }

        if (!_trigLock) {
            _trigLock = true;
            _flagOnTime = true;
            _startTime = micros();
        }

        if (!_flagOnTime) return false;

        if (getElapsedTimeMicros() >= period) {
            _flagOnTime = false;
            return true;
        }
        return false;
    }

    // =========================================================================
    // ГЕНЕРАТОРЫ (С управляющим входом enable)
    // =========================================================================

    // Генератор импульсов (мс). 
    // enable = true: Работает циклически.
    // enable = false: Сбрасывается.
    bool Gen(uint32_t period, bool enable = true)
    {
        if (!enable) {
            _flagOnTime = false;
            return false;
        }

        if (!_flagOnTime) {
            _flagOnTime = true;
            _startTime = millis();
        }

        if (getElapsedTime() >= period)
        {
            _startTime = millis(); 
            return true;
        }
        return false;
    }

    // Симметричный мультивибратор (мигалка).
    bool Multi(uint32_t period, bool enable = true)
    {
        if (!enable) {
            _flagOnTime = false;
            _outMulti = false;
            return false;
        }

        if (!_flagOnTime) {
            _flagOnTime = true;
            _startTime = millis();
            _outMulti = false;
        }

        if (getElapsedTime() >= period)
        {
            _startTime = millis();
            _outMulti = !_outMulti;
        }
        return _outMulti;
    }

    // Асимметричный мультивибратор.
    bool AsMulti(uint32_t highPeriod, uint32_t lowPeriod, bool enable = true)
    {
        if (!enable) {
            _flagOnTime = false;
            _outMulti = false;
            return false;
        }

        if (!_flagOnTime) {
            _flagOnTime = true;
            _startTime = millis();
            _outMulti = false;
        }

        uint32_t elapsedTime = getElapsedTime();

        if (_outMulti && elapsedTime >= highPeriod)
        {
            _startTime = millis();
            _outMulti = false;
        }
        else if (!_outMulti && elapsedTime >= lowPeriod)
        {
            _startTime = millis();
            _outMulti = true;
        }
        return _outMulti;
    }

    // =========================================================================
    // АВТОМАТИКА (TON / TOF)
    // =========================================================================

    // Задержка ВКЛЮЧЕНИЯ (TON)
    // input = 1: Таймер тикает. Время вышло -> true (удержание).
    // input = 0: Мгновенный сброс в false.
    bool TON(uint32_t period, bool input)
    {
        if (!input) {
            _flagOnTime = false;
            return false;
        }

        if (!_flagOnTime) {
            _flagOnTime = true;
            _startTime = millis();
            return false;
        }

        if (getElapsedTime() >= period) return true;

        return false;
    }

    // Задержка ОТКЛЮЧЕНИЯ (TOF) с перезапуском
    // input = 1: Выход true, таймер постоянно перезаряжается.
    // input = 0: Выход true, идет отсчет времени. Время вышло -> false.
    bool TOF(uint32_t period, bool input)
    {
        if (input) {
            _flagOnTime = true;
            _startTime = millis(); // Рестарт (удержание заряда)
            return true;
        }

        if (!_flagOnTime) return false; // Таймер уже разрядился

        if (getElapsedTime() >= period) {
            _flagOnTime = false; // Время вышло
            return false;
        }
        return true; // Держим питание
    }

    // Утилита: Сколько времени осталось (для TOF)
    uint32_t TOF_Remaining(uint32_t period)
    {
        if (!_flagOnTime) return 0;
        uint32_t elapsedTime = getElapsedTime();
        if (elapsedTime >= period) return 0;
        return period - elapsedTime;
    }

private:
    bool _flagOnTime;        // Флаг активности
    uint32_t _startTime;     // Время старта
    bool _outMulti;          // Выход генератора
    bool _trigLock;          // Блокировка одиночного импульса

    // Оптимизированный расчет времени (авто-переполнение uint32_t)
    uint32_t getElapsedTime(){
        return millis() - _startTime;
    }
    uint32_t getElapsedTimeMicros(){
        return micros() - _startTime;
    }
};

#endif