add_definitions("-DACSDK_LOG_MODULE=sensoryKeywordDetector")
add_library(SENSORY SHARED
    SensoryKeywordDetector.cpp)

target_include_directories(SENSORY PUBLIC
	"${KWD_SOURCE_DIR}/include" 
	"${SENSORY_SOURCE_DIR}/include"
    "${SENSORY_SOURCE_DIR}/olli/maika")
#   "${SENSORY_SOURCE_DIR}/olli/6.16.0")
#   "${SENSORY_SOURCE_DIR}/olli/6.11.0")
#	"${SENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR}")

#target_link_libraries(SENSORY KWD AVSCommon "${SENSORY_KEY_WORD_DETECTOR_LIB_PATH}")
#target_link_libraries(SENSORY KWD AVSCommon "${SENSORY_SOURCE_DIR}/olli/6.11.0/libsnsr.a")
#target_link_libraries(SENSORY KWD AVSCommon "${SENSORY_SOURCE_DIR}/olli/6.16.0/libsnsr.a")
target_link_libraries(SENSORY KWD AVSCommon "${SENSORY_SOURCE_DIR}/olli/maika/libsnsr.a")

# install target
asdk_install()