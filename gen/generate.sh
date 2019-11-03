#!/bin/sh
glslangValidator -V -x -o instance_filled.vert.spv.inl instance_filled.vert.glsl
glslangValidator -V -x -o instance_filled_mv.vert.spv.inl -DMULTIVIEW instance_filled.vert.glsl
glslangValidator -V -x -o instance_wire.vert.spv.inl instance_wire.vert.glsl
glslangValidator -V -x -o instance_wire_mv.vert.spv.inl -DMULTIVIEW instance_wire.vert.glsl
glslangValidator -V -x -o array_filled.vert.spv.inl array_filled.vert.glsl
glslangValidator -V -x -o array_filled_mv.vert.spv.inl -DMULTIVIEW array_filled.vert.glsl
glslangValidator -V -x -o array_wire.vert.spv.inl array_wire.vert.glsl
glslangValidator -V -x -o array_wire_mv.vert.spv.inl -DMULTIVIEW array_wire.vert.glsl
glslangValidator -V -x -o filled.frag.spv.inl filled.frag.glsl
glslangValidator -V -x -o wire.frag.spv.inl wire.frag.glsl
{
	echo "/*"
	cat instance_filled.vert.glsl
	echo "*/"
	echo "static uint32_t const g_imdd_vulkan_spv_instance_filled_vert[] = {"
	cat instance_filled.vert.spv.inl
	echo "};"
	echo "static uint32_t const g_imdd_vulkan_spv_instance_filled_mv_vert[] = {"
	cat instance_filled_mv.vert.spv.inl
	echo "};"
	echo
	echo "/*"
	cat instance_wire.vert.glsl
	echo "*/"
	echo "static uint32_t const g_imdd_vulkan_spv_instance_wire_vert[] = {"
	cat instance_wire.vert.spv.inl
	echo "};"
	echo "static uint32_t const g_imdd_vulkan_spv_instance_wire_mv_vert[] = {"
	cat instance_wire_mv.vert.spv.inl
	echo "};"
	echo
	echo "/*"
	cat array_filled.vert.glsl
	echo "*/"
	echo "static uint32_t const g_imdd_vulkan_spv_array_filled_vert[] = {"
	cat array_filled.vert.spv.inl
	echo "};"
	echo "static uint32_t const g_imdd_vulkan_spv_array_filled_mv_vert[] = {"
	cat array_filled_mv.vert.spv.inl
	echo "};"
	echo
	echo "/*"
	cat array_wire.vert.glsl
	echo "*/"
	echo "static uint32_t const g_imdd_vulkan_spv_array_wire_vert[] = {"
	cat array_wire.vert.spv.inl
	echo "};"
	echo "static uint32_t const g_imdd_vulkan_spv_array_wire_mv_vert[] = {"
	cat array_wire_mv.vert.spv.inl
	echo "};"
	echo
	echo "/*"
	cat filled.frag.glsl
	echo "*/"
	echo "static uint32_t const g_imdd_vulkan_spv_filled_frag[] = {"
	cat filled.frag.spv.inl
	echo "};"
	echo
	echo "/*"
	cat wire.frag.glsl
	echo "*/"
	echo "static uint32_t const g_imdd_vulkan_spv_wire_frag[] = {"
	cat wire.frag.spv.inl
	echo "};"
} >generated.h
