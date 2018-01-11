add_definitions(
    -D__USE_GNU 
    -D_GNU_SOURCE
    -D_FILE_OFFSET_BITS=64
)


set(
    fskodo_LIB_SRCS
    fsal_kodo_main.c
    fsal_kodo_handle.c
    fsal_kodo_internal.c
    fsal_kodo_ds.c
)

add_library(fsalkodo MODULE ${fskodo_LIB_SRCS})
add_sanitizers(fsalkodo)

target_link_libraries(fsalkodo ${SYSTEM_LIBRARIES})

install(TARGETS fsalkodo COMPONENT fsal DESTINATION ${FSAL_DESTINATION} )