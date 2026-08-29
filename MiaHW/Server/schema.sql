CREATE TABLE IF NOT EXISTS game_server (
    idx INT PRIMARY KEY,
    ip VARCHAR(45) NOT NULL,
    port INT NOT NULL,
    registered_at DATETIME NOT NULL
);
