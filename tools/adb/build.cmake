list(APPEND CMAKE_MODULE_PATH "./tools/adb")
find_package(Graphviz)
if(GRAPHVIZ_FOUND)
	message("libgraphviz found, visualization enabled(CFLAGS=${GRAPHVIZ_INCLUDE_DIRS} LIBS=${GRAPHVIZ_LIBRARIES})")
	set(GRAPHVIZ_CFLAGS "-DWITH_GRAPHVIZ")
else(GRAPHVIZ_FOUND)
	message("libgraphviz not found, visualization disabled")
	set(GRAPHVIZ_CFLAGS "")
endif(GRAPHVIZ_FOUND)
find_library(LIB_RL NAMES readline)  
if("${LIB_RL}" STREQUAL "")
	set(READLINE_CFLAGS "")
	set(READLINE_LIBRARIES "")
else("${LIB_RL}" STREQUAL "")
	set(READLINE_CFLAGS "-DWITH_READLINE")
	set(READLINE_LIBRARIES ${LIB_RL})
endif("${LIB_RL}" STREQUAL "")

set(TYPE binary)
set(LOCAL_CFLAGS "-DDEBUGGER ${READLINE_CFLAGS} ${GRAPHVIZ_CFLAGS}")
set(LOCAL_LIBS adam_dbg readline ${READLINE_LIBRARIES} ${GRAPHVIZ_LIBRARIES})
