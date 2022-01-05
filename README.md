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



