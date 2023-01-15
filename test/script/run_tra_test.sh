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
    ../../build/tra_test $f a.aig >> tra_test.log
    if [ $? -eq 0 ]
    then
        a=`abc -c "cec $f a.aig"`
        c=`echo $a | sed -e 's/.*Networks are \(.*\)\. Time.*/\1/'`
        if [ "$c" = "equivalent" ] || [ "$c" = "equivalent after structural hashing" ]
        then
            j=$((j+1))
        fi
        rm a.aig
    fi
done

echo "Summary : $j / $i passed"

if (($i == $j))
then
    rm tra_test.log
fi
