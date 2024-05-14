set(MLD_SRC "mld.c")

add_library(${MLD_LIB} STATIC
				${MLD_SRC})

target_include_directories(${MLD_LIB} PUBLIC
								${HEADER_DIR})