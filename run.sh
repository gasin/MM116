g++ -std=c++11 -O3 -Wall src/$1.cpp

rm output/$1.txt

for i in {1..100}; do
    java -jar tester.jar -exec "a.exe" -seed $i -novis >> output/$1.txt
done

./obj/del < output/$1.txt > output/tmp.txt
mv output/tmp.txt output/$1.txt
