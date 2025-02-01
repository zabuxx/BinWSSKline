DROP TABLE IF EXISTS kline;

CREATE TABLE kline (
    tstamp BIGINT,
    symbol VARCHAR(32),
    open REAL,
    min  REAL,
    max REAL,
    close REAL,
    mean REAL,
    PRIMARY KEY(symbol, tstamp)
);


