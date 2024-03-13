# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(as3 ${MY_SOURCES})


set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(ALSA REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(as3 LINK_PRIVATE hal asound Threads::Threads)

# Copy executable to final location (change `as3` to project name as needed)
add_custom_command(TARGET as3 POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:as3>"
     "~/cmpt433/public/myApps/as3" 
  COMMENT "Copying ARM executable to public NFS directory")
