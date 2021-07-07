file(GLOB TEST_SRC_FILES
	"*.cpp"
)

add_executable(RPiCAN-Tests
	${TEST_SRC_FILES}
)

set(INCLUDE_PATH 
	../include/
)

include_directories(
	${INCLUDE_PATH}
)

add_test(NAME RPiCAN-Tests COMMAND RPiCAN-Tests)

target_link_libraries(RPiCAN-Tests PUBLIC RPiCAN-static gtest gmock)
