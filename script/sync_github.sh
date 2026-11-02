#!/usr/bin/env bash

# You do not need it, i use it for mirroring gitea with sha256 to github

SHADOW_DIR="tungsten-github"
find . -type d -path "./src*" -exec mkdir -p "${SHADOW_DIR}/{}" \;
find . -type f -path "./src*" -exec ln -f "{}" "${SHADOW_DIR}/{}" \;
find . -type d -path "./lib*" -exec mkdir -p "${SHADOW_DIR}/{}" \;
find . -type f -path "./lib*" -exec ln -f "{}" "${SHADOW_DIR}/{}" \;
find . -type d -path "./inc*" -exec mkdir -p "${SHADOW_DIR}/{}" \;
find . -type f -path "./inc*" -exec ln -f "{}" "${SHADOW_DIR}/{}" \;
find . -type d -path "./script*" -exec mkdir -p "${SHADOW_DIR}/{}" \;
find . -type f -path "./script*" -exec ln -f "{}" "${SHADOW_DIR}/{}" \;
ln -f conanfile.py ${SHADOW_DIR}/conanfile.py
ln -f CMakeLists.txt ${SHADOW_DIR}/CMakeLists.txt
ln -f LICENSE ${SHADOW_DIR}/LICENSE
ln -f README.md ${SHADOW_DIR}/README.md
ln -f .clangd ${SHADOW_DIR}/.clangd

pushd $SHADOW_DIR
git add .
git commit -m "$@"
git push origin main
popd
