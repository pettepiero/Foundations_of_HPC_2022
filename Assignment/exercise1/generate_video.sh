#!/bin/bash

# Check if the parameter is provided
if [ -z "$1" ]; then
    echo "Using default name 'snapshot'"
    name="snapshot"
else
    name=$1
fi

index=1

mkdir -p images/snapshots/converted
cd images/snapshots

for i in $(ls $name*.pgm | sort -V); do
  echo "in loop, looking for $i"
  output_file="converted/${name}_${index}.png"
  
  # Convert to strictly black and white, then resize using nearest-neighbor interpolation
  convert "$i" -threshold 50% -resize 1280x -filter point -quality 80 "$output_file"
  
  ((index++))
done

echo "Finished loop"
ffmpeg -r 15 -start_number 0 -i "converted/${name}_%d.png" -vcodec mpeg4 "output.mp4" -y

