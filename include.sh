#!/usr/bin/env bash

MOD_MORPHSUMMON_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/" && pwd )"

DB_CHARACTERS_CUSTOM_PATHS+=(
        $MOD_MORPHSUMMON_ROOT"/data/sql/db-characters/"
)

DB_WORLD_CUSTOM_PATHS+=(
        $MOD_MORPHSUMMON_ROOT"/data/sql/db-world/"
)
