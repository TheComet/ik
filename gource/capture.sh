#!/bin/sh
gource                      \
    -1280x720               \
    -r 60                   \
    --time-scale 4          \
    --title "IK" \
    --key                   \
    --file-filter $(cat ignore_files) \
    -o - |                  \
ffmpeg                      \
    -y                      \
    -r 60                   \
    -f image2pipe           \
    -c:v ppm                \
    -i -                    \
    -c:v libx264            \
    -preset medium          \
    -pix_fmt yuv420p        \
    -crf 1                  \
    -threads 0              \
    -bf 0                   \
    output.mp4
