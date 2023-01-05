Программный проект системы управления импульсного источника питания

### Среда компиляции 
KEIL.uVision V5.34.0.0

### Настройка периферии
STM32 CubeMX

### Контроллер
STM32F103C8T6 (фактически стоит китайский аналог CH32F103C8T6)

### RTOS
FreeRTOS Kernel V10.0.1

### Сборка 
Запустить **Proba.BAT**
 
(Options for target ... -> Вкладка "Output" -> Галка "Create batch file")

### Прошивка ПО
ST-Link V2, запустить **programFlash.bat**

### Аппаратная часть
Расположена в директории *hardware*

Схемы Э3 и Э4 представлены в каталоге *shemes*

Модель импульсного преобразователя в программе LTSpice в каталоге *shemes*

Исходные файлы печатных плат в каталоге *pcb*

Изображения макетного образца в каталоге *picturies*

### Полезные ссылки
https://microtechnics.ru/stm32cubemx-bystryj-start-s-freertos-dlya-stm32/

https://hackaday.com/2020/10/22/stm32-clones-the-good-the-bad-and-the-ugly/
