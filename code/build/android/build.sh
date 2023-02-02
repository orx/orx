#!/bin/bash

./gradlew build publishToMavenLocal

mkdir -p ../../lib/static/android/repository/org/orx-project
cp -rf ~/.m2/repository/org/orx-project ../../lib/static/android/repository/org

exit 0
