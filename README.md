# Описание

Форматы и плагины для просмотра файлов игр серии Златогорье. Описание форматов в шаблонах .bt для программы 010Editor.

**Описание форматов**

 № | Формат файла       | Шаблон (010Editor)     |    Описание |
| :--- | :--------- | :----------- | :---------- |
| 1 | .nrm        | [NRM.bt](https://github.com/AlexKimov/goldenland-file-formats/blob/master/formats/010editor/NRM.bt)  |   архив игровых ресурсов Златогорье 1 |

    Как использовать шаблоны 010Editor
    1. Нужен 010Editor.
    2. Открыть нужный вам .lib файл в программе. Меню File - Open File...
    3. Применить скрипт decodeLIB.1sc, чтобы был доступ к зашифрованной таблице с расположением файлов внутри .lib. Меню Scripts - Open Script и Run Script.
    4. Применить шаблон LIB.bt. Меню Templates - Open Template и Run Template.

**Инструменты**

| № | Плагин       | Программа | Описание |  
| :--- | :--------- | :----------- | :---- | 
| 1 | [unpack_nrm.bms](https://github.com/AlexKimov/goldenland-file-formats/blob/master/scripts/quickbms/unpack_nrm.bms) | Quickbms | Распаковка файлов ресурсов .nrm  Златогорье 1 |

    Как использовать
    1. Нужен quickbms https://aluigi.altervista.org/quickbms.htm
    2. Для запуска в репозитории лежит bat файл с настройками, нужно открыть его и задать свои пути: до места, где находится quickbms, папки с игрой и места куда нужно сохранить результат.
    3. Запустить процесс через bat файл или вручную задава свои параметры. 
