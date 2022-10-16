d="log"

i=0
j=0

for f in `cat mcnc-pla_large.txt`
do
    i=$((i+1))
    g=`basename $f`
    #transduction_test_cspf $f $d/$g | tee $d/$g.log
    #if [ ${PIPESTATUS[1]} -eq 0 ]
    transduction_test_cspf $f $d/$g > $d/$g.log
    if [ $? -eq 0 ]
    then
        a=`abc -c "cec $f $d/$g"`
        c=`echo $a | sed -e 's/.*Networks are \(.*\)\. Time.*/\1/'`
        if [ "$c" = "equivalent" ] || [ "$c" = "equivalent after structural hashing" ]
        then
            j=$((j+1))            
        fi
        #echo $a
    fi
done

echo "Summary : $j / $i passed"
