# Лабораторная работа № 1 — вариант 12

[![CI](https://github.com/mevdrr/lab_device/actions/workflows/c-cpp.yml/badge.svg?branch=variant-12-recycle)](https://github.com/mevdrr/lab_device/actions/workflows/c-cpp.yml)
[![Покрытие строк](https://raw.githubusercontent.com/mevdrr/lab_device/coverage-variant-12/coverage.svg)](https://github.com/mevdrr/lab_device/tree/coverage-variant-12)

Работа выполнена на основе [imitrichev/lab_device](https://github.com/imitrichev/lab_device). Вариант 12: добавить признак «рассчитан / не рассчитан» и предупреждать, если выходной поток ведёт в уже рассчитанный аппарат.

## Запуск

Для сборки из исходников нужны CMake 3.20+ и C++17. GoogleTest скачивается при первой сборке:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel 2
ctest --test-dir build -C Debug --output-on-failure
```

В полной папке на рабочем столе есть `RUN_DEMO.cmd` и `RUN_TESTS.cmd`. Они запускают демонстрацию и 27 тестов.

## Решение

`Device::isCalculated()` возвращает состояние аппарата. `updateOutputs()` сбрасывает старое состояние и ставит флаг только после успешного расчёта. `resetCalculationState()` начинает новый проход. `checkRecycle()` реализует точное правило варианта 12; `updateAndCheckRecycle()` объединяет расчёт и проверку.

Аппараты соединены, когда один и тот же объект `Stream` является выходом одного и входом другого. `hasDirectedCycle()` дополнительно проверяет структуру графа DFS с тремя цветами: белый, серый, чёрный. Это отличает настоящий цикл от повторного прихода в узел при слиянии ветвей.

Программа обнаруживает рецикл, но не решает численную сходимость циклической химической схемы.

## Проверка и CI

В `tests/` находятся 27 тестов GoogleTest: цепочки, петли на одном аппарате, циклы из двух и трёх аппаратов, разветвление, слияние, сброс состояния, ошибки схемы и баланс массы. `runTests()` — отдельная функция запуска тестов.

`.github/workflows/c-cpp.yml` запускает сборку и тесты на Ubuntu и Windows, санитайзеры на Linux, gcovr-покрытие с порогом 95%, JUnit/HTML/XML/JSON-артефакты и автоматически обновляемый SVG-бейдж в ветке `coverage-variant-12`.

Локальная проверка дала 27/27 тестов и 98,2% покрытия строк библиотеки (`device.cpp/.hpp`, `recycle.cpp/.hpp`).

Подробнее: [docs/REPORT.md](docs/REPORT.md), [docs/DEFENSE.md](docs/DEFENSE.md), [docs/PULL_REQUEST.md](docs/PULL_REQUEST.md).
