# Fuzzing GIF

Тестовое задание в отдел фаззинга ИСП РАН

## Оглавление

+ [#]

## Постановка задачи

**Задача:**
Собранный бинарь (таргет), который ты будешь фаззить - должен принимать на вход данные определённого формата.

- Набрать качественный корпус входных данных.
Т.е. входные файлы должны соответствовать формату, который таргет принимает на вход.

- Написать и применить кастомную мутацию

В фаззере AFL++ есть такая возможность. Нужно написать мутацию, которая учитывая формат входных данных, принимаемый анализируемой программы, поддерживает 2 режима работы и применяет их (может быть один первый режим если он позволяет достичь прироста путей):
1) Сохранение формата входных данных после мутаций фаззера
2) Мутация входных данных с учетом формата входных данных (комбинация 
токенов и прочее)

Этот таргет нужно профаззить фаззером AFL++.

Нужно запустить 2+ процессов afl-fuzz и обеспечить синхронизацию между ними.
Необходимо убедиться, что есть прирост найденных путей.

## Фаззинг проекта  [TIMG](https://github.com/hzeller/timg), Epic Fail

Выбранная программа [TIMG](https://github.com/hzeller/timg) , формат файла `*.gif`.

Ход работы:
1. Собрал TIMG и посмотрел функционал.
2. Пересборка проекта с помощью CMake с добавлением AFL++:
Добавлено в CMakeLists.txt проекта TIMG:
```bash
set(CMAKE_C_COMPILER afl-clang-fast)
set(CMAKE_CXX_COMPILER afl-clang-fast++)

message(STATUS ">>> CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS ">>> CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS ">>> ASAN: ${AFL_USE_ASAN}")
```

![[images/20240708013018.png]]

3. В отдельной папке организую фаззинг.
Собрал корпус входных `*.gif` файлов и подключил соответственный стандартный словарь от aflpp. Запускаю aflpp(прохожу исправление всех ошибок).

```bash
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1
afl-fuzz -i input -o output -x ~/AFLplusplus/dictionaries/gif.dict -- /usr/local/bin/timg @@
```

![[images/20240709133535.png]]

Первый вывод: ошибка (odd, check syntax) и levels = 1 - фаззер не может пройти в глубину, нужно применить инструменты и посмотреть где он ломается

Пробую применить `afl-watsup`:
```bash
afl-whatsup -s output/
```

![[images/20240709134212.png]]

Ничего не дало.

Пробую применить `afl-plot`:
```bash
afl-plot output/default plot
```
![[images/20240709140736.png]]

Ничего не дало.

Пробую применить `afl-showmap`:

![[images/20240709141412.png]]

Показало, что дальше 20 вершин я не могу проникнуть по какой-то причине.

Пытаюсь применить [`afl-cov`](https://github.com/vanhauser-thc/afl-cov/tree/master)([установка](https://www.programmerall.com/article/96432444283/)): 

Пересобрал TIMG с подключением `gcov`: 

В CMakeLists.txt timg добавил и изменил компилятор на afl-gcc-fast после скомпилил и установил:
```cpp
set (CMAKE_CXX_COMPILER afl-g++-fast)
set (CMAKE_C_COMPILER afl-gcc-fast)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fprofile-arcs -ftest-coverage")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
```
Разместил в папке `./output/default/queue/` - свои хорошие примеры для дебага. Afl-cov скрипт считывает файлы оттуда и затем создает в месте бинарников файлы .gcda о покрытии. 

Запуск:

```bash
~/afl-cov/afl-cov -d ./output --overwrite --code-dir ~/timg/build/src/CMakeFiles/timg.dir --coverage-cmd "cat AFL_FILE | LD_LIBRARY_PATH=~/timg/build/src/CMakeFiles/timg.dir /usr/local/bin/timg AFL_FILE -g100x100"
```

Чтобы получить правильный вывод понадобилось 2 дня дебагга питоновского скрипта afl-cov который вызывал из под себя криво `lcov`.  Получал ошибку `Did not find any AFL test cases, exiting.`

Пришлось в установленном afl-cov пайтон скрипте искать проблему - оказалось при обработке оно считывало все файлы из default/queue/ с маской `id:000000` НО afl генерит файлы `id_000000` из-за этого он не мог считать ни одного файла. Поправил в самом коде, далее пришлось смотреть какие команды он вызывает и как объединяет файлы для lcov. Делал он это запуская 
`cat ./output/default/queue/id_000001 | timeout -s KILL 5 cat ./output/default/queue/id_000001 | LD_LIBRARY_PATH=~/timg/build/src/CMakeFiles/timg.dir /usr/local/bin/timg ./output/default/queue/id_000001 -g100x100` - треш. Поэтому подогнал параметры строки и запускал все с исходниками в непосредственном месте билда. 

После всех изменений и модификаций было получено для хороших изображений вывод:

![[images/20240711231701.png]]

Запускаю afl-cov с одним мутировавшим кривым gif:
![[images/20240712091107.png]]

В этот момент стало понятно, что лучше взять другой проект для фаззинга, чтобы вернуться к изначальному заданию :cry: :cry: :cry: :cry: Несмотря на это было знакомство с cmake и afl-cov.

## Фаззинг проекта [Сhafa](https://github.com/hpjansson/chafa), Success

## 4.1. Часть (Установка и базовый фаззинг)

Нужно было выбрать проект, работающий с командной строкой, без графического интерфейса(для простоты) и принимающий `*.gif`. Для этого я искал программы по типу TIMG, т.е. те которые могут выводить изображение в терминал, отдельная проблема с gif - не самый удобный. Нашел [подборку](https://www.linuxlinks.com/best-free-open-source-terminal-based-image-viewers/) из которой выбрал [chafa](https://github.com/hpjansson/chafa?tab=readme-ov-file)(tiv был занят)
Поэтому выбран проект [chafa](https://github.com/hpjansson/chafa?tab=readme-ov-file) и тип файлов `*.gif`

1. Собрал проект и запустил 

![[images/20240712182842.png]]

2. Сборка с afl++: для компиляции используется сначала `./autogen.sh` затем множество вложенных `Makefile`. В каждом определен `CC=gcc`, поэтому в make добавляю `make --environment-overrides CC=afl-gcc-fast`. И продолжаю установку.

В отдельном месте создаю `chafa/` с папками `input/*.gif` с корпусом входных гифок и `output/` для afl.

> `*.gif` может иметь несколько картинок подряд которые могут проигрываться с анимацией. Из-за этого нужно добавлять критерии выполнения иначе будет превышение лимита по времени. После изучения параметров chafa решил выключить анимацию `--animate=off`. Также решил оставить гифки из 1 изображения.

**Сырой запуск с обычными мутациями:**
```bash
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1
afl-fuzz -i input -o output_1 -- /usr/local/bin/chafa --animate=off @@
```
Итог:

![[images/20240713175636.png]]

**Запуск с подключением стандартного GIF словаря:**

```bash
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1
afl-fuzz -i input -o output_2 -x ~/AFLplusplus/dictionaries/gif.dict -- /usr/local/bin/chafa --animate=off @@
```

Итог:

![[images/20240713193126.png]]

Были проведены тесты для стандартного фаззинга

## 4.2. Часть (Создание своей мутации для `.gif`)

Чтобы увеличить эффективность фаззинга, нужно разобраться с типом входных данных и учесть мутацию конкретно для моего типа, т.е. `.gif`.

Поэтому нужно создать [кастомную мутацию](https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/custom_mutators.md) для AFL++ на основе `gif`. 
Для изучения внутренности `gif` использую статьи:

+ https://habr.com/ru/articles/274917/
+ https://habr.com/ru/articles/127083/
+ https://habr.com/ru/companies/tradingview/articles/184660/
+ https://www.matthewflickinger.com/lab/whatsinagif/bits_and_bytes.asp
+ https://www.matthewflickinger.com/lab/whatsinagif/lzw_image_data.asp
Для создания мутаций смотрю:
+ https://github.com/google/fuzzing/blob/master/docs/structure-aware-fuzzing.md - доки по мутациям от LibFuzzer с примером png
+ https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/custom_mutators.md - доки по мутациям от AFL++ с примером как использовать.

![[images/20240714144618.png]]

*Простыми словами:* файл в формате [GIF](https://ru.wikipedia.org/wiki/GIF) состоит из фиксированной области в начале файла, за которой располагается переменное число блоков, и заканчивается файл завершителем изображения.

*На деле:* в нем есть области которые задают общие параметры изображения, параметры анимации и еще очень много параметров. Есть глобальная таблица цветов, локальная(опционально) а также данные изображения. 

![[images/20240716205323.png]]

Самая интересная область это Image Data это информация об изображении(состояниях пикселей), которая хранится в виде блоков байтов.

Этот блок байт - результат сжатия LZW алгоритмом изначального блока цветов(последовательности индексов цветов 1,2,2,0,2,4).

**Мутация**

Моя мутация генерирует GIF файл, при этом файл может быть валидным или нет.
В нем случайным образом генерятся размеры холста, сдвиги слоя, кол-во картинок для анимации, цвета и данные изображение. 

При этом данные изображения генерятся изначально в виде последовательности индексов цвета, а затем сжимаются по LZW алгоритму(сделал упрощенную его реализацию по [статье](https://www.matthewflickinger.com/lab/whatsinagif/lzw_image_data.asp)). Поэтому несмотря на рандомизацию многих параметров, данные изображения всегда будут валидными! 

Но в комбинации результат получается очень интересным и разнообразным(на фото ручной прогон сгенеренного файла в chafa) от бесконечной гифки до пустого места/failed to open.

Поэтому такая генерация изображений вместе с мутациями AFL++ увеличивает покрытие.

![[images/20240716210400.png]]

Сборка с помощью цели `make mutation`

**Запуск с кастомной мутацией:**

```bash
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1
export AFL_USE_ASAN=1
export AFL_CUSTOM_MUTATOR_LIBRARY="../gif_mutation.so"
afl-fuzz -i input -o output_3 -- /usr/local/bin/chafa --animate=off @@
```
Итог:

**Запуск с кастомной мутацией и словарем:**















## Ошибки и материалы

### AFL

[AFL](https://github.com/google/AFL), [инструкция](https://github.com/google/AFL/blob/master/docs/INSTALL).

**Система**: Linux Mint 21.3 Cinnamon Intel i5-12450H

**Ошибки**:
```bash
[-] Hmm, your system is configured to send core dump notifications to an
    external utility. This will cause issues: there will be an extended delay
    between stumbling upon a crash and having this information relayed to the
    fuzzer via the standard waitpid() API.

    To avoid having crashes misinterpreted as timeouts, please log in as root
    and temporarily modify /proc/sys/kernel/core_pattern, like so:

    echo core >/proc/sys/kernel/core_pattern

[-] PROGRAM ABORT : Pipe at the beginning of 'core_pattern'
         Location : check_crash_handling(), afl-fuzz.c:7347

```
Исправляю, почитав об ошибке [ответ разраба](https://stackoverflow.com/questions/35441062/afl-fuzzing-without-root-avoid-modifying-proc-sys-kernel-core-pattern)(не пользовался AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES [форум](https://groups.google.com/g/afl-users/c/7arn66RyNfg/m/BsnOPViuCAAJ)). 
```bash
sudo su -   
echo core >/proc/sys/kernel/core_pattern
sudo su user
```

```bash
[-] Whoops, your system uses on-demand CPU frequency scaling, adjusted
    between 390 and 4296 MHz. Unfortunately, the scaling algorithm in the
    kernel is imperfect and can miss the short-lived processes spawned by
    afl-fuzz. To keep things moving, run these commands as root:

    cd /sys/devices/system/cpu
    echo performance | tee cpu*/cpufreq/scaling_governor

    You can later go back to the original state by replacing 'performance' with
    'ondemand'. If you don't want to change the settings, set AFL_SKIP_CPUFREQ
    to make afl-fuzz skip this check - but expect some performance drop.

[-] PROGRAM ABORT : Suboptimal CPU scaling governor
         Location : check_cpu_governor(), afl-fuzz.c:7409
```
Исправил с помощью команды.
```bash
[-]  SYSTEM ERROR : Unable to create './findings_dir/queue/id:000000,orig:example1'
    Stop location : link_or_copy(), afl-fuzz.c:2959
       OS message : Invalid argument
```
Исправил по [ответу](https://groups.google.com/g/afl-users/c/oJUGxCUg4Vo) разработчика, переустановил с `#define SIMPLE_FILES`. 

### LibFuzzer и AFL++

LibFuzzer:
+ https://llvm.org/docs/LibFuzzer.html
+ https://github.com/google/fuzzing/tree/master
+ https://github.com/google/fuzzing/blob/master/docs/structure-aware-fuzzing.md
+ https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
AFL++:
+ https://github.com/AFLplusplus/AFLplusplus
+ https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/fuzzing_in_depth.md
+ https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/tutorials.md
+ https://habr.com/ru/articles/772156/