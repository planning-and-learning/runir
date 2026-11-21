# The database's benchmarks (runir::kr::database).
find_package(benchmark 1.8.3 REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(Threads REQUIRED)

foreach(benchmark incremental distance query)
    add_executable(database_${benchmark}_benchmark "kr/database/${benchmark}.cpp")
    target_link_libraries(database_${benchmark}_benchmark PRIVATE runir::kr benchmark::benchmark_main Threads::Threads)
endforeach()
