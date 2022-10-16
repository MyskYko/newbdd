d="res"

for f in `cat mcnc-pla_large.txt`
do
    g=`basename $f`
    transduction_cspf $f $d/$g | tee $d/$g.log
done
