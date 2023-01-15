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
    b=`../../build/aig2bdd_reotest $f`
    if [ $? -eq 0 ]
    then
        a=`abc -c "miter a.aig $f; collapse; sat"`
        c=`echo $a | sed -e 's/.*UNSAT\. \(.*\) Time.*/\1/'`
        if [ "$c" = "UNSATISFIABLE" ]
        then
            j=$((j+1))
            rm a.aig
        fi
#        echo $a
    fi
#    echo $b
done

echo "Summary : $j / $i passed"
