#!/bin/bash

# Check if the parameter is provided
if [ -z "$1" ]; then
    echo "Please provide the name."
    exit 1
fi

name=$1
index=1

mkdir -p images/snapshots/converted
cd images/snapshots

for i in $(ls $name*.pgm | sort -V); do
  echo "in loop, looking for $i"
  output_file="converted/${name}_${index}.jpeg"
  convert "$i" -quality 80 "$output_file"
  ((index++))
done

echo "Finished loop"
ffmpeg -r 15 -start_number 0 -i "converted/${name}_%d.jpeg" -vf "scale=iw*20:ih*20:flags=neighbor" -vcodec mpeg4 "output.mp4" -y