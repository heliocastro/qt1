include(CMakeParseArguments)

function(qt1_wrap_cpp source_files)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(mocable ${arg_SOURCES})
        get_filename_component(realfile ${mocable} ABSOLUTE)
        get_filename_component(outfileName ${mocable} NAME_WE)
        add_custom_command(
            OUTPUT ${outfileName}.moc
            COMMAND moc-qt1 ${realfile} -o ${outfileName}.moc
        )
        # Check if header really generates output
        list(APPEND ${outFiles} ${outfileName}.moc)
        list(APPEND ${source_files} ${CMAKE_CURRENT_BINARY_DIR}/${outfileName}.moc)
    endforeach()

    set(${source_files} ${${source_files}} PARENT_SCOPE)
endfunction()

function(qt1_wrap_moc mocable_files)
    set(options)
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(mocable ${arg_SOURCES})
        get_filename_component(realfile ${mocable} ABSOLUTE)
        get_filename_component(outfileName ${mocable} NAME_WE)
        file(STRINGS ${realfile} Q_OBJECT REGEX "Q_OBJECT")
        if(Q_OBJECT)
            add_custom_command(
                OUTPUT moc_${outfileName}.cpp
                COMMAND moc-qt1 ${realfile} -o moc_${outfileName}.cpp
            )
            # Check if header really generates output
            list(APPEND ${outFiles} moc_${outfileName}.cpp)
            list(APPEND ${mocable_files} ${CMAKE_CURRENT_BINARY_DIR}/moc_${outfileName}.cpp)
        endif()
    endforeach()

    set(${mocable_files} ${${mocable_files}} PARENT_SCOPE)
endfunction()
