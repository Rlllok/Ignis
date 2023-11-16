file(GLOB SOURCE_FILES *.cpp *.hpp)
file(GLOB HEADER_FILES *.h)

target_sources(ignis PRIVATE ${SOURCE_FILES} ${HEADER_FILES})