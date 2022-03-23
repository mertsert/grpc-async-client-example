    # ==Build Builder Docker Image===
    echo
    echo ---------------------------------------------
    echo Building docker image
    echo to build App using CMake.
    echo ---------------------------------------------

    BUILD_IMAGE_NAME="build-kit"

    docker build -t $BUILD_IMAGE_NAME ./docker

    docker run -t --rm --name "$BUILD_IMAGE_NAME"\
    -v `pwd`:`pwd` \
    -w `pwd` \
    $BUILD_IMAGE_NAME \
    /bin/bash -c " \
        mkdir -p .build 
        cd .build
        cmake ..
        cmake --build .
        cd ..
    "