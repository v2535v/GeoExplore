#!/bin/bash
#
#    File:    build.sh
#    Author:  Marvin Smith
#    Date:    4/12/2014
#
#    Purpose:  Installation utility for Unix-like systems.
#

#---------------------------------#
#-      Usage Instructions       -#
#---------------------------------#
usage(){

    echo "usage: $0 [flag] [options]"
    echo ''
    echo '    flags:'
    echo ''
    echo '    -h, --help    : Print usage instructions.'
    echo '    -m, --make    : Build GeoExplore.'
    echo '    -t, --test    : Build and run unit tests.'
    echo '    -i, --install : Deploy GeoExplore to $PREFIX'
    echo "                    Note: PREFIX=$PREFIX"
    echo "    -v, --verbose : Show all build commands"
    echo '    --headers     : Create deployable include directory.'
    echo '    -c, --clean   : Clean up GeoExplore builds.'
    echo ''
    echo '    options:'
    echo ''
    echo '    -j <int>        : Set number of threads to build with.'
    echo '    --PREFIX <PREFIX> : Set Prefix.'
    echo '    --release       : Build release version.'
    echo '    --debug         : Build debug version.'
    echo ''

}


#---------------------------------------------------#
#-          Install GeoExplore Software            -#
#---------------------------------------------------#
install_software(){

    echo "-> installing GeoExplore to $PREFIX"
    
    # make sure binary directory exists
    if [ ! -d "$PREFIX/bin" ]; then mkdir -p $PREFIX/bin; fi
    
    # make sure the library directory exists
    if [ ! -d "$PREFIX/lib" ]; then mkdir -p $PREFIX/lib; fi

    # copy binary files
    cp $BUILD_TYPE/bin/* $PREFIX/bin

    # copy libs
    cp $BUILD_TYPE/lib/libGeoExplore* $PREFIX/lib/

}


#---------------------------------------------#
#-        Build GeoExplore Software          -#
#---------------------------------------------#
build_software(){

    #  Get the cmake file
    CMAKE_LOCATION="$1"

    #  Get the build type
    BUILD_TYPE="$2"

    #  Get the make arguments 
    MAKE_ARGS="$3"
    
    #  Get the build directory
    BUILD_DIRECTORY="${BUILD_TYPE}/${4}"

    #  Create the directory
    mkdir -p ${BUILD_DIRECTORY}

    #  Enter directory
    pushd ${BUILD_DIRECTORY}

    #  Run CMake
    if [ "$BUILD_VERBOSE" = '1' ]; then
        echo "cmake ${CMAKE_LOCATION}"
    fi
    cmake ${CMAKE_LOCATION}
    if [ ! "$?" = '0' ]; then
        echo 'error: CMake encountered error. Please see output.' 1>&2;
        exit 1
    fi

    #  Run Make
    make ${MAKE_ARGS}
    if [ ! "$?" = '0' ]; then
        echo 'error: Make encountered an error. Please see output.' 1>&2;
        exit 1
    fi

    #  Exit directory
    popd
}

#----------------------------#
#-       Test Software      -#
#----------------------------#
test_software(){
    
    #  Get the cmake file
    CMAKE_LOCATION="$1"

    #  Get the build type
    BUILD_TYPE="$2"

    #  Get the make arguments 
    MAKE_ARGS="$3"
    
    #  Get the build directory
    BUILD_DIRECTORY="${BUILD_TYPE}/${4}"

    #  Create the directory
    mkdir -p ${BUILD_DIRECTORY}

    #  Enter directory
    pushd ${BUILD_DIRECTORY}

    #  Run CMake
    cmake ${CMAKE_LOCATION}
    if [ ! "$?" = '0' ]; then
        echo 'error: CMake encountered error. Please see output.' 1>&2;
        exit 1
    fi

    #  Run Make
    make ${MAKE_ARGS}
    if [ ! "$?" = '0' ]; then
        echo 'error: Make encountered an error. Please see output.' 1>&2;
        exit 1
    fi

    #  Run Unit Test
    ./geoexplore-unit-test
    
    #  Exit directory
    popd
    
    #  Test the geo-convert function
    #./tests/bash/apps/geo-convert-test.sh
}

#------------------------------------------#
#-        Copy GeoExplore Headers         -#
#------------------------------------------#
copy_headers(){

    #  
    echo '-> copying header files.'
    
    #  If argument 1 was specified, place include there
    BASE_DIR='include/GeoExplore'
    if [ ! "$1" = '' ]; then
        BASE_DIR="${1}/${BASE_DIR}"
    fi

    #  Create include directory
    mkdir -p $BASE_DIR
    cp src/cpp/GeoExplore.hpp    $BASE_DIR/../

    #  Copy Core Module
    mkdir -p $BASE_DIR/core
    cp src/cpp/core/*.hpp        $BASE_DIR/core/

    #  Copy Coordinate Module
    mkdir -p $BASE_DIR/coordinate
    cp src/cpp/coordinate/*.hpp  $BASE_DIR/coordinate/
    
    #  Copy Image Module
    mkdir -p $BASE_DIR/image      
    cp src/cpp/image/*.hpp       $BASE_DIR/image/
    
    #  Copy IO Module
    mkdir -p $BASE_DIR/io
    cp src/cpp/io/*.hpp          $BASE_DIR/io/

    #  Copy Utilities Module
    mkdir -p $BASE_DIR/utilities
    cp src/cpp/utilities/*.hpp   $BASE_DIR/utilities/

}

#--------------------------------------#
#-       Clean up GeoExplore          -#
#--------------------------------------#
clean_software(){

    #  Print a pretty header
    echo '-> cleaning software'

    #  Wipe out any release builds
    if [ -d 'release' ]; then
        echo '  -> removing release build.'
        rm -r release
    fi
    
    #  Wipe out any debug builds
    if [ -d 'debug' ]; then
        echo '  -> removing debug build.'
        rm -r debug
    fi

}


#-----------------------------------#
#-          Main Function          -#
#-----------------------------------#

#  Software installation path
PREFIX='/opt/local'

#  Default Flags
RUN_MAKE=0
RUN_INSTALL=0
COPY_HEADERS=0
RUN_CLEAN=0
RUN_TEST=0
BUILD_VERBOSE=0

BUILD_TYPE='release'
NUM_THREADS=1
MAKE_ARGS=

PREFIX_FLAG=0

#  Process Command-Line Arguments
for ARG in "$@"; do

    case $ARG in
        
        #   Print usage instructions
        '-h' | '--help' )
            usage
            exit 1
            ;;

        #   Clean GeoExplore
        '-c' | '--clean' )
            RUN_CLEAN=1
            ;;

        #   Test GeoExplore
        '-t' | '--test' )
            RUN_TEST=1
            ;;

        #   Make GeoExplore
        '-m' | '--make' )
            RUN_MAKE=1
            ;;

        #   Install GeoExplore
        '-i' | '--install' )
            RUN_INSTALL=1
            ;;
       
        #   Build verbose
        '-v' | '--verbose' | '--VERBOSE' )
            BUILD_VERBOSE=1
            THREAD_FLAG=0
            PREFIX_FLAG=0
            ;;

        #   Number of threads
        '-j' )
            THREAD_FLAG=1
            ;;
        
        #   Copy Header Files
        '--headers' )
            COPY_HEADERS=1
            ;;
        #  Set Prefix flag
        '--PREFIX' )
            PREFIX_FLAG=1
            ;;

        #   Other Responses
        *)
            
            #  If the user specified num threads
            if [ "$THREAD_FLAG" == '1' ]; then
                THREAD_FLAG=0;
                NUM_THREADS=$ARG

            #  If we need to grab the prefix
            elif [ "$PREFIX_FLAG" = '1' ]; then
                PREFIX_FLAG=0
                PREFIX=$ARG

            #  Otherwise, we have an error
            else
                echo "error:  Unknown option $ARG" 1>&2
                usage
                exit 1
            fi
            ;;
    esac
done


#---------------------------------------------------------#
#-     Make sure at least one run flag was provided      -#
#---------------------------------------------------------#
if [ "$RUN_MAKE" = '0' -a "$RUN_INSTALL" = '0' -a "$COPY_HEADERS" = '0' -a "$RUN_CLEAN" = '0' -a "$RUN_TEST" = '0' ]; then
    echo 'error: At least one flag must be provided.' 1>&2
    usage
    exit 1;
fi

#---------------------------------#
#-    Generate Make arguments    -#
#---------------------------------#
MAKE_ARGS=-j${NUM_THREADS}
if [ "${BUILD_VERBOSE}" = '1' ]; then
    MAKE_ARGS="${MAKE_ARGS} VERBOSE=1"
fi

#---------------------------------#
#-       Clean GeoExplore        -#
#---------------------------------#
if [ "$RUN_CLEAN" = '1' ]; then
    clean_software
fi


#------------------------------#
#-      Make GeoExplore       -#
#------------------------------#
if [ "$RUN_MAKE" = '1' ]; then

    copy_headers ${BUILD_TYPE}
    build_software '../install/lib'                         ${BUILD_TYPE}  "${MAKE_ARGS}"  '.'
    build_software '../../../install/apps/geo-convert'      ${BUILD_TYPE}  "${MAKE_ARGS}"  'apps/geo-convert'
    build_software '../../../install/apps/terrain-explore'  ${BUILD_TYPE}  "${MAKE_ARGS}" 'apps/terrain-explore'
    build_software '../../../install/apps/geo-info'         ${BUILD_TYPE}  "${MAKE_ARGS}"  'apps/geo-info'
    build_software '../../install/gui'                      ${BUILD_TYPE}  "${MAKE_ARGS}"  'gui'

fi

#------------------------------#
#-     Install GeoExplore     -#
#------------------------------#
if [ "$RUN_INSTALL" = '1' ]; then
    install_software
fi

#---------------------------#
#-     Test GeoExplore     -#
#---------------------------#
if [ "$RUN_TEST" = '1' ]; then
    test_software '../../install/test'  ${BUILD_TYPE} "${MAKE_ARGS}" 'test'
fi


#------------------------#
#-     Copy Headers     -#
#------------------------#
if [ "$COPY_HEADERS" = '1' ]; then
    copy_headers
fi



