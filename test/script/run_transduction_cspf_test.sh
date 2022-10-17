if [ -z $1 ] || [ -z $2 ]
then
    echo "Specify list and directory"
    exit 1    
fi

i=0
j=0

for f in `cat $1`
do
    i=$((i+1))
    g=`basename $f`
    #echo $f
    #transduction_test_cspf $f $2/$g | tee $2/$g.log
    #if [ ${PIPESTATUS[1]} -eq 0 ]
    transduction_test_cspf $f $2/$g > $2/$g.log
    if [ $? -eq 0 ]
    then
        a=`abc -c "cec $f $2/$g"`
        c=`echo $a | sed -e 's/.*Networks are \(.*\)\. Time.*/\1/'`
        if [ "$c" = "equivalent" ] || [ "$c" = "equivalent after structural hashing" ]
        then
            j=$((j+1))            
        fi
        #echo $a
    fi
done

echo "Summary : $j / $i passed"
