#!/usr/bin/env bats

BD=${BATS_TEST_DIRNAME}/..
DD=${BATS_TEST_DIRNAME}/data

@test "convert: ascii ply to stl" {
    result=$(cat ${DD}/cube_ascii.ply | ${BD}/stenomesh | sha1sum | awk '{print $1}')

    # Verify
    [ $result == "60dc86e7d24543843eb2eb3518b409e2ea08858a" ]
}

@test "convert: binary ply to stl" {
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh | sha1sum | awk '{print $1}')

    # Verify
    [ $result == "5499e4a0e74bc4e0ed09ee92bcf0e35285aa2437" ]
}

@test "convert: no header message" {
    # cube_bin.ply contains a "VCGLIB generated" comment
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh | head -c 16)

    # Verify original comment in first 16 bytes of binary STL file
    # (first 80 bytes of binary STL file is reserved for arbitrary comments)
    [ "${result}" == "VCGLIB generated" ]
}

@test "convert: set short header message" {
    # Override header message explicitly
    header_message="hello world"
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -h "${header_message}" | head -c 11)

    # Verify original comment in first 11 bytes of binary STL file
    # (first 80 bytes of binary STL file is reserved for arbitrary comments)
    [ "${result}" == "${header_message}" ]
}

@test "convert: max size header message" {
    # Override header message with max size STL header
    header_message=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 80 | head -n 1)
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -h "${header_message}" | head -c 80)

    # Verify
    [ "${result}" == "${header_message}" ]
}

@test "convert: overflow header message" {
    # Override header message with larger than max size STL header
    header_message=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 81 | head -n 1)
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -h "${header_message}" | head -c 81)

    # Verify not equal to original
    [ "${result}" != "${header_message}" ]
    # Verify match of first 80 bytes
    [ "$(echo $result | head -c 80)" == "$(echo $header_message | head -c 80)" ]
}

@test "convert: scale" {
    # Create named pipes for communication
    stl_out=$(mktemp -t stenomesh.test.out.XXXXXXXXX.stl --dry-run)

    cat ${DD}/cube_bin.ply | ${BD}/stenomesh -s 1000 > $stl_out
    result=$(echo '<!DOCTYPE FilterScript><FilterScript><xmlfilter name="Compute Geometric Measures"/></FilterScript>' | meshlabserver -i $stl_out -s /dev/stdin 2> /dev/null)

    [[ $result == *"Bounding Box Size 1000.000000  1000.000000  1000.000000"* ]]
}
