DROP TABLE IF EXISTS pairs;
CREATE TABLE pairs (
       symbol VARCHAR(32),
       base VARCHAR(32),
       quote VARCHAR(32),
       PRIMARY KEY(symbol)
       );


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


DROP TABLE IF EXISTS process_metadata;
CREATE TABLE process_metadata (	
   id SERIAL PRIMARY KEY,
   last_processed TIMESTAMP
);
INSERT INTO process_metadata (last_processed) VALUES ('1970-01-01 00:00:00');  -- Initialize wit
