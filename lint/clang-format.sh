# SPDX-FileCopyrightText: 2025 Contributors to the Eclipse Foundation
#
# See the NOTICE file(s) distributed with this work for additional
# information regarding copyright ownership.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
#!/bin/bash

PROJECT_ROOT="$(realpath "$(dirname "$0")/../")"

if [ -n "$(which clang-format-13)" ]; then
    # NOTE: Using clang-format-13 in CI system, too
    FORMATTER=clang-format-13
elif [ -n "$(which clang-format)" ]; then
    echo "Did not find clang-format-13. Trying clang-format. Results may not"
    echo "match formatting in GitHub CI process."
    FORMATTER=clang-format
else
    echo "Could not find clang-format. Please make sure it is installed" 1>&2
    exit 2
fi

echo "Running $FORMATTER on all files in '$PROJECT_ROOT'"
shopt -s globstar

pushd "$PROJECT_ROOT" > /dev/null
for f in **/*.h **/*.cpp; do
	if [[ ! ("$f" =~ "build/") ]]; then
		echo
		echo "Checking file '$f'"
		$FORMATTER -i "$f"
	fi
done
popd > /dev/null
