Программно-аппаратный проект импульсного источника питания c цифровой системой контроля

### Инструментарий
Среда компиляции: KEIL.uVision V5.34.0.0 

Редактор кода: VScode (файл *HOME_PS_300.code-workspace*)

Используемые библиотеки: HAL ST

### Настройка периферии
STM32 CubeMX

### Контроллер
STM32F103C8T6 (фактически стоит китайский аналог CH32F103C8T6)

### RTOS
FreeRTOS Kernel V10.0.1

### Сборка 
Запустить **BuildRun.cmd**
 
(Options for target ... -> Вкладка "Output" -> Галка "Create batch file")

### Прошивка ПО
Подключть ST-Link V2, запустить **programFlash.bat**

### Репозиторий
Применена модель ветвления git flow: http://danielkummer.github.io/git-flow-cheatsheet/

Для инициализации запустить git bash и ввести:
```bash
git flow init -f
```
### Command Line Interface, CLI
Терминал реализован через интерфейс UART TTL. 
Для Tera Term файл конфигурации: *utils/TERATERM.INI*
Для PuTTY запустить **PuTTYstart.cmd**

<img src= "https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/CLI.bmp" width=25% height=24%>

### Аппаратная часть
Расположена в директории *hardware*. Схемы Э3 и Э4 представлены в каталоге *shemes*. Модель импульсного преобразователя в программе LTSpice в каталоге *shemes*. Исходные файлы печатных плат в каталоге *pcb*. Изображения макетного образца в каталоге *picturies*

<img src="https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/%D0%A1%D1%85%D0%B5%D0%BC%D0%B0%20%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D1%8F%20%2B%20%D0%B7%D0%B0%D1%89%D0%B8%D1%82%D1%8B.jpg" width=15% height=15%>  <img src="https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/%D0%A1%D0%B8%D0%BB%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%87%D0%B0%D1%81%D1%82%D1%8C.jpg" width=12% height=12%> 

### Характеристики
-	Рабочий диапазон входного напряжения 198 - 242 В, 50 Гц 
-	Диапазон выходного напряжения, В (24 - 30) ± 0,2 
-	Мощность, отдаваемая в нагрузку не менее, Вт 300 
-	КПД не менее 0,85 
-	Время выхода на режим не более, с 0,5 
-	Реализованы следующие аппаратные защиты:-от пониженного входного напряжения; -от повышенного выходного напряжения; -от короткого замыкания. 
-	Интерфейс информационного обмена: CAN (на данный момент не реализован), UART-монитор
- Осуществляется контроль параметров источника питания и управление посредством интерфейса: измерение выходного напряжения и тока; контроль состояния аппаратных защит; дистанционное включение-выключение источника.


### Полезные ссылки
https://microtechnics.ru/stm32cubemx-bystryj-start-s-freertos-dlya-stm32/

https://hackaday.com/2020/10/22/stm32-clones-the-good-the-bad-and-the-ugly/

### TODO
- Поднять CAN;

- Перевести на библиотеку LL;

- Вместо RTOS применить легкие протопотоки (сопрограммы), например http://dunkels.com/adam/pt/
