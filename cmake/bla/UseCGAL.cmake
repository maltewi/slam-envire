find_package(BLAS)
find_package(LAPACK)
find_package(CGAL)

if (BLAS_FOUND AND LAPACK_FOUND AND CGAL_FOUND)
    add_definitions( ${BLAS_DEFINITIONS} )
    link_directories( ${BLAS_LIBRARIES_DIR} )
    link_libraries( ${BLAS_LIBRARIES} )

    add_definitions( ${LAPACK_DEFINITIONS} )
    link_directories( ${LAPACK_LIBRARIES_DIR} )
    link_libraries( ${LAPACK_LIBRARIES} )

    find_package(TAUCS)
    if(TAUCS_FOUND)
        add_definitions( ${TAUCS_DEFINITIONS} )
        link_directories( ${TAUCS_LIBRARIES_DIR} )
        link_libraries( ${TAUCS_LIBRARIES} )
    endif(TAUCS_FOUND)

    find_package(GMP)
    if(GMP_FOUND)
        add_definitions( ${GMP_DEFINITIONS} )
        link_directories( ${GMP_LIBRARIES_DIR} )
        link_libraries( ${GMP_LIBRARIES} )
    endif(GMP_FOUND)

    add_definitions(${CGAL_DEFINITIONS})
    link_directories(${CGAL_LIBRARIES})
    add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)
    if ( "${BLAS_DEFINITIONS}" MATCHES ".*BLAS_USE_F2C.*" )
        add_definitions( "-DCGAL_USE_F2C" )
    endif()
    if ( "${LAPACK_DEFINITIONS}" MATCHES ".*LAPACK_USE_F2C.*" )
        add_definitions( "-DCGAL_USE_F2C" )
    endif()

    set(CGAL_LIBRARIES ${GMP_LIBRARIES} ${CGAL_LIBRARIES} ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
else()
    set(CGAL_FOUND FALSE)
endif()