add_definitions(-DUNICODE -D_UNICODE)
set(APP_NAME tabbed)
add_executable(${APP_NAME} WIN32 tabbed.cpp)
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)

target_include_directories(${APP_NAME} PUBLIC ${WinSDK})

target_link_libraries(${APP_NAME} PRIVATE user32)
target_link_libraries(${APP_NAME} PRIVATE gdi32)

target_link_libraries(${APP_NAME} PRIVATE autocomplete)
target_link_libraries(${APP_NAME} PRIVATE gui)
target_link_libraries(${APP_NAME} PRIVATE automation)
