@echo off
rem срипт запускает статический анализ кода проекта используюя утилиту Cppcheck
rem для работы необходимо установить https://cppcheck.sourceforge.io/ и прописать путь в PATH
rem Гайд: https://habr.com/ru/articles/210256/
@echo on

cppcheck -q -j4 --enable=all --inconclusive  ./Core
PAUSE