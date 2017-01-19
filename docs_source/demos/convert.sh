for file in *.wav; do
    lame --preset standard "$file" "${file%.wav}.mp3";
done
