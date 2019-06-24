#!/usr/bin/env bats

BD=${BATS_TEST_DIRNAME}/..
DD=${BATS_TEST_DIRNAME}/data

@test "convert: ascii ply to stl" {
    result=$(cat ${DD}/cube_ascii.ply | ${BD}/stenomesh | sha1sum | awk '{print $1}')

    # Verify
    [ $result == "abd05e321ee94d18f4a50b37e37c4b9bfbf3e2ae" ]
}

@test "convert: binary ply to stl" {
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh | sha1sum | awk '{print $1}')

    # Verify
    [ $result == "84d8602d88ef49516e17bcc5b863850fe545a7cf" ]
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
