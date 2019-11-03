cmake_minimum_required(VERSION 2.8)
project(imdd)

if(UNIX)
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif(NOT CMAKE_BUILD_TYPE)

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")

	set(GLFW_LIBS glfw)
	set(STD_LIBS m)
	set(GL_LIBS GL)
	set(VK_LIBS vulkan)
else(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	set(GLFW_PATH "../glfw" CACHE PATH "Path to GLFW library")
	include_directories(${GLFW_PATH}/include)
	if(MSVC_VERSION GREATER_EQUAL 1920)
		link_directories(${GLFW_PATH}/lib-vc2019)
	elseif(MSVC_VERSION GREATER_EQUAL 1910)
		link_directories(${GLFW_PATH}/lib-vc2017)
	elseif(MSVC_VERSION EQUAL 1900)
		link_directories(${GLFW_PATH}/lib-vc2015)
	elseif(MSVC_VERSION EQUAL 1800)
		link_directories(${GLFW_PATH}/lib-vc2013)
	elseif(MSVC_VERSION EQUAL 1700)
		link_directories(${GLFW_PATH}/lib-vc2012)
	else(MSVC_VERSION GREATER_EQUAL 1920)
		message(FATAL_ERROR "unknown MSC_VERSION for GLFW")
	endif(MSVC_VERSION GREATER_EQUAL 1920)

	set(VULKAN_PATH "../vulkan" CACHE PATH "Path to Vulkan SDK")
	include_directories(${VULKAN_PATH}/Include)
	link_directories(${VULKAN_PATH}/Lib)

	set(GLFW_LIBS glfw3)
	set(GL_LIBS opengl32)
	set(VK_LIBS vulkan-1)
endif(UNIX)

set(IMDD_HDR
	imdd.h
	imdd_atomic.h
	imdd_draw_util.h
	imdd_simd.h
	imdd_simd_fallback.h
	imdd_simd_sse.h
	imdd_store.h
	)

add_executable(example_gl3
	example_gl3.c
	example_common.c
	gl_core_3_3.c
	gl_core_3_3.h
	${IMDD_HDR}
	imdd_draw_gl3.h
	)
target_link_libraries(example_gl3 ${GL_LIBS} ${GLFW_LIBS} ${STD_LIBS})

add_executable(example_vulkan
	example_vulkan.c
	example_common.c
	${IMDD_HDR}
	imdd_draw_vulkan.h
	)
target_link_libraries(example_vulkan ${VK_LIBS} ${GLFW_LIBS} ${STD_LIBS})

add_executable(compile_test_gl3
	compile_test_gl3.cpp
	gl_core_3_3.c
	gl_core_3_3.h
	${IMDD_HDR}
	imdd_draw_gl3.h
	)
target_link_libraries(compile_test_gl3 ${GL_LIBS})

add_executable(compile_test_vulkan
	compile_test_vulkan.cpp
	${IMDD_HDR}
	imdd_draw_vulkan.h
	)
target_link_libraries(compile_test_vulkan ${VK_LIBS})
