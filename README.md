# BinWSSKline
Use Binance WSS stream to store local custom candlestick DB with arbitrari length (up to 1s)

This C++ program subscribes to the Binance Web Socket Stream to update a local database with candlestick data for all actively traded spot market pairs.

# Dependencies

Required packages:
* PostgreSQL
* Boost >= 1.73


# Usage instructions

Install necessary Packagess (Fedora/CentOS):

    # dnf install cmake g++ clang
    # dnf install openssl-devel boost-devel libpqxx-devel
    # dnf install postgresql-server
    
Compile:

    (myuser)$ git clone https://github.com/zabuxx/BinWSSKline.git
    (myuser)$ cd BinWSSKline/
    (myuser)$ mkdir build
    (myuser)$ cd build
    (myuser)$ cmake ..
    (myuser)$ make

Prepare database:
    
    (postgres)$ pg_ctl init
    (postgres)$ pg_ctl start
    (postgres)$ createuser myuser
    (postgres)$ createdb exchange -O myuser
    (myuser)$ cat SQL/create_table.sql | psql exchange

Run:

    (myuser)$ ./build/src/WSKline -c prodnet.cfg -v5
   
 # Output
 
 The output is written to the configured database in the table kline. You can use this table for your TI calculations.
 
     exchange=> select * from kline where symbol =  'ETHBTC' order by tstamp;
       tstamp   | symbol |   open    |    min    |    max    |   close   
    ------------+--------+-----------+-----------+-----------+-----------
    1641410814 | ETHBTC |  0.081152 | 0.0811485 |  0.081152 | 0.0811485
    1641410824 | ETHBTC | 0.0811485 | 0.0811485 |  0.081161 | 0.0811555
    1641410834 | ETHBTC | 0.0811555 | 0.0811535 | 0.0811625 | 0.0811585
    1641410844 | ETHBTC | 0.0811585 | 0.0811565 | 0.0811635 | 0.0811565
    1641410854 | ETHBTC | 0.0811565 | 0.0811555 | 0.0811645 | 0.0811645
    1641410864 | ETHBTC | 0.0811645 | 0.0811645 | 0.0811835 | 0.0811815
    (6 rows)
 
 # Configuration
 
 Configuration happens with the config files, the syntax should be self-explaining
 
    psql=postgresql:///exchange
    logfile=/tmp/BinWSSKline-testnet.log
    logfile-lvl=5
    perf_detail=true
    
    [binance]
    api=api.binance.com
    wss=stream.binance.com
    wssport=9443

    [kline]
    cycle_time=2   # kline span in seconds


Only tested on Fedora, but using on other platforms should be easy.
