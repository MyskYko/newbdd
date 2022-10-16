d="res"

for f in `cat mcnc-pla_small.txt`
do
    g=`basename $f`
    transduction $f $d/$g > $d/$g.log
done
