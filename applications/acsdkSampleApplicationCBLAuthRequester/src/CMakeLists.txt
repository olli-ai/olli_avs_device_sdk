add_definitions("-DACSDK_LOG_MODULE=acsdkSampleApplicationCBLAuthRequester")
add_library(acsdkSampleApplicationCBLAuthRequester SHARED
    SampleApplicationCBLAuthRequester.cpp)

target_include_directories(acsdkSampleApplicationCBLAuthRequester PUBLIC
    "${acsdkSampleApplicationCBLAuthRequester_SOURCE_DIR}/include")

target_link_libraries(acsdkSampleApplicationCBLAuthRequester PUBLIC
    acsdkSampleApplicationInterfaces
    acsdkAuthorizationInterfaces
    CBLAuthDelegate)

# install target
asdk_install()
