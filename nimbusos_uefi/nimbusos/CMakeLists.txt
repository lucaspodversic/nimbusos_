# NimbusOS — build raíz (clean-room).
#   cmake -S . -B out -DNIMBUS_HOST=ON && cmake --build out -j && ctest --test-dir out --output-on-failure
cmake_minimum_required(VERSION 3.16)
project(NimbusOS C CXX)

option(NIMBUS_HOST "Build as host libraries for unit tests" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(NIMBUS_HOST=1)
include_directories(
  ${CMAKE_SOURCE_DIR}/kernel/include
  ${CMAKE_SOURCE_DIR}/kernel/host
  ${CMAKE_SOURCE_DIR}/hal
  ${CMAKE_SOURCE_DIR}/boot/uefi
  ${CMAKE_SOURCE_DIR}/tests/host
  ${CMAKE_SOURCE_DIR}/userland/include
  ${CMAKE_SOURCE_DIR}/drivers/include
)

# ---- Kernel ----
file(GLOB_RECURSE NIMBUS_KE_SOURCES CONFIGURE_DEPENDS
  kernel/ke/*.c kernel/mm/*.c kernel/ob/*.c kernel/ps/*.c
  kernel/io/*.c kernel/se/*.c kernel/cm/*.c kernel/cc/*.c
  kernel/alpc/*.c kernel/etw/*.c kernel/vbs/*.c kernel/main.c)
add_library(nimbus_kernel STATIC ${NIMBUS_KE_SOURCES})
target_compile_options(nimbus_kernel PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-pointer-sign)

file(GLOB NIMBUS_HAL_SOURCES CONFIGURE_DEPENDS hal/*.c hal/acpi/*.c hal/pci/*.c)
file(GLOB NIMBUS_BOOT_SOURCES CONFIGURE_DEPENDS boot/uefi/*.c boot/recovery/*.c)
add_library(nimbus_hal STATIC ${NIMBUS_HAL_SOURCES})
add_library(nimbus_boot STATIC ${NIMBUS_BOOT_SOURCES})
target_compile_options(nimbus_hal PRIVATE -Wall -Wno-unused-parameter)
target_compile_options(nimbus_boot PRIVATE -Wall -Wno-unused-parameter)

# ---- Userland Win32/ntdll (host) ----
file(GLOB_RECURSE NIMBUS_NTDLL_SOURCES CONFIGURE_DEPENDS userland/ntdll/*.c)
file(GLOB_RECURSE NIMBUS_K32_SOURCES CONFIGURE_DEPENDS userland/win32/kernel32/*.c)
file(GLOB_RECURSE NIMBUS_U32_SOURCES CONFIGURE_DEPENDS userland/win32/user32/*.c)
file(GLOB_RECURSE NIMBUS_GDI_SOURCES CONFIGURE_DEPENDS userland/win32/gdi32/*.c)
file(GLOB_RECURSE NIMBUS_ADV_SOURCES CONFIGURE_DEPENDS userland/win32/advapi32/*.c)
file(GLOB NIMBUS_WS2_SOURCES CONFIGURE_DEPENDS userland/win32/ws2_32/*.c)
file(GLOB_RECURSE NIMBUS_DX_SOURCES CONFIGURE_DEPENDS userland/directx/*/shim.c)
add_library(nimbus_userland STATIC
  ${NIMBUS_NTDLL_SOURCES} ${NIMBUS_K32_SOURCES} ${NIMBUS_U32_SOURCES}
  ${NIMBUS_GDI_SOURCES} ${NIMBUS_ADV_SOURCES} ${NIMBUS_WS2_SOURCES} ${NIMBUS_DX_SOURCES})
target_compile_options(nimbus_userland PRIVATE -Wall -Wno-unused-parameter)

# ---- Drivers (host skeletons) ----
file(GLOB_RECURSE NIMBUS_DRV_SOURCES CONFIGURE_DEPENDS
  drivers/storage/*.c drivers/gpu/*.c drivers/network/*.c drivers/network/*/*.c
  drivers/usb/*.c drivers/input/*.c drivers/audio/*.c drivers/power/*.c
  drivers/tpm/*.c drivers/virtio/*.c)
add_library(nimbus_drivers STATIC ${NIMBUS_DRV_SOURCES})
target_compile_options(nimbus_drivers PRIVATE -Wall -Wno-unused-parameter)
target_include_directories(nimbus_drivers PRIVATE ${CMAKE_SOURCE_DIR}/drivers/include)

enable_testing()

set(HOST_TESTS
  test_scheduler test_pe_parse test_registry test_zram
  test_acl test_handles test_memory_budget
  test_win32_pe test_win32_registry test_drivers_virtio
)

foreach(t ${HOST_TESTS})
  if(EXISTS ${CMAKE_SOURCE_DIR}/tests/host/${t}.c)
    add_executable(${t} tests/host/${t}.c)
    target_link_libraries(${t} PRIVATE nimbus_kernel nimbus_hal nimbus_boot nimbus_userland nimbus_drivers pthread)
    add_test(NAME ${t} COMMAND ${t})
  endif()
endforeach()

message(STATUS "NimbusOS: NIMBUS_HOST=${NIMBUS_HOST}")
