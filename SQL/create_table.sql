DROP TABLE IF EXISTS kline;

CREATE TABLE kline (
    tstamp BIGINT,
    symbol VARCHAR(12),
    open REAL,
    min  REAL,
    max REAL,
    close REAL,
    PRIMARY KEY(symbol, tstamp)
);


