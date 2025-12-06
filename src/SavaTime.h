/*
 * SavaTime Library
 * Программный таймер на системных функциях millis()/micros()
 * Реализует логику промышленных функциональных блоков (TON, TOF, Pulse, Gen).
 * 
 * Версия: 1.0.1 (Release)
 * Автор: SavaLab
 */

#ifndef SavaTime_h
#define SavaTime_h
#include <Arduino.h>

class SavaTime
{
public:
    SavaTime() : _flagOnTime(false), _startTime(0), _outMulti(false), _trigLock(false) {}

    // =========================================================================
    // УПРАВЛЕНИЕ СОСТОЯНИЕМ
    // =========================================================================

    /**
     * @brief ПОЛНЫЙ СБРОС (RESET)
     * Принудительно обнуляет состояние таймера.
     * Используйте эту функцию, если логика программы прерывалась (например, в switch-case),
     * чтобы таймер не сработал мгновенно из-за старого времени.
     */
    void Reset()
    {
        _flagOnTime = false;
        _trigLock = false; 
        _outMulti = false; 
    }

    // =========================================================================
    // ОДНОВИБРАТОР (ИМПУЛЬС)
    // =========================================================================

    /**
     * @brief ОДНОВИБРАТОР (Одиночный импульс / Delayed Pulse)
     * Выдает true ОДИН раз через заданное время после появления сигнала.
     * Повторный запуск возможен только после сброса input в 0.
     * 
     * @param period Время задержки перед импульсом (мс).
     * @param input Управляющий сигнал. true -> Старт отсчета. false -> Сброс ("взвод курка").
     * @return true - импульс выдан (один цикл), false - ожидание или сброс.
     */
    bool Time(uint32_t period, bool input = true)
    {
        if (!input) {
            _trigLock = false;   
            _flagOnTime = false; 
            return false;
        }

        if (!_trigLock) {
            _trigLock = true;    
            _flagOnTime = true;  
            _startTime = millis();
        }

        if (!_flagOnTime) return false;

        if (getElapsedTime() >= period) {
            _flagOnTime = false; 
            return true;
        }
        return false;
    }

    /**
     * @brief ОДНОВИБРАТОР (Микросекунды)
     * Аналог функции Time, но работает с высокой точностью (micros).
     * 
     * @param period Время задержки (мкс).
     * @param input Управляющий сигнал.
     */
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
    // ГЕНЕРАТОРЫ (ЦИКЛИЧЕСКИЕ ТАЙМЕРЫ)
    // =========================================================================

    /**
     * @brief ГЕНЕРАТОР СОБЫТИЙ (Generator)
     * Выдает короткий импульс (true) каждые N миллисекунд.
     * 
     * @param period Период срабатывания (мс).
     * @param enable Разрешение работы. true -> Работает. false -> Сброс.
     * @return true - наступило событие (один цикл).
     */
    bool Gen(uint32_t period, bool enable = true)
    {
        if (!enable) {
            _flagOnTime = false;
            return false;
        }

        if (!_flagOnTime) Start();

        if (getElapsedTime() >= period)
        {
            _startTime = millis(); 
            return true;
        }
        return false;
    }

    /**
     * @brief МУЛЬТИВИБРАТОР (Blinker / Multi)
     * Симметричный генератор прямоугольных импульсов ("Мигалка").
     * 
     * @param period Время полупериода (мс). Например, 500 = 500мс ВКЛ, 500мс ВЫКЛ.
     * @param enable Разрешение работы. true -> Мигает. false -> Выключен (0).
     * @return Текущее состояние выхода (true/false).
     */
    bool Multi(uint32_t period, bool enable = true)
    {
        if (!enable) {
            _flagOnTime = false;
            _outMulti = false;
            return false;
        }

        if (!_flagOnTime) {
            Start();
            _outMulti = false;
        }

        if (getElapsedTime() >= period)
        {
            _startTime = millis();
            _outMulti = !_outMulti;
        }
        return _outMulti;
    }

    /**
     * @brief АСИММЕТРИЧНЫЙ МУЛЬТИВИБРАТОР (Asymmetric Generator)
     * Генератор с разным временем включенного и выключенного состояния.
     * 
     * @param highPeriod Время во включенном состоянии (мс).
     * @param lowPeriod Время в выключенном состоянии (мс).
     * @param enable Разрешение работы.
     * @return Текущее состояние выхода.
     */
    bool AsMulti(uint32_t highPeriod, uint32_t lowPeriod, bool enable = true)
    {
        if (!enable) {
            _flagOnTime = false;
            _outMulti = false;
            return false;
        }

        if (!_flagOnTime) {
            Start();
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

    /**
     * @brief ЗАДЕРЖКА ВКЛЮЧЕНИЯ (TON / On-Delay)
     * Выход становится активным, только если вход удерживается в течение заданного времени.
     * 
     * @param period Время задержки (мс).
     * @param input Входной сигнал. true -> Таймер считает. false -> Мгновенный сброс.
     * @return true - время вышло (выход активен), false - ожидание или сброс.
     */
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

    /**
     * @brief ЗАДЕРЖКА ОТКЛЮЧЕНИЯ (TOF / Off-Delay)
     * Удерживает выход активным в течение заданного времени после пропадания сигнала.
     * Если сигнал появляется снова, таймер перезапускается (удержание продлевается).
     * 
     * @param period Время удержания (мс).
     * @param input Входной сигнал. true -> Выход активен (таймер заряжен). false -> Обратный отсчет.
     * @return true - выход активен. false - время вышло.
     */
    bool TOF(uint32_t period, bool input)
    {
        if (input) {
            _flagOnTime = true;
            _startTime = millis(); 
            return true;
        }

        if (!_flagOnTime) return false; 

        if (getElapsedTime() >= period) {
            _flagOnTime = false; 
            return false;
        }
        return true; 
    }

    /**
     * @brief ОСТАТОК ВРЕМЕНИ (TOF Remaining)
     * Возвращает оставшееся время удержания для таймера TOF.
     * 
     * @param period Тот же период, что задан в функции TOF (мс).
     * @return Количество миллисекунд до отключения. 0 - если таймер не активен.
     */
    uint32_t TOF_Remaining(uint32_t period)
    {
        if (!_flagOnTime) return 0;
        uint32_t elapsedTime = getElapsedTime();
        if (elapsedTime >= period) return 0;
        return period - elapsedTime;
    }

private:
    bool _flagOnTime;        
    uint32_t _startTime;     
    bool _outMulti;          
    bool _trigLock;          

    // Внутренний запуск (теперь недоступен пользователю напрямую)
    void Start() {    
        _flagOnTime = true;
        _startTime = millis(); 
    }
	
    void StartMicros() {
        _flagOnTime = true;
        _startTime = micros();
    }

    uint32_t getElapsedTime(){
        return millis() - _startTime;
    }
    uint32_t getElapsedTimeMicros(){
        return micros() - _startTime;
    }
};

#endif