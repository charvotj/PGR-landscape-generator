#!/usr/bin/env bash

# Requires: ImageMagick (convert)
# sudo apt install imagemagick

for img in *.png *.jpg *.jpeg; do
    [ -f "$img" ] || continue

    # Extract number from pattern Cubemap_Sky_XX-...
    # Example: Cubemap_Sky_01-512x512.png → 01
    if [[ "$img" =~ Cubemap_Sky_([0-9]+)- ]]; then
        folder="${BASH_REMATCH[1]}"
    else
        # fallback: use filename without extension
        folder="${img%.*}"
    fi

    mkdir -p "$folder"

    echo "Processing $img → $folder/"

    # cut 6 faces from cross layout
    # each face is assumed to be 512x512

    # OFFSETS (x,y):
    #  +X at (2W, 1H)
    #  -X at (0W, 1H)
    #  +Y at (1W, 0H)
    #  -Y at (1W, 2H)
    #  +Z at (1W, 1H)
    #  -Z at (3W, 1H)

    W=512
    H=512

    convert "$img" -crop ${W}x${H}+$((2*W))+$((1*H)) "$folder/right.jpg"
    convert "$img" -crop ${W}x${H}+0+$((1*H))         "$folder/left.jpg"
    convert "$img" -crop ${W}x${H}+$((1*W))+0         "$folder/top.jpg"
    convert "$img" -crop ${W}x${H}+$((1*W))+$((2*H))  "$folder/bottom.jpg"
    convert "$img" -crop ${W}x${H}+$((1*W))+$((1*H))  "$folder/front.jpg"
    convert "$img" -crop ${W}x${H}+$((3*W))+$((1*H))  "$folder/back.jpg"

    echo "Done!"
done
