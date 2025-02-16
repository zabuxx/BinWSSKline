cmake_minimum_required(VERSION 3.10.0)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

#ADD_DEFINITIONS(-DBOOST_BIND_GLOBAL_PLACEHOLDERS) 
set(CMAKE_CXX_STANDARD 20)


#find_package(Threads REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pipe -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pipe -Ofast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb -pipe -Ofast -march=skylake -mmmx -mno-3dnow -msse -msse2 -msse3 -mssse3 -mno-sse4a -mcx16 -msahf -mmovbe -maes -mno-sha -mpclmul -mpopcnt -mno-lwp -mfma -mno-fma4 -mno-xop -mbmi -msgx -mbmi2 -mno-pconfig -mno-wbnoinvd -mno-tbm -mavx -mavx2 -msse4.2 -msse4.1 -mlzcnt -mno-rtm -mrdrnd -mf16c -mfsgsbase -mrdseed -mprfchw -madx -mfxsr -mxsave -mxsaveopt -mno-avx512f -mno-avx512bw -mno-avx512cd -mno-avx512f -mno-prefetchi -mclflushopt -mxsavec -mxsaves -mno-avx512dq -mno-avx512bw -mno-avx512vl -mno-avx512ifma -mno-avx512vbmi -mno-clwb -mno-mwaitx -mno-clzero -mno-pku -mno-rdpid -mno-gfni -mno-shstk -mno-avx512vbmi2 -mno-avx512vnni -mno-vaes -mno-vpclmulqdq -mno-avx512bitalg -mno-avx512vpopcntdq -mno-movdiri -mno-movdir64b -mno-waitpkg -mno-cldemote -mno-ptwrite -mno-avx512bf16 -mno-enqcmd -mno-avx512vp2intersect --param l1-cache-size=32 --param l1-cache-line-size=64 --param l2-cache-size=8192 -mtune=skylake")

find_package(OpenSSL REQUIRED)
#find_package(Pagmo REQUIRED)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost COMPONENTS log program_options iostreams system  REQUIRED)

add_executable(WSKline
    ${BOOST_BEAST_FILES}
    main_kline.cpp
    HttpClientBase.cpp
    Options.cpp
    WSBase.cpp
)


target_include_directories(WSKline PRIVATE common )
target_link_libraries(WSKline LINK_PUBLIC ${Boost_LIBRARIES} OpenSSL::SSL "pqxx")
