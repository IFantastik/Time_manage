# Time_manage: Desktop-приложение для учёта рабочего времени

**Time_manage** — это Desktop-приложение, разработанное на **Qt**, предназначенное для эффективного учёта рабочего времени и ведения заметок. Все данные надёжно хранятся в базе данных **PostgreSQL**.

## Быстрый запуск (Windows Release)

Для использования приложения на Windows не требуется установка Qt. Предоставлен готовый к запуску архив.

1.  **Скачайте архив:** Перейдите в раздел [**Releases**](https://github.com/IFantastik/Time_manage/releases/tag/v1) данного репозитория.
2.  Скачайте архив вида: `Time_manage_v1.zip`.
3.  **Распакуйте архив** в любую удобную папку (например: `C:\Time_manage\`).
4.  **Настройте подключение:** Откройте файл `config.ini`, который находится рядом с исполняемым файлом (`Time_manage.exe`), и укажите параметры подключения к вашей базе данных:

    ```ini
    [db]
    host=localhost
    name=timeman
    user=postgres
    password=your_password
    ```

5. Создайте базу данных и таблицы(по примеру снизу)
5.  **Запуск:** Сохраните файл `config.ini` и запустите `Time_manage.exe`.

> **Примечание:** Qt и все необходимые библиотеки включены в архив. Требуется только запущенный сервер PostgreSQL.

---

## Требования

Для корректной работы приложения необходимо выполнение следующих условий:

*   Установленный и запущенный сервер PostgreSQL (локальный или удалённый).
*   **Минимальная версия PostgreSQL:** 12+.
*   Созданная база данных с именем `timeman`.
*   Созданные необходимые таблицы (см. ниже).
*   Корректно заполненный файл `config.ini`.

---

## 🗄️ Создание Базы Данных и Таблиц

Вы можете использовать любой удобный инструмент, такой как `psql` или `pgAdmin`.

### 1. Создание базы данных

```sql
CREATE DATABASE timeman;
```

### 2. Подключение к базе

```sql
\c timeman
```

### 3. Создание таблиц

```sql
CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    login TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    name TEXT,
    mail TEXT,
    photo TEXT,
    money_for_hour INT NOT NULL DEFAULT 0
);

CREATE TABLE time_manage (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(user_id),
    work_date DATE NOT NULL,
    start_time TIME NOT NULL,
    end_time TIME NOT NULL,
    note TEXT
);
```

### После создания таблиц приложение готово к работе.

## 🛠 Сборка из исходников

### 📋 Требования

- Qt 6.x (модули: Widgets, Sql, PrintSupport)
- MinGW 64-bit
- CMake 3.16+

---

### 🚀 Сборка через Qt Creator

1. Открыть проект (`CMakeLists.txt`)
2. Выбрать Kit: **Desktop Qt 6.x MinGW 64-bit**
3. Выбрать конфигурацию **Debug** или **Release**
4. Выполнить **Run CMake**
5. Выполнить **Build Project**

---

## ⚙ Настройка подключения к базе данных

После сборки необходимо:

1. Создать базу данных
2. Убедиться, что таблицы созданы
3. Добавить файл `config.ini`:

```ini
[db]
host=localhost
name=timeman
user=postgres
password=your_password
```
