#include "Database.h"

Database::Database(const std::string& connStr)
    : conn_(connStr) {
    // Если соединение не удалось — конструктор pqxx::connection бросит исключение
}

void Database::createTables() {
    pqxx::work txn(conn_);  // транзакция

    // Три таблицы: документы, слова, связь многие-ко-многим с частотой.
    txn.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS documents (
            id   SERIAL PRIMARY KEY,
            path TEXT UNIQUE NOT NULL
        );

        CREATE TABLE IF NOT EXISTS words (
            id   SERIAL PRIMARY KEY,
            word VARCHAR(32) UNIQUE NOT NULL
        );

        CREATE TABLE IF NOT EXISTS document_words (
            document_id INTEGER REFERENCES documents(id) ON DELETE CASCADE,
            word_id     INTEGER REFERENCES words(id)     ON DELETE CASCADE,
            frequency   INTEGER NOT NULL,
            PRIMARY KEY (document_id, word_id)
        );
    )SQL");

    txn.commit();
}

void Database::saveDocument(const std::string& path,
                            const std::map<std::string, int>& words) {
    // Блокируем мьютекс на всё время работы с соединением.
    // lock_guard сам разблокирует при выходе из функции.
    std::lock_guard<std::mutex> lock(mtx_);

    pqxx::work txn(conn_);

    // 1. Вставляем (или находим) документ, получаем его id.
    //    ON CONFLICT нужен, чтобы повторная индексация не падала.
    auto docRow = txn.exec_params1(
        "INSERT INTO documents(path) VALUES($1) "
        "ON CONFLICT(path) DO UPDATE SET path = EXCLUDED.path "
        "RETURNING id",
        path);
    int docId = docRow[0].as<int>();

    // 2. Удаляем старые связи этого документа (на случай переиндексации).
    txn.exec_params("DELETE FROM document_words WHERE document_id = $1", docId);

    // 3. Для каждого слова: вставляем слово, получаем id, пишем частоту.
    for (const auto& [word, freq] : words) {
        auto wordRow = txn.exec_params1(
            "INSERT INTO words(word) VALUES($1) "
            "ON CONFLICT(word) DO UPDATE SET word = EXCLUDED.word "
            "RETURNING id",
            word);
        int wordId = wordRow[0].as<int>();

        txn.exec_params(
            "INSERT INTO document_words(document_id, word_id, frequency) "
            "VALUES($1, $2, $3) "
            "ON CONFLICT(document_id, word_id) "
            "DO UPDATE SET frequency = EXCLUDED.frequency",
            docId, wordId, freq);
    }

    txn.commit();
}

std::vector<std::pair<std::string,int>>
Database::search(const std::vector<std::string>& words, int limit) {
    std::lock_guard<std::mutex> lock(mtx_);

    pqxx::work txn(conn_);

    // Собираем PostgreSQL-массив вида {"привет","мир"}
    std::string arr = "{";
    for (size_t i = 0; i < words.size(); ++i) {
        if (i) arr += ",";
        arr += "\"" + words[i] + "\"";
    }
    arr += "}";


    pqxx::result res = txn.exec_params(
        "SELECT d.path, SUM(dw.frequency) AS rank "
        "FROM documents d "
        "JOIN document_words dw ON dw.document_id = d.id "
        "JOIN words w           ON w.id = dw.word_id "
        "WHERE w.word = ANY($1) "
        "GROUP BY d.id, d.path "
        "HAVING COUNT(DISTINCT w.word) = $2 "
        "ORDER BY rank DESC "
        "LIMIT $3",
        arr,
        static_cast<int>(words.size()),
        limit);

    std::vector<std::pair<std::string,int>> out;
    for (auto row : res) {
        out.emplace_back(row["path"].c_str(), row["rank"].as<int>());
    }
    return out;
}
