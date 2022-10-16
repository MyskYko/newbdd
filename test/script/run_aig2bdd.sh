for f in `cat mcnc-comb_small.txt`
do
    aig2bdd $f
done
