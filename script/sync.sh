#!/bin/bash

git add .
git commit -m "$@"
git push origin main

sh script/sync_github.sh "$@"
