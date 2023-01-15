if [ -z $1 ]
then
    echo "Specify list"
    exit 1
fi

i=0
j=0

for f in `cat $1`
do
    i=$((i+1))
    a=`../oldbdd/build/example/aig2bdd -s -v 1 -p 4 $f`
    a=`echo $a | sed -e 's/.*= \(.*\)Sum.*/\1/'`
    b=`../../build/aig2bdd $f`
    if [ $a == $b ]
    then
        #echo -n "[PASS] "
        j=$((j+1))
    #else
        #echo -n "[FAIL] "
    fi
    #echo $a $b $f
done

echo "Summary : $j / $i passed"
