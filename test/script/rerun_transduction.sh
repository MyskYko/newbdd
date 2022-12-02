if [ -z $1 ] || [ -z $2 ]
then
    echo "Specify input and output directories"
    exit 1    
fi

for f in `ls $1/*.aig`
do
    g=`basename $f`
    echo $f
    transduction $f $2/$g | tee $2/$g.log
done
