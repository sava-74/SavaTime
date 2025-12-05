/*brief  
*Программный таймер на системном таймере millis() для Arduino
*Илья С sava-74@inbox.ru 24.09.2024
*Код взят из блоков программы FLProg.
*/

#ifndef SavaTime_h
#define SavaTime_h
#include <Arduino.h>

class SavaTime
{
public:
    SavaTime() : _flagOnTime(false), _startTime(0), _flagFix(false), _outMulti(false), _tofOutput(false), _tonOutput(false),_flagStTon(false) {}

	void TstTON() //включаем TON
	{
		_flagOnTime = true;
		if(!_flagStTon)
		{	
		_startTime = millis();  // Сбрасываем время начала отсчета
		_flagStTon = true;
		}
	}

   // Сброс и запуск таймера (вызывается при нажатии кнопки)
    void TRes()
    {
        _flagOnTime = true;
        _startTime = millis();  // Сбрасываем время начала отсчета
        _tonOutput = false;     // Сбрасываем выходной сигнал TON
    }

    // Выключить таймер (вызывается при отпускании кнопки)
    void TStop()
    {
        _flagOnTime = false;  // Останавливаем таймер
        _tonOutput = false;   // Сбрасываем выходной сигнал TON
		_flagStTon = false;
    }

    // Одновибратор с указанием периода в мс (требует явного запуска через TRes)
    bool TimeML(uint32_t period)
    {
        if (!_flagOnTime) {
            return false;  // Если таймер не запущен, возвращаем false
        }

        // Проверяем, истек ли период
        if (getElapsedTime() >= period)
        {
            _flagOnTime = false;  // Останавливаем таймер
            return true;          // Возвращаем true, если период истек
        }

        return false;  // Возвращаем false, если период еще не истек
    }

    // Генерация импульсов с указанием периода в мс (автозапуск)
    bool GenML(uint32_t period)
    {
        if (!_flagOnTime) {
            TRes();  // Автоматически запускаем таймер при первом вызове
        }

        if (getElapsedTime() >= period)
        {
            _startTime = millis();  // Обновляем время начала отсчета
            return true;            // Возвращаем true, если период истек
        }

        return false;  // Возвращаем false, если период еще не истек
    }

    // Мультивибратор с указанием периода в мс (автозапуск)
    bool Multi(uint32_t period)
    {
        if (!_flagOnTime) {
            TRes();  // Автоматически запускаем таймер при первом вызове
			_outMulti = false;
		}

        if (getElapsedTime() >= period)
        {
            _startTime = millis();
            _outMulti = !_outMulti;  // Переключаем состояние
        }

        return _outMulti;
    }

    // Несимметричный мультивибратор (автозапуск)
    bool AsMulti(uint32_t highPeriod, uint32_t lowPeriod)
    {
        if (!_flagOnTime) {
            TRes();  // Автоматически запускаем таймер при первом вызове
			_outMulti = false;
		}

        uint32_t elapsedTime = getElapsedTime();

        // Проверка, истек ли период для текущего состояния
        if (_outMulti && elapsedTime >= highPeriod)
        {
            _startTime = millis();  // Обновляем время начала отсчета
            _outMulti = false;      // Переключаем состояние на LOW
        }
        else if (!_outMulti && elapsedTime >= lowPeriod)
        {
            _startTime = millis();  // Обновляем время начала отсчета
            _outMulti = true;       // Переключаем состояние на HIGH
        }

        return _outMulti;  // Возвращаем текущее состояние
    }

    // Задержка на отключение (TOF) (требует явного запуска через TRes)
    bool TOF(uint32_t period)
    {
        if (!_flagOnTime) {
            return false;  // Если таймер не запущен, возвращаем false
        }

        // Проверяем, истек ли период с момента последнего вызова TRes
        if (getElapsedTime() >= period)
        {
            return false;  // Возвращаем false, если период истек
        }

        return true;  // Возвращаем true, если период еще не истек
    }

    // Оставшееся время для TOF (обратный отсчет)
    uint32_t TOF_Remaining(uint32_t period)
    {
        if (!_flagOnTime) {
            return 0;  // Если таймер не запущен, возвращаем 0
        }

        uint32_t elapsedTime = getElapsedTime();

        // Если период истек, возвращаем 0
        if (elapsedTime >= period)
        {
            return 0;
        }

        // Возвращаем оставшееся время
        return period - elapsedTime;
    }

    // Задержка на включение (TON) (требует явного запуска через TRes)
    bool TON(uint32_t period)
    {
        if (!_flagOnTime) {//Serial.println("_flagOnTime сработал");
            return false;  // Если таймер не запущен, возвращаем false
        }

        // Проверяем, истек ли период с момента последнего вызова TRes
        //Serial.print("getElapsedTime() = ");Serial.println(getElapsedTime());
		if (getElapsedTime() >= period)
        {
            _tonOutput = true;  // Устанавливаем выходной сигнал в true
			//Serial.println("getElapsedTime сработал");
        }

        return _tonOutput;  // Возвращаем текущее состояние выхода
    }

    // Запуск таймера с указанием периода в микросекундах (требует явного запуска через TRes)
    bool TimeMicros(uint32_t period)
    {
        if (!_flagOnTime) {
            return false;  // Если таймер не запущен, возвращаем false
        }

        if (!_flagFix)
        {
            _startTime = micros();
        }

        return checkElapsedTimeMicros(period);
    }

private:
    bool _flagOnTime;        // Флаг запуска таймера
    uint32_t _startTime;     // Время начала отсчета
    bool _flagFix;           // Флаг фиксации таймера
    bool _outMulti;          // Выход мультивибратора
    bool _tofOutput;         // Выход TOF
    bool _tonOutput;         // Выход TON
	bool _flagStTon;		 // Запуск TON
    // Получение истекшего времени с учетом переполнения (для millis())
    uint32_t getElapsedTime()
    {
        uint32_t currentTime = millis();
		//Serial.print("currentTime = ");Serial.print(currentTime);Serial.print(" - ");Serial.print(_startTime);Serial.print(" = ");Serial.println(currentTime - _startTime);
        if (currentTime >= _startTime)
        {
            return currentTime - _startTime;
        }
        else
        {
            return (0xFFFFFFFF - _startTime) + currentTime + 1;
        }
    }

    // Получение истекшего времени с учетом переполнения (для micros())
    uint32_t getElapsedTimeMicros()
    {
        uint32_t currentTime = micros();
		
        if (currentTime >= _startTime)
        {
            return currentTime - _startTime;
        }
        else
        {
            return (0xFFFFFFFF - _startTime) + currentTime + 1;
        }
    }

    // Проверка истекшего времени для micros()
    bool checkElapsedTimeMicros(uint32_t period)
    {
        if (!_flagOnTime) return false;  // Таймер остановлен

        uint32_t elapsedTime = getElapsedTimeMicros();
        return (elapsedTime >= period);
    }
};

#endif