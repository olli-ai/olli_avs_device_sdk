cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(KWD LANGUAGES CXX)

add_subdirectory("src")
add_subdirectory("test")

if(AMAZON_KEY_WORD_DETECTOR)
    add_subdirectory("Amazon")
endif()
if(AMAZONLITE_KEY_WORD_DETECTOR)
    add_subdirectory("AmazonLite")
endif()
if(SENSORY_KEY_WORD_DETECTOR)
    add_subdirectory("Sensory")
endif()

if(KWD)
    add_subdirectory("KWDProvider")
endif()
