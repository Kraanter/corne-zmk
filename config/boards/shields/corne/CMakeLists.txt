if(CONFIG_CORNE_CUSTOM_DISPLAY)
  zephyr_library_named(corne_custom_display)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_sources(src/custom_status_screen.c)
endif()

# DEBUG ONLY: LED boot tracer for HW-954 clone bring-up
zephyr_library_named(corne_boot_trace)
zephyr_library_sources(src/boot_trace.c)
