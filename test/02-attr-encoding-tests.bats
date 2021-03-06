#!/usr/bin/env bats

BD=${BATS_TEST_DIRNAME}/..
DD=${BATS_TEST_DIRNAME}/data


@test "attr encoding: set short message" {
    message="hello world"
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -am "${message}" | ${BD}/stenomesh -ax)

    # Verify decoded value
    [ "${result}" == "${message}" ]
}

@test "attr encoding: max size message" {
    # 12 face cube -> 24 byte attr space -> 4 byte length prefix -> 20 byte encoding space
    message=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 20 | head -n 1)
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -am "${message}" | ${BD}/stenomesh -ax)

    # Verify
    [ "${result}" == "${message}" ]
}

@test "attr encoding: overflow message" {
    # 12 face cube -> 24 byte attr space -> 4 byte length prefix -> 20 byte encoding space
    message=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 21 | head -n 1)
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -am "${message}" | ${BD}/stenomesh -ax)

    # Verify not equal to original
    [ "${result}" != "${message}" ]
}

@test "attr encoding: ignore overflow message" {
    # 12 face cube -> 24 byte attr space -> 4 byte length prefix -> 20 byte encoding space
    message=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 21 | head -n 1)
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -aim "${message}" | ${BD}/stenomesh -ax)

    # Verify not equal to original
    [ "${result}" != "${message}" ]
    msg_clip=$(echo $message | fold -w 20 | head -n 1)
    echo $result
    echo $msg_clip
    [ "${result}" == "${msg_clip}" ]
}

@test "attr encoding: message from file" {
    message="$(<${DD}/message.txt)"
    result=$(cat ${DD}/cube_bin.ply | ${BD}/stenomesh -af ${DD}/message.txt | ${BD}/stenomesh -ax)

    # Verify decoded value
    [ "${result}" == "${message}" ]
}
