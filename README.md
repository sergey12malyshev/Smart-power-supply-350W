Программно-аппаратный проект импульсного источника питания c цифровой системой контроля

Дипломный проект университета

Software and hardware design of a switching power supply with a digital control system. Thesis project of the university


## Программная часть (Software) ##

### Инструментарий
Среда компиляции: KEIL.uVision V5.34.0.0 

Редактор кода: VScode (файл *HOME_PS_300.code-workspace*)

Используемые библиотеки: HAL ST
### Статический анализатор кода
Cppcheck 2.10 https://cppcheck.sourceforge.io/

Запустить RunStaticAnalysisCODE.cmd

Возможен вывод результата анализа в файл
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

### Command Line Interface, CLI
Терминал реализован через интерфейс UART TTL. 
Для Tera Term файл конфигурации: *utils/TERATERM.INI*
Для PuTTY запустить **PuTTYstart.cmd**

<img src= "https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/Screen.png" width=45% height=45%>

### Репозиторий
Применена модель ветвления git flow: http://danielkummer.github.io/git-flow-cheatsheet/

Для инициализации запустить git bash и ввести:
```bash
git flow init -f
```
### Версионирование
Semantic Versioning 2.0.0 https://semver.org/

## Аппаратная часть (Hardware) ##
Расположена в директории *hardware*. Схемы Э3 и Э4 представлены в каталоге *shemes*. Модель импульсного преобразователя в программе LTSpice в каталоге *shemes*. Исходные файлы печатных плат в каталоге *pcb*. Изображения макетного образца в каталоге *picturies*
 
 <img src="https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/Структурная.png" width=65% height=55%> 
 
Физически представляет собой две платы: 
- Cиловой полумостовой модуль; 
- Модуль управления и защиты.

<img src="https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/%D0%A1%D1%85%D0%B5%D0%BC%D0%B0%20%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D1%8F%20%2B%20%D0%B7%D0%B0%D1%89%D0%B8%D1%82%D1%8B.jpg" width=25% height=25%>  <img src="https://github.com/sergey12malyshev/Smart-power-supply-350W/blob/develop/hardware/picturies/%D0%A1%D0%B8%D0%BB%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%87%D0%B0%D1%81%D1%82%D1%8C.jpg" width=18% height=18%> 

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

https://alexgyver.ru/lessons/code-optimisation/

https://alexgyver.ru/lessons/filters/

https://itnan.ru/post.php?c=1&p=267573 - про RTOS

## TODO ##
- Поднять CAN;

- Перевести на библиотеку LL;

- Вместо RTOS применить легкие протопотоки (сопрограммы), например http://dunkels.com/adam/pt/
