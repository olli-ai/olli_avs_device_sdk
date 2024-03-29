find_package(Threads ${THREADS_PACKAGE_CONFIG})

add_definitions("-DACSDK_LOG_MODULE=cblAuthDelegate")

message(WARNING "\nWARNING! Default SQLiteCBLAuthDelegateStorage is NOT encrypted. Your application's auth token storage MUST be encrypted for certification.\n")

add_library(CBLAuthDelegate SHARED
    CBLAuthDelegate.cpp
    SQLiteCBLAuthDelegateStorage.cpp)
target_include_directories(CBLAuthDelegate PUBLIC
    "${CBLAuthDelegate_SOURCE_DIR}/include")
target_include_directories(CBLAuthDelegate PRIVATE
    "${RAPIDJSON_INCLUDE_DIR}")
target_link_libraries(
    CBLAuthDelegate
    AVSCommon
    RegistrationManager
    SQLiteStorage
    acsdkAuthorization
    acsdkAuthorizationInterfaces
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

# install target
asdk_install()
