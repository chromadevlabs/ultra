cmake_minimum_required(VERSION 3.12)
project(ultra VERSION 0.0.1)

add_subdirectory(dependencies/JUCE)
add_subdirectory(dependencies/capstone-master)

set(CMAKE_CXX_STANDARD 20)

juce_add_gui_app(${PROJECT_NAME}
	PRODUCT_NAME "Ultra"
)

target_sources(${PROJECT_NAME}
	PRIVATE
		main.cpp
		cpu.cpp
		memory.cpp
		cpu_instructions.cpp
		disassembler.cpp
		tests.cpp
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		dependencies/capstone-master/include
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${PROJECT_NAME},JUCE_PRODUCT_NAME>"
		JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${PROJECT_NAME},JUCE_VERSION>"

		JUCE_OPENGL3=1
		JUCE_WEB_BROWSER=0
		JUCE_USE_CURL=0
		JUCE_DISPLAY_SPLASH_SCREEN=0
		JUCE_ASIO=0
		JUCE_WASAPI=1
		JUCE_DIRECTSOUND=0
		JUCE_JACK=0
		JUCE_ALSA=1
)

#file(GLOB_RECURSE ASSET_FILES ./assets *.png;*.ttf;*.svg)
#juce_add_binary_data(BinaryData SOURCES ${ASSET_FILES})
#target_compile_options(BinaryData PRIVATE "-fPIC")

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		#BinaryData
		juce::juce_core
		juce::juce_gui_extra
		juce::juce_opengl
		juce::juce_audio_basics
		juce::juce_audio_devices
		capstone-static
	PUBLIC
		juce::juce_recommended_config_flags
		#juce::juce_recommended_lto_flags
		#juce::juce_recommended_warning_flags
)
