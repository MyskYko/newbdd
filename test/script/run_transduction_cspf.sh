if [ -z $1 ] || [ -z $2 ]
then
    echo "Specify list and directory"
    exit 1    
fi

for f in `cat $1`
do
    g=`basename $f`
    echo $f
    transduction_cspf $f $2/$g | tee $2/$g.log
done
